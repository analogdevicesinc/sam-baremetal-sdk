/*********************************************************************************

 Copyright(c) 2014-2019 Analog Devices, Inc. All Rights Reserved.

This software is proprietary and confidential.  By using this software you agree
to the terms of the associated Analog Devices License Agreement.

 *********************************************************************************/

/*
 * This example demonstrates how to interface to a flash device
 * from the flash programmer.
 *
 * This is meant only as an example and may not be fully optimized
 * to access flash as efficiently as possible.
 */


#include <sys/platform.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

/* driver includes */
#include <services/int/adi_int.h>
#include <drivers/spi/adi_spi.h>
#if defined(__ADSP215xx__)
#include <services/spu/adi_spu.h>
#endif

#include "adi_initialize.h"
#include "common/flash_errors.h"
#include "common/flash.h"

static const char *pEzKitTitle = "SHARC Audio Module Board";

#ifdef _MISRA_RULES
#pragma diag(push)
#pragma diag(suppress:misra_rules_all:"Suppress all rules for other header files")
#endif

#ifdef _MISRA_RULES
#pragma diag(pop)
#endif


#ifdef _MISRA_RULES
#pragma diag(push)
#pragma diag(suppress:misra_rule_17_4:"Array indexing shall be the only allowed form of pointer arithmetic.")
#endif /* _MISRA_RULES */

/* Chip info (for W25Q512V) */
#define MANUF_ID                0x20u
#define DEVICE_ID				0xba20u

/* Chip info (for ISSI flash) */
#define ISSI_MANUF_ID			0x9du
#define ISSI_DEVICE_ID			0x601au

//#define USE_QUAD				1			// Use Quad mode for write

/* spi Device Info */
#define SPI_DEVICE_NUM             2u
#define SPI_SELECT_NUM            ADI_SPI_SSEL_ENABLE1

/* Size info */
#define PROLOGUE_SIZE            8u
#define SECTOR_SIZE                256u
#define JEDEC_SIZE                3u
#define STATUS_SIZE                1u

/* status bits */
#define MAX_TIMEOUT                0x0fffu  /* ~20x wait-on-busy limit */
#define SR1_BUSY_BIT            0x01u    /* S0 in SR1 */
#define SR1_WEL_BIT                0x02u    /* S1 in SR1 */
#define SR2_QE_BIT                0x02u    /* S9 in SR2 */

/* Select SPI Flash Commands */
#define CMD_RESET_ENABLE        0x66u
#define CMD_RESET_DEVICE        0x99u

#define CMD_SR_READ            0x05u
#define CMD_SR_WRITE            0x01u

#define CMD_JEDEC_READ            0x9fu
#define CMD_SECTOR_ERASE        0x20u
#define CMD_BLOCK_ERASE            0xD8u
#define CMD_WRITE_ENABLE        0x06u
#define CMD_SINGLE_MODE_WRITE    0x02u
#define CMD_SINGLE_MODE_READ    0x03u
#define CMD_DUAL_MODE_READ        0x3bu
#define CMD_QUAD_MODE_WRITE        0x32u
#define CMD_QUAD_MODE_READ        0x6bu
#define CMD_ENABLE_RESET        0x66u
#define CMD_RESET_DEVICE        0x99u

/* global data buffers */
__attribute__((aligned(32))) uint8_t PrologueBuffer[PROLOGUE_SIZE];
__attribute__((aligned(32))) uint8_t StatusBuffer[STATUS_SIZE];

/* misc functions */
bool ConfigureSPI                (ADI_SPI_HANDLE hSpi);
bool FlashBusyWait                (ADI_SPI_HANDLE hSpi);
bool FlashWriteSR1                (ADI_SPI_HANDLE hSpi, uint8_t bits);
bool FlashWriteSR2                (ADI_SPI_HANDLE hSpi, uint8_t bits);
bool FlashTestSR1                (ADI_SPI_HANDLE hSpi, uint8_t bit);
bool FlashTestSR2                (ADI_SPI_HANDLE hSpi, uint8_t bit);
bool FlashWriteEnable            (ADI_SPI_HANDLE hSpi);
bool FlashEraseSector            (ADI_SPI_HANDLE hSpi, uint32_t Address);
bool FlashReset                    (void);

/* flash write-only utilities (dual-mode write does not exist) */
bool SingleModeWrite            (ADI_SPI_HANDLE hSpi, uint32_t Address, uint8_t *Data, uint32_t ByteCount);
bool QuadModeWrite                (ADI_SPI_HANDLE hSpi, uint32_t Address, uint8_t *Data, uint32_t ByteCount);

/* flash read-only utilities */
bool SingleModeRead                (ADI_SPI_HANDLE hSpi, uint32_t Address, uint8_t *Data, uint32_t ByteCount);
bool DualModeRead                (ADI_SPI_HANDLE hSpi, uint32_t Address, uint8_t *Data, uint32_t ByteCount);
bool QuadModeRead                (ADI_SPI_HANDLE hSpi, uint32_t Address, uint8_t *Data, uint32_t ByteCount);

spi_mode_t sFlashMode;

/*
 * The buffer size can be altered to increase performance provided the heap
 * is large enough.
 */

#define BUFFER_SIZE            0x1000    // should be a multiple of SECTOR_SIZE

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

ADI_SPI_HANDLE hSpi;

/* Flash programmer commands */
typedef enum
{
    FLASH_NO_COMMAND,        /* 0 */
    FLASH_GET_CODES,        /* 1 */
    FLASH_RESET,            /* 2 */
    FLASH_WRITE,            /* 3 */
    FLASH_FILL,                /* 4 */
    FLASH_ERASE_ALL,        /* 5 */
    FLASH_ERASE_SECT,        /* 6 */
    FLASH_READ,                /* 7 */
    FLASH_GET_SECTNUM,        /* 8 */
    FLASH_GET_SECSTARTEND,    /* 9 */
}enProgCmds;

/* structure for flash sector information */
typedef struct _SECTORLOCATION
{
    unsigned long ulStartOff;
    unsigned long ulEndOff;
}SECTORLOCATION;

