@echo off
if [%1]==[] goto usage

set CCES_HOME=C:\Analog Devices\CrossCore Embedded Studio 2.8.3

@echo .
@echo ************************CrossCore Embedded Studio Path***************************
@echo %CCES_HOME%
@echo *********************************************************************************
@echo .

IF NOT EXIST Input_DXE_Files\%1_Core0 (
@echo The specified input file does not exist: Input_DXE_Files\%1_Core0.exe
@echo. 
@echo Be sure to copy the DXE files into the Input_DXE_Files/ directory
) ELSE (
"%CCES_HOME%\elfloader.exe" -proc ADSP-SC589 -core0=Input_DXE_Files/%1_Core0 -init "%CCES_HOME%/SHARC/ldr/ezkitSC589_initcode_core0_v10" -core1=Input_DXE_Files/%1_Core1.dxe  -core2=Input_DXE_Files/%1_Core2.dxe  -NoFinalTag=Input_DXE_Files/%1_Core0 -NoFinalTag=Input_DXE_Files/%1_Core1.dxe -b SPI -f BINARY -verbose -Width 8 -bcode 0x1 -o Output_LDR_Files/%1-SC589.ldr

@echo programming Output_LDR_Files/%1-SC589.ldr to flash
"%CCES_HOME%\cldp.exe" -verbose -proc ADSP-SC589 -core 1 -emu 1000 -driver Supporting_Files/sam_dpia_Core1.dxe -cmd prog -erase affected -format bin -file Output_LDR_Files/%1-SC589.ldr 
@echo Done!
goto :eof
)

:usage
@echo Flash Programming Utility
@echo Copy your 3 DXE files into the Input_DXE_Files directory 
@echo The only argument of this script should be the root name of your DXE (e.g. if your DXE is SHARC_App_Core0, the root name is SHARC_App) 
@echo Example: prog_SAM_flash_ice1000.bat SHARC_App