/* Globals */
const char         *AFP_Title ;                            /* EzKit info */
const char         *AFP_Description;                        /* Device Description */
const char        *AFP_DeviceCompany;                        /* Device Company */
#pragma retain_name /* Not referenced from within DPIA, but needed by CLDP */
char             *AFP_DrvVersion        = "1.00.0";            /* Driver Version */
#pragma retain_name /* Not referenced from within DPIA, but needed by CLDP */
char            *AFP_BuildDate        = __DATE__;            /* Driver Build Date */
enProgCmds         AFP_Command         = FLASH_NO_COMMAND;    /* command sent down from the programmer */
int             AFP_ManCode         = -1;                /* 0x20 = Numonyx */
int             AFP_DevCode         = -1;                /* 0x15 = w25q32bv */
unsigned long     AFP_Offset             = 0x0;                /* offset into flash */
int             *AFP_Buffer;                            /* buffer used to read and write flash */
#pragma retain_name /* Not referenced from within DPIA, but needed by CLDP */
long             AFP_Size             = BUFFER_SIZE;        /* buffer size */
long             AFP_Count             = -1;                /* count of locations to be read or written */
long             AFP_Stride             = -1;                /* stride used when reading or writing */

/* The size of value in bytes (1, 2, or 4).  Generally it can be any size, like 3 or 5, but
 * 1, 2, 4 should be the most useful and 4-byte data is the largest value can be passed in
 * as the fill data value.  Normally stride should be larger than or equal to the value size.
 * But to be compatible with the old implementation, value size is set to 2 by default.
 * So we just ignore the value size and treat it as 1 if the stride is 1.  If the stride is
 * larger than 1, it should be also larger than or equal to the value size.  */
int                AFP_ValueSize        = 2;

                                                        /* ignored when it's larger than AFP_Stride */
int             AFP_NumSectors         = -1;                /* number of sectors in the flash device */
int             AFP_Sector             = -1;                /* sector number */
int             AFP_Error             = NO_ERR;            /* contains last error encountered */
bool            AFP_Verify             = FALSE;            /* verify writes or not */
unsigned long     AFP_StartOff         = 0x0;                /* sector start offset */
unsigned long     AFP_EndOff             = 0x0;                /* sector end offset */
#pragma retain_name /* Not referenced from within DPIA, but needed by CLDP */
int                AFP_FlashWidth        = 0x8;                /* width of the flash device */
int             *AFP_SectorInfo;

/* Locals */
static bool bExit = FALSE;
static SECTORLOCATION *pSectorInfo;

/* external functions */
#ifdef USE_SOFT_SWITCHES
extern void ConfigSoftSwitches(void);
#endif

/* internal functions */
static ERROR_CODE GetFlashInfo(ADI_SPI_HANDLE);


static ERROR_CODE GetNumSectors(void);
static ERROR_CODE AllocateAFPBuffer(void);
static void FreeAFPBuffer(void);
static ERROR_CODE GetSectorMap(SECTORLOCATION *pSectInfo);

static ERROR_CODE ProcessCommand(void);
static ERROR_CODE FillData(unsigned long ulStart, long lCount, long lStride, int *pnData, int ValueSize);
static ERROR_CODE ReadData(unsigned long ulStart, long lCount, long lStride, int *pnData, int ValueSize);
static ERROR_CODE WriteData(unsigned long ulStart, long lCount, long lStride, int *pnData, int ValueSize);

#ifndef SPI_NO
#define SPI_NO 2
#endif

/**
 *****************************************************************************
 * Program entry point
 *
 * @return                True of False depending on if the function is
 *                         successful
 */
int main(void)
{
    uint32_t Result;
    uint8_t SpiMemory[ADI_SPI_INT_MEMORY_SIZE];

    Result = adi_initComponents();

    if (Result != 0)
        return FALSE;

    /* open SPI */
    if (adi_spi_Open(SPI_DEVICE_NUM, SpiMemory, (uint32_t)ADI_SPI_INT_MEMORY_SIZE, &hSpi))
    {
        AFP_Error = SETUP_ERROR;
        return FALSE;
    }

    /* configure SPI */
    if (ConfigureSPI(hSpi))
    {
        AFP_Error = SETUP_ERROR;
        return FALSE;
    }

    /* get flash manufacturer & device codes, title & desc */
    if( AFP_Error == NO_ERR )
    {
        AFP_Error = GetFlashInfo(hSpi);
    }

    /* get the number of sectors for this device */
    if( AFP_Error == NO_ERR )
    {
        AFP_Error = GetNumSectors();
    }

    if( AFP_Error == NO_ERR )
    {
        /* malloc enough space to hold our start and end offsets */
        pSectorInfo = (SECTORLOCATION *)malloc(AFP_NumSectors * sizeof(SECTORLOCATION));
    }

    /* allocate AFP_Buffer */
    if( AFP_Error == NO_ERR )
    {
        AFP_Error = AllocateAFPBuffer();
    }

    /* get sector map */
    if( AFP_Error == NO_ERR )
    {
        AFP_Error = GetSectorMap(pSectorInfo);
    }

    /* point AFP_SectorInfo to our sector info structure */
    if( AFP_Error == NO_ERR )
    {
        AFP_SectorInfo = (int*)pSectorInfo;
    }

    /* command processing loop */
    while ( !bExit )
    {
        /*
         * the programmer will set a breakpoint at "AFP_BreakReady" so it knows
         * when we are ready for a new command because the processor will halt
         *
         * the jump is used so that the label will be part of the debug
         * information in the driver image otherwise it may be left out
         * since the label is not referenced anywhere
         */
        asm("AFP_BreakReady:");
        asm("nop;");
        if ( FALSE )
            asm("jump AFP_BreakReady;");

        /* Make a call to the ProcessCommand */
        AFP_Error = ProcessCommand();
    }

    /* Clear the AFP_Buffer */
    FreeAFPBuffer();

    if( pSectorInfo )
    {
        free(pSectorInfo);
        pSectorInfo = 0;
    }

    /* Close the Device */
    AFP_Error = adi_spi_Close(hSpi);

    if (AFP_Error != NO_ERR)
        return FALSE;

    return TRUE;
}

/**
 *****************************************************************************
 * Get the manufacturer code and device code
 *
 * @return                value if any error occurs getting flash info
 */
static ERROR_CODE GetFlashInfo(ADI_SPI_HANDLE hSpi)
{
    ADI_SPI_TRANSCEIVER xfr;
    uint8_t jedecData[JEDEC_SIZE];
    uint8_t mid = 0xff;
    uint16_t did = 0xffff;

    while (1) {

        /* verify not busy */
        if (FlashBusyWait(hSpi))
            break;

        /* single-mode read sequence */
        PrologueBuffer[0]         = CMD_JEDEC_READ;
        xfr.pPrologue            = PrologueBuffer;
        xfr.PrologueBytes        = 1;
        xfr.pTransmitter        = NULL;
        xfr.TransmitterBytes    = 0;
        xfr.pReceiver            = &jedecData[0];
        xfr.ReceiverBytes        = JEDEC_SIZE;

        /* blocking-mode SPI transaction */
        if(adi_spi_ReadWrite(hSpi, &xfr) != ADI_SPI_SUCCESS)
            break;

        /* extract IDs */
        mid = jedecData[0];
        did = (jedecData[1] << 8) | jedecData[2];

        /* verify */
		if ((MANUF_ID != mid) && (ISSI_MANUF_ID != mid))
            break;
		if ((DEVICE_ID != did) && (ISSI_DEVICE_ID != did))
            break;
        AFP_ManCode = mid;
        AFP_DevCode = did;

        AFP_Title = pEzKitTitle;
		if ( MANUF_ID == mid) {
			AFP_Description = "W25Q512V"; /* Device Description */
			AFP_DeviceCompany = "Winbond"; /* Device Company */
		} else {
			AFP_Description = "IS43TR16128BL"; /* Device Description */
			AFP_DeviceCompany = "ISSI"; /* Device Company */
		}

        return NO_ERR;
    }

    /* failure */
    return SETUP_ERROR;
}

/**
 *****************************************************************************
 * Get the number of sectors for this device.
 *
 * @return                value if any error occurs getting number of sectors
 */
static ERROR_CODE GetNumSectors(void)
{
    AFP_NumSectors = 256;
    return NO_ERR;
}

/**
 *****************************************************************************
 * Allocate memory for the AFP_Buffer
 *
 * @return                value if any error occurs allocating memory
 */
static ERROR_CODE AllocateAFPBuffer(void)
{

    ERROR_CODE ErrorCode = NO_ERR;    //return error code

    /*
     * by making AFP_Buffer as big as possible the plug-in can send and
     * receive more data at a time making the data transfer quicker
     *
     * by allocating it on the heap the compiler does not create an
     * initialized array therefore making the driver image smaller
     * and faster to load
     *
     * The linker description file (LDF) could be modified so that
     * the heap is larger, therefore allowing the BUFFER_SIZE to increase.
     *
     */

    /*
     * the data type of the data being sent from the programmer
     * is in bytes but we store the data as integers to make data
     * manipulation easier when actually programming the data.  This is why
     * BUFFER_SIZE bytes are being allocated rather than BUFFER_SIZE * sizeof(int).
     */
    AFP_Buffer = malloc(BUFFER_SIZE);

    /* AFP_Buffer will be NULL if we could not allocate storage for the buffer */
    if ( AFP_Buffer == 0 )
    {
        /* tell programmer that our buffer was not initialized */
        ErrorCode = BUFFER_IS_NULL;
    }

    return(ErrorCode);
}

/**
 *****************************************************************************
 * Free the AFP_Buffer
 */
static void FreeAFPBuffer(void)
{
    /* free the buffer if we were able to allocate one */
    if ( AFP_Buffer )
        free( AFP_Buffer );

}

/**
 *****************************************************************************
 * Get the start and end offset for each sector in the flash.
 *
 * @param    pSectInfo    pointer to the SECTORLOCATION struct
 *
 * @return                value if any error occurs getting sector map
 */
static ERROR_CODE GetSectorMap(SECTORLOCATION *pSectInfo)
{
    int i;
    uint32_t start = 0, end;

    /* initiate sector information structures */
    for(i = 0; i < AFP_NumSectors; i++)
    {
        pSectInfo[i].ulStartOff = start;
        end = start + 0xFFFF;
        pSectInfo[i].ulEndOff = end;
        start = ++end;
    }

    return NO_ERR;
}

/**
 *****************************************************************************
 * Process each command sent by the programmer based on the value
 * in AFP_Command
 *
 * @return                any error trying to process a command
 */
static ERROR_CODE ProcessCommand(void)
{
    ERROR_CODE ErrorCode = NO_ERR;
    int result = 0;

    switch (AFP_Command)
    {
    /* erase all */
    case FLASH_ERASE_ALL:
    {
        int i = 0;
        for( i = 0; i < AFP_NumSectors; ++i )
        {
            result = FlashEraseSector(hSpi, pSectorInfo[i].ulStartOff);
            if (result) ErrorCode = PROCESS_COMMAND_ERR;
        }
        break;
    }
    /* erase sector */
    case FLASH_ERASE_SECT:
    {
        result = FlashEraseSector(hSpi, pSectorInfo[AFP_Sector].ulStartOff);
        if (result) ErrorCode = PROCESS_COMMAND_ERR;
        break;
    }
    /* fill */
    case FLASH_FILL:
        ErrorCode = FillData(AFP_Offset, AFP_Count, AFP_Stride, AFP_Buffer, AFP_ValueSize);
        break;

    /* get manufacturer and device codes */
    case FLASH_GET_CODES:
        /* AFP_ManCode and AFP_DevCode should have already been initialized */
        break;

    /* get sector number based on address */
    case FLASH_GET_SECTNUM:
    {
        int i = 0;
        for(i = 0; i < AFP_Sector; ++i)
        {
            if(( AFP_Offset >= pSectorInfo[AFP_Sector].ulStartOff) &&
                    (AFP_Offset <= pSectorInfo[AFP_Sector].ulEndOff))
            {
                AFP_Sector = i;
                ErrorCode = NO_ERR;
                break;
            }
        }
        if (i >= AFP_NumSectors) ErrorCode = PROCESS_COMMAND_ERR;
        break;
    }
    /* get sector number start and end offset */
    case FLASH_GET_SECSTARTEND:
    {
        if ( AFP_Sector < AFP_NumSectors )
        {
            AFP_StartOff = pSectorInfo[AFP_Sector].ulStartOff;
            AFP_EndOff = pSectorInfo[AFP_Sector].ulEndOff;
            ErrorCode = NO_ERR;
        }
        else
        {
            ErrorCode = INVALID_SECTOR;
        }
        break;
    }
    /* read */
    case FLASH_READ:
        ErrorCode = ReadData(AFP_Offset, AFP_Count, AFP_Stride, AFP_Buffer, AFP_ValueSize);
        break;
        /* reset */
    case FLASH_RESET:
    {
        result = FlashReset();
        if (result) ErrorCode = PROCESS_COMMAND_ERR;
        break;
    }
    /* write */
    case FLASH_WRITE:
        ErrorCode = WriteData(AFP_Offset, AFP_Count, AFP_Stride, AFP_Buffer, AFP_ValueSize);
        break;

        /* no command or unknown command do nothing */
    case FLASH_NO_COMMAND:
    default:
        /* set our error */
        ErrorCode = UNKNOWN_COMMAND;
        break;
    }

    /* clear the command */
    AFP_Command = FLASH_NO_COMMAND;

    return(ErrorCode);
}

/**
 *****************************************************************************
 * Fill flash device with a value.
 *
 * @param    ulStart        Address in flash to start the writes at
 * @param    lCount        Number of elements to write, in this case bytes
 * @param    lStride        Number of locations to skip between writes
 * @param    *pnData        Pointer to data buffer(written to by the programmer)
 * @param    ValueSize    Size of the value in bytes (1, 2, or 4)
 *
 * @return                value if any error occurs during fill
 */
static ERROR_CODE FillData(unsigned long ulStart, long lCount, long lStride, int* pnData, int ValueSize)
{
    /* Save value so we can reuse the buffer instead of allocate another buffer.
       This should be helpful for processors with small internal memory.  */
    int value = *pnData;
    uint8_t *buf = (uint8_t *) pnData;
    uint32_t addr = ulStart;
    long BufferCount = BUFFER_SIZE / ValueSize;
    long i;
    ERROR_CODE Result;

    /* See the comment for AFP_ValueSize.  */

    if (lStride == 1)
        ValueSize = 1;

    if (lStride < ValueSize)
        return WRITE_ERROR;

    if (ValueSize != 1 && ValueSize != 2 && ValueSize != 4)
        return WRITE_ERROR;

    /* Fill the buffer */
    for (i = 0; i < BufferCount; i++)
    {
        if (ValueSize == 1)
            buf[i] = value & 0xff;
        else if (ValueSize == 2)
        {
            buf[i * 2] = value & 0xff;
            buf[i * 2 + 1] = (value >> 8) & 0xff;
        }
        else /* ValueSize == 4 */
        {
            buf[i * 4] = value & 0xff;
            buf[i * 4 + 1] = (value >> 8) & 0xff;
            buf[i * 4 + 2] = (value >> 16) & 0xff;
            buf[i * 4 + 3] = (value >> 24) & 0xff;
        }
    }

    while (lCount > 0)
    {
        long c = lCount > BufferCount ? BufferCount : lCount;
        Result = WriteData(addr, c, lStride, (int *) buf, ValueSize);
        if (Result != NO_ERR) return Result;
        lCount -= c;
        addr += c * lStride;
    }
    return NO_ERR;
}


/**
 *****************************************************************************
 * Write a buffer to flash device.
 *
 * @param    ulStart        Address in flash to start the writes at
 * @param    lCount        Number of elements to write, in this case bytes
 * @param    lStride        Number of locations to skip between writes
 * @param    *pnData        Pointer to data buffer(written to by the programmer)
 * @param    ValueSize    Size of the value in bytes (1, 2, or 4)
 *
 * @return                value if any error occurs during write
 */
static ERROR_CODE WriteData(unsigned long ulStart, long lCount, long lStride, int *pnData, int ValueSize)
{
    int result = 0;
    uint32_t addr;
    uint8_t *buf = (uint8_t *) pnData;
    uint32_t uLocalCount = lCount;

    /* See the comment for AFP_ValueSize.  */

    if (lStride == 1)
        ValueSize = 1;

    if (lStride < ValueSize)
        return WRITE_ERROR;

    if (ValueSize != 1 && ValueSize != 2 && ValueSize != 4)
        return WRITE_ERROR;

    /* Make sure the buffer is large enough.  */
    if (ValueSize * lCount > BUFFER_SIZE)
        return WRITE_ERROR;

    addr = ulStart;

    if (lStride == 1)
    {
        long i;

        while (uLocalCount)
        {
            uint32_t uPartCount = 0;

            // Finish the sector
            if(addr & 0xff)
            {
                uPartCount = SECTOR_SIZE - (addr & 0xff);    // fill remaining sector
                if(uPartCount > uLocalCount)
                    uPartCount = uLocalCount;
#if USE_QUAD
                result = QuadModeWrite(hSpi, addr, (uint8_t *)buf, uPartCount);
#else
                result = SingleModeWrite(hSpi, addr, (uint8_t *)buf, uPartCount);
#endif
                addr += uPartCount;
                buf += uPartCount;
                uLocalCount -= uPartCount;
            }
            if(uLocalCount)
            {
                uint32_t uSize = uLocalCount < SECTOR_SIZE ? uLocalCount : SECTOR_SIZE;
#if USE_QUAD
                result = QuadModeWrite(hSpi, addr, (uint8_t *)buf, uSize);
#else
                result = SingleModeWrite(hSpi, addr, (uint8_t *)buf, uSize);
#endif
                addr += uSize;
                buf += uSize;
                uLocalCount -= uSize;
            }
        }
    }
    else
    {
        long i;

        for (i = 0; i < lCount; i++)
        {
            result = SingleModeWrite(hSpi, addr, (uint8_t *)buf + i * ValueSize,  ValueSize);
            if (result) break;
            addr += lStride;
        }
    }

    if (result == 0 && AFP_Verify == TRUE)
    {
        /* Use a small buffer to reduce memory usage.  */

        uint8_t buf2[4];
        long i;

        addr = ulStart;
        for (i = 0; i < lCount; i++)
        {
            result = SingleModeRead( hSpi, addr, (uint8_t *)buf2, ValueSize);
            if (result)
                return NOT_READ_ERROR;
            if (memcmp (buf, buf2, ValueSize))
                return VERIFY_WRITE;
            ulStart += lStride;
            buf += ValueSize;
        }
    }

    return (result ? WRITE_ERROR : NO_ERR);
}


/**
 *****************************************************************************
 * Read a buffer from flash device.
 *
 * @param    ulStart        Address in flash to start the reads at
 * @param    lCount        Number of elements to read, in this case bytes
 * @param    lStride        Number of locations to skip between reads
 * @param    *pnData        Pointer to data buffer to fill
 * @param    ValueSize    Size of the value in bytes (1, 2, or 4)
 *
 * @return                value if any error occurs during reading
 */
static ERROR_CODE ReadData(unsigned long ulStart, long lCount, long lStride, int *pnData, int ValueSize)
{
    int result = 0;
    uint8_t *buf = (uint8_t *)pnData;

    /* See the comment for AFP_ValueSize.  */
    if (lStride == 1)
        ValueSize = 1;

    if (lStride < ValueSize)
        return NOT_READ_ERROR;

    if (ValueSize != 1 && ValueSize != 2 && ValueSize != 4)
        return NOT_READ_ERROR;

    /* Make sure the buffer is large enough.  */
    if (ValueSize * lCount > BUFFER_SIZE)
        return NOT_READ_ERROR;

    if (lStride == 1)
    {
        result = SingleModeRead( hSpi, ulStart, (uint8_t *)buf, lCount);
    }
    else
    {
        long i;

        for (i = 0; i < lCount; i++)
        {
            result = SingleModeRead( hSpi, ulStart, (uint8_t *)buf, ValueSize);
            if (result)
                break;
            ulStart += lStride;
            buf += ValueSize;
        }
    }

    return (result ? NOT_READ_ERROR : NO_ERR);
}
/* busy wait with timeout */
bool FlashBusyWait(ADI_SPI_HANDLE hSpi)
{
    uint16_t timeout;

    /* poll on the busy bit until it clears or we time out */
    for (timeout = 0; timeout < MAX_TIMEOUT; timeout++) {
        if (false == FlashTestSR1(hSpi, SR1_BUSY_BIT))
            break;
    }

    /* return timeout status */
    return (MAX_TIMEOUT == timeout);
}

bool ConfigureSPI(ADI_SPI_HANDLE hSpi)
{

    while (1) {

        /* disable DMA */
        if (adi_spi_EnableDmaMode(hSpi, false))                        break;

        /* device in master of the SPI interface */
        if (adi_spi_SetMaster(hSpi, true))                            break;

        /* default transceiver mode to read/write */
        if (adi_spi_SetTransceiverMode(hSpi, ADI_SPI_TXRX_MODE))    break;

        /* send zeros if tx SPI underflows*/
        if (adi_spi_SetTransmitUnderflow(hSpi, true))                break;

        /* data transitions on falling edge of clock */
        if (adi_spi_SetClockPhase(hSpi, false))                        break;

        /* SPI clock is SCLK divided by 500 + 1 */
        if (adi_spi_SetClock(hSpi, 500u))                            break;

        /* disable hardware-based SPI slave select */
        if (adi_spi_SetHwSlaveSelect(hSpi, false))                    break;

        /* use driver controlled slave select */
        if (adi_spi_ManualSlaveSelect(hSpi, false))                    break;

        /* designate slave select */
        if (adi_spi_SetSlaveSelect(hSpi, SPI_SELECT_NUM))        break;

        /* SPI data transfers are 8 bit */
        if (adi_spi_SetWordSize(hSpi, ADI_SPI_TRANSFER_8BIT))        break;

        /* generate tx data interrupt when watermark level breaches 50% level */
        /* DMA watermark levels are disabled because SPI is in interrupt mode */
        if (adi_spi_SetTxWatermark(hSpi,
                                ADI_SPI_WATERMARK_50,
                                ADI_SPI_WATERMARK_DISABLE,
                                ADI_SPI_WATERMARK_DISABLE))        break;

        /* generate rx data interrupt when watermark level breaches 50% level */
        /* DMA watermark levels are disabled because SPI is in interrupt mode */
        if (adi_spi_SetRxWatermark(hSpi,
                                ADI_SPI_WATERMARK_50,
                                ADI_SPI_WATERMARK_DISABLE,
                                ADI_SPI_WATERMARK_DISABLE))        break;

        /* success */
        return false;

    } /* end while */

    /* failure */
    return true;
}

/* write the flash SR1 register */
bool FlashWriteSR1 (ADI_SPI_HANDLE hSpi, uint8_t bits)
{
    ADI_SPI_TRANSCEIVER xfr;

    while (1) {

        /* assert flash write enable state */
        if (FlashWriteEnable(hSpi))
            break;

        /* write SR1 register */
        PrologueBuffer[0]         = CMD_SR_WRITE;
        PrologueBuffer[1]        = (uint8_t)(bits);            /* lower bits first to SR1 */
        xfr.pPrologue            = PrologueBuffer;
        xfr.PrologueBytes        = 2;
        xfr.pTransmitter        = NULL;
        xfr.TransmitterBytes    = 0;
        xfr.pReceiver            = NULL;
        xfr.ReceiverBytes        = 0;

        /* blocking-mode SPI transaction */
        if(adi_spi_ReadWrite(hSpi, &xfr) != ADI_SPI_SUCCESS)
            break;

        /* wait with timeout */
        if (FlashBusyWait(hSpi))
            break;

        /* verify flash is write-disabled */
        if (false != FlashTestSR1(hSpi, SR1_WEL_BIT))
            break;

        /* success */
        return false;
    }

    /* failure */
    return true;

}


/* write the flash SR2 register */
bool FlashWriteSR2 (ADI_SPI_HANDLE hSpi, uint8_t bits)
{
    ADI_SPI_TRANSCEIVER xfr;

    while (1) {

        /* assert flash write enable state */
        if (FlashWriteEnable(hSpi))
            break;

        /* write SR2 register */
        PrologueBuffer[0]         = CMD_SR_WRITE;
        PrologueBuffer[1]        = (uint8_t)bits;
        xfr.pPrologue            = PrologueBuffer;
        xfr.PrologueBytes        = 2;
        xfr.pTransmitter        = NULL;
        xfr.TransmitterBytes    = 0;
        xfr.pReceiver            = NULL;
        xfr.ReceiverBytes        = 0;

        /* blocking-mode SPI transaction */
        if(adi_spi_ReadWrite(hSpi, &xfr) != ADI_SPI_SUCCESS)
            break;

        /* wait with timeout */
        if (FlashBusyWait(hSpi))
            break;

        /* verify flash is write-disabled */
        if (false != FlashTestSR1(hSpi, SR1_WEL_BIT))
            break;

        /* success */
        return false;
    }

    /* failure */
    return true;

}
/* return flash SR bit status as a boolean */
/* note: the bit being tested is in a 16-bit combined field */
bool FlashTestSR1 (ADI_SPI_HANDLE hSpi, uint8_t bit)
{
    ADI_SPI_TRANSCEIVER xfr;

    while (1) {

        /* read SR1 */
        PrologueBuffer[0]         = CMD_SR_READ;
        xfr.pPrologue            = PrologueBuffer;
        xfr.PrologueBytes        = 1;
        xfr.pTransmitter        = NULL;
        xfr.TransmitterBytes    = 0;
        xfr.pReceiver            = StatusBuffer;
        xfr.ReceiverBytes        = 1;  /* one byte for SR1 readback */

        /* blocking-mode SPI transaction */
        if(adi_spi_ReadWrite(hSpi, &xfr) != ADI_SPI_SUCCESS)
            break;

        return (StatusBuffer[0] & bit);
    }

    /* failure */
    return true;
}

/* return flash SR2 bit status as a boolean */
bool FlashTestSR2 (ADI_SPI_HANDLE hSpi, uint8_t bit)
{
    ADI_SPI_TRANSCEIVER xfr;

    while (1) {

        /* read SR1 */
        PrologueBuffer[0]         = CMD_SR_READ;
        xfr.pPrologue            = PrologueBuffer;
        xfr.PrologueBytes        = 1;
        xfr.pTransmitter        = NULL;
        xfr.TransmitterBytes    = 0;
        xfr.pReceiver            = StatusBuffer;
        xfr.ReceiverBytes        = 1;  /* one byte for SR2 readback */

        /* blocking-mode SPI transaction */
        if(adi_spi_ReadWrite(hSpi, &xfr) != ADI_SPI_SUCCESS)
            break;

        return (StatusBuffer[0] & bit);
    }

    /* failure */
    return true;
}

/* prepare the flash for writing */
bool FlashWriteEnable(ADI_SPI_HANDLE hSpi)
{
    ADI_SPI_TRANSCEIVER xfr;

    while (1) {

        /* verify not busy */
        if (FlashBusyWait(hSpi))
            break;

        PrologueBuffer[0]         = CMD_WRITE_ENABLE;
        xfr.pPrologue            = PrologueBuffer;
        xfr.PrologueBytes        = 1;
        xfr.pTransmitter        = NULL;
        xfr.TransmitterBytes    = 0;
        xfr.pReceiver            = NULL;
        xfr.ReceiverBytes        = 0;

        /* blocking-mode SPI transaction */
        if(adi_spi_ReadWrite(hSpi, &xfr) != ADI_SPI_SUCCESS)
            break;

        /* verify not busy */
        if (FlashBusyWait(hSpi))
            break;

        /* verify flash is write-enabled */
        if (true != FlashTestSR1(hSpi, SR1_WEL_BIT))
            break;

        /* success */
        return false;
    }

    /* failure */
    return true;
}

/* erase a 64k-byte block on the flash, assumes address is aligned to sector start boundary */
bool FlashEraseSector(ADI_SPI_HANDLE hSpi, uint32_t Address)
{
    ADI_SPI_TRANSCEIVER xfr;

    while (1) {

        /* assert flash write enable state */
        if (FlashWriteEnable(hSpi))
            break;

        /* sector erase sequence */
        PrologueBuffer[0]        = CMD_BLOCK_ERASE;
        PrologueBuffer[1]        = (uint8_t)(Address >> 16);
        PrologueBuffer[2]        = (uint8_t)(Address >> 8);
        PrologueBuffer[3]        = (uint8_t)Address;
        xfr.pPrologue            = PrologueBuffer;
        xfr.PrologueBytes        = 4;
        xfr.pTransmitter        = NULL;
        xfr.TransmitterBytes    = 0;
        xfr.pReceiver            = NULL;
        xfr.ReceiverBytes        = 0;

        /* blocking-mode SPI transaction */
        if(adi_spi_ReadWrite(hSpi, &xfr) != ADI_SPI_SUCCESS)
            break;

        /* wait with timeout */
        if (FlashBusyWait(hSpi))
            break;

        /* verify flash is write-disabled */
        if (false != FlashTestSR1(hSpi, SR1_WEL_BIT))
            break;

        /* success */
        return false;
    }

    /* failure */
    return true;
}

/* reset the flash */
bool FlashReset(void)
{
    ADI_SPI_TRANSCEIVER xfr;
    uint16_t i, j;

    while (1) {

        /* verify not busy */
        if (FlashBusyWait(hSpi))
            break;

        /* enable flash reset */
        PrologueBuffer[0]         = CMD_RESET_ENABLE;
        xfr.pPrologue            = PrologueBuffer;
        xfr.PrologueBytes        = 1;
        xfr.pTransmitter        = NULL;
        xfr.TransmitterBytes    = 0;
        xfr.pReceiver            = NULL;
        xfr.ReceiverBytes        = 0;

        /* blocking-mode SPI transaction */
        if(adi_spi_ReadWrite(hSpi, &xfr) != ADI_SPI_SUCCESS)
            break;

        /* reset the flash */
        PrologueBuffer[0]         = CMD_RESET_DEVICE;
        xfr.pPrologue            = PrologueBuffer;
        xfr.PrologueBytes        = 1;
        xfr.pTransmitter        = NULL;
        xfr.TransmitterBytes    = 0;
        xfr.pReceiver            = NULL;
        xfr.ReceiverBytes        = 0;

        /* blocking-mode SPI transaction */
        if(adi_spi_ReadWrite(hSpi, &xfr) != ADI_SPI_SUCCESS)
            break;

        /*
            chip reset (tRST) is specified at about 30us during which time no commands will
            be accepted during the reset period, so we can't even poll the busy bit...
            this spin loop more than meets the need.
        */
        for (i = 0, j = 0; i < 0xfff; i++) {
            j = i;
            i = j;
        }

        /* wait with timeout */
        if (FlashBusyWait(hSpi))
            break;

        /*
         Clear SR2 explicitly to avoid possibility of dangling QE bits set
         (maybe due to previously-failed quad-mode tests) that are not cleared
         by the above reset instruction sequence. This can take many 10s of
         busy read polling cycles.
        */
        if (FlashWriteSR2(hSpi, 0))
            break;

        /* insure QE bit is clear */
        if (false != FlashTestSR2(hSpi, SR2_QE_BIT))
            while (1);  /* if we ever land here, restore the explicit QE clear above */

        /* success */
        return false;
    }

    /* failure */
    return true;
}

bool SingleModeWrite(ADI_SPI_HANDLE hSpi, uint32_t Address, uint8_t *Data, uint32_t ByteCount)
{
    ADI_SPI_TRANSCEIVER xfr;

    while (1) {

        /* assert flash write enable state */
        if (FlashWriteEnable(hSpi))
            break;

        /* page write sequence */
        PrologueBuffer[0]        = CMD_SINGLE_MODE_WRITE;
        PrologueBuffer[1]        = (uint8_t)(Address >> 16);
        PrologueBuffer[2]        = (uint8_t)(Address >> 8);
        PrologueBuffer[3]        = (uint8_t)Address;
        xfr.pPrologue            = PrologueBuffer;
        xfr.PrologueBytes        = 4;
        xfr.pTransmitter        = Data;
        xfr.TransmitterBytes    = ByteCount;
        xfr.pReceiver            = NULL;
        xfr.ReceiverBytes        = 0;

        /* blocking-mode SPI transaction */
        if(adi_spi_ReadWrite(hSpi, &xfr) != ADI_SPI_SUCCESS)
            break;

        /* wait with timeout */
        if (FlashBusyWait(hSpi))
            break;

        /* verify flash is write-disabled */
        if (false != FlashTestSR1(hSpi, SR1_WEL_BIT))
            break;

        /* success */
        return false;

    } /* end while */

    /* failure */
    return true;
}

bool QuadModeWrite(ADI_SPI_HANDLE hSpi, uint32_t Address, uint8_t *Data, uint32_t ByteCount)
{
    ADI_SPI_TRANSCEIVER xfr;
    bool bError = false;

    /* assert flash write enable for setting QE bit */
    if (FlashWriteEnable(hSpi))
    {
        bError = true;
    }

    /* set the QE bit manually */
    if (FlashWriteSR2(hSpi, SR2_QE_BIT))
    {
        bError = true;
    }

    /* assert flash write enable again for quad write command */
    if (FlashWriteEnable(hSpi))
    {
        bError = true;
    }

    /* use manual slave select */
    if (adi_spi_ManualSlaveSelect(hSpi, true) != ADI_SPI_SUCCESS)
    {
        bError = true;
    }

    /* select (continuous select through command and data phases) */
    if (adi_spi_SlaveSelect(hSpi, true) != ADI_SPI_SUCCESS)
    {
        bError = true;
    }

    /* command phase: queue the quad mode flash write instruction in single-bit mode */
    PrologueBuffer[0]        = CMD_QUAD_MODE_WRITE;
    PrologueBuffer[1]        = (uint8_t)(Address >> 16);
    PrologueBuffer[2]        = (uint8_t)(Address >> 8);
    PrologueBuffer[3]        = (uint8_t)Address;
    xfr.pPrologue            = PrologueBuffer;
    xfr.PrologueBytes        = 4;
    xfr.pTransmitter        = NULL;
    xfr.TransmitterBytes    = 0;
    xfr.pReceiver            = NULL;
    xfr.ReceiverBytes        = 0;

    /* blocking-mode SPI transaction */
    if(adi_spi_ReadWrite(hSpi, &xfr) != ADI_SPI_SUCCESS)
    {
        bError = true;
    }

    /* transition SPI to quad write mode */
    if (adi_spi_SetTransceiverMode(hSpi, ADI_SPI_TX_QSPI_MODE))
    {
        bError = true;
    }

    /* data phase: queue the data for quad mode transfer */
    xfr.pPrologue            = NULL;
    xfr.PrologueBytes        = 0;
    xfr.pTransmitter        = Data;
    xfr.TransmitterBytes    = ByteCount;
    xfr.pReceiver            = NULL;
    xfr.ReceiverBytes        = 0;

    /* blocking-mode SPI transaction */
    if(adi_spi_ReadWrite(hSpi, &xfr) != ADI_SPI_SUCCESS)
    {
        bError = true;
    }

    /* deselect */
    if (adi_spi_SlaveSelect(hSpi, false))
    {
        bError = true;
    }

    /* revert SPI transceiver mode */
    if (adi_spi_SetTransceiverMode(hSpi, ADI_SPI_TXRX_MODE))
    {
        bError = true;
    }

    /* use driver controlled slave select */
    if (adi_spi_ManualSlaveSelect(hSpi, false)  != ADI_SPI_SUCCESS)
    {
        bError = true;
    }

    /* exit quad mode for polling check */
    if (FlashWriteSR2(hSpi, 0))
    {
        bError = true;
    }

    /* verify flash is write-disabled */
    if (false != FlashTestSR1(hSpi, SR1_WEL_BIT))
    {
        bError = true;
    }

    /* verify not busy */
    if (FlashBusyWait(hSpi))
    {
        bError = true;
    }

    /* clear SR1 once again... not sure why this is needed, but it seems critical */
    /* perhaps some kind of additional magic delay */
    if (FlashWriteSR1(hSpi, 0))
    {
        bError = true;
    }

    if (bError)
    {
        /* failure */
        return true;
    }
    else
    {
        /* success */
        return false;
    }
}

bool SingleModeRead(ADI_SPI_HANDLE hSpi, uint32_t Address, uint8_t *p, uint32_t ByteCount)
{
    ADI_SPI_TRANSCEIVER xfr;

    while (1) {

        /* verify not busy */
        if (FlashBusyWait(hSpi))
            break;

        /* single-mode read sequence */
        PrologueBuffer[0]        = CMD_SINGLE_MODE_READ;
        PrologueBuffer[1]        = (uint8_t)(Address >> 16);
        PrologueBuffer[2]        = (uint8_t)(Address >> 8);
        PrologueBuffer[3]        = (uint8_t)Address;
        xfr.pPrologue            = PrologueBuffer;
        xfr.PrologueBytes        = 4;
        xfr.pTransmitter        = NULL;
        xfr.TransmitterBytes    = 0;
        xfr.pReceiver            = p;
        xfr.ReceiverBytes        = ByteCount;

        /* blocking-mode SPI transaction */
        if(adi_spi_ReadWrite(hSpi, &xfr) != ADI_SPI_SUCCESS)
            break;

        /* success */
        return false;
    }

    /* failure */
    return true;
}

bool DualModeRead(ADI_SPI_HANDLE hSpi, uint32_t Address, uint8_t *Data, uint32_t ByteCount)
{
    ADI_SPI_TRANSCEIVER xfr;

    bool bError = false;

    /* verify not busy */
    if (FlashBusyWait(hSpi))
    {
        bError = true;
    }

    /* use manual slave select */
    if (adi_spi_ManualSlaveSelect(hSpi, true) != ADI_SPI_SUCCESS)
    {
        bError = true;
    }

    /* dual-mode read sequence only (sent in single-bit mode) */
    PrologueBuffer[0]        = CMD_DUAL_MODE_READ;
    PrologueBuffer[1]        = (uint8_t)(Address >> 16);
    PrologueBuffer[2]        = (uint8_t)(Address >> 8);
    PrologueBuffer[3]        = (uint8_t)Address;
    PrologueBuffer[4]        = 0u;  /* one "dummy" slot required */
    xfr.pPrologue            = PrologueBuffer;
    xfr.PrologueBytes        = 5;
    xfr.pTransmitter        = NULL;
    xfr.TransmitterBytes    = 0;
    xfr.pReceiver            = NULL;
    xfr.ReceiverBytes        = 0;

    /* select (continuous select through command and data phases) */
    if (adi_spi_SlaveSelect(hSpi, true) != ADI_SPI_SUCCESS)
    {
        bError = true;
    }

    /* blocking-mode SPI transaction */
    if(adi_spi_ReadWrite(hSpi, &xfr) != ADI_SPI_SUCCESS)
    {
        bError = true;
    }

    /* dual-mode data read sequence (no prologue) */
    xfr.pPrologue            = NULL;
    xfr.PrologueBytes        = 0;
    xfr.pTransmitter        = NULL;
    xfr.TransmitterBytes     = 0;
    xfr.pReceiver            = Data;
    xfr.ReceiverBytes        = ByteCount;

    /* place SPI into dual transceiver mode */
    if (adi_spi_SetTransceiverMode(hSpi, ADI_SPI_RX_DIOM_MODE) != ADI_SPI_SUCCESS)
    {
        bError = true;
    }

    /* blocking-mode SPI transaction */
    if(adi_spi_ReadWrite(hSpi, &xfr) != ADI_SPI_SUCCESS)
    {
        bError = true;
    }

    /* deselect */
    if (adi_spi_SlaveSelect(hSpi, false)  != ADI_SPI_SUCCESS)
    {
        bError = true;
    }

    /* revert SPI transceiver mode */
    if (adi_spi_SetTransceiverMode(hSpi, ADI_SPI_TXRX_MODE) != ADI_SPI_SUCCESS)
    {
        bError = true;
    }

    /* use driver controlled slave select */
    if (adi_spi_ManualSlaveSelect(hSpi, false)  != ADI_SPI_SUCCESS)
    {
        bError = true;
    }

    if (bError)
    {
        /* failure */
        return true;
    }
    else
    {
        /* success */
        return false;
    }
}

bool QuadModeRead(ADI_SPI_HANDLE hSpi, uint32_t Address, uint8_t *Data, uint32_t ByteCount)
{
    ADI_SPI_TRANSCEIVER xfr;
    bool bError = false;


    /* verify not busy */
    if (FlashBusyWait(hSpi))
    {
        bError = true;
    }

    /* set the QE bit manually */
    if (FlashWriteSR2(hSpi, SR2_QE_BIT))
    {
        bError = true;
    }

    /* use manual slave select */
    if (adi_spi_ManualSlaveSelect(hSpi, true) != ADI_SPI_SUCCESS)
    {
        bError = true;
    }

    /* queue quad-mode read sequence only (sent in single-bit mode) */
    PrologueBuffer[0]        = CMD_QUAD_MODE_READ;
    PrologueBuffer[1]        = (uint8_t)(Address >> 16);
    PrologueBuffer[2]        = (uint8_t)(Address >> 8);
    PrologueBuffer[3]        = (uint8_t)Address;
    PrologueBuffer[4]        = 0u;  /* one "dummy" slot required */
    xfr.pPrologue            = PrologueBuffer;
    xfr.PrologueBytes        = 5;
    xfr.pTransmitter        = NULL;
    xfr.TransmitterBytes    = 0;
    xfr.pReceiver            = NULL;
    xfr.ReceiverBytes        = 0;

    /* select (continuous select through command and data phases) */
    if (adi_spi_SlaveSelect(hSpi, true) != ADI_SPI_SUCCESS)
    {
        bError = true;
    }

    /* blocking-mode SPI transaction */
    if(adi_spi_ReadWrite(hSpi, &xfr) != ADI_SPI_SUCCESS)
    {
        bError = true;
    }

    /* queue quad-mode data read sequence (no prologue) */
    xfr.pPrologue            = NULL;
    xfr.PrologueBytes        = 0;
    xfr.pTransmitter        = NULL;
    xfr.TransmitterBytes    = 0;
    xfr.pReceiver            = Data;
    xfr.ReceiverBytes        = ByteCount;

    /* place SPI into quad read mode */
    if (adi_spi_SetTransceiverMode(hSpi, ADI_SPI_RX_QSPI_MODE))
    {
        bError = true;
    }

    /* blocking-mode SPI transaction */
    if(adi_spi_ReadWrite(hSpi, &xfr) != ADI_SPI_SUCCESS)
    {
        bError = true;
    }

    /* de-select */
    if (adi_spi_SlaveSelect(hSpi, false) != ADI_SPI_SUCCESS)
    {
        bError = true;
    }

    /* revert SPI transceiver mode */
    if (adi_spi_SetTransceiverMode(hSpi, ADI_SPI_TXRX_MODE))
    {
        bError = true;
    }

    /* use driver controlled slave select */
    if (adi_spi_ManualSlaveSelect(hSpi, false)  != ADI_SPI_SUCCESS)
    {
        bError = true;
    }

    /* clear the QE bit manually */
    if (FlashWriteSR2(hSpi, 0))
    {
        bError = true;
    }

    if (bError)
    {
        /* failure */
        return true;
    }
    else
    {
        /* success */
        return false;
    }
}
