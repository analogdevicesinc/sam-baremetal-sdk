@echo off
if [%1]==[] goto usage

set CCES_HOME=%2
set CCES_HOME=%CCES_HOME:"=%

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
"%CCES_HOME%\cldp.exe" -verbose -proc ADSP-SC589 -core 1 -emu %3 -driver Supporting_Files/w25ql512fv_dpia_SC589_SHARC_Core1.dxe -cmd prog -erase affected -format bin -file Output_LDR_Files/%1-SC589.ldr 
@echo Done!
goto :eof
)

:usage
@echo Flash Programming Utility
@echo Copy your 3 DXE files into the Input_DXE_Files directory 
@echo The first argument of this script should be the root name of your DXE (e.g. if your DXE is SHARC_App_Core0 the root name is SHARC_App) 
@echo The second argument of this script should be the CCES path (e.g. "C:\Analog Devices\CrossCore Embedded Studio 2.8.0") 
@echo The third argument of this script should be the type of your emulator (e.g. 1000 or 2000)
@echo Example: Prog_SAM_flash.bat SS_App_SAM "C:\Analog Devices\CrossCore Embedded Studio 2.8.0" 1000
@echo Example: Prog_SAM_flash.bat SS_App_SAM_Automotive "C:\Analog Devices\CrossCore Embedded Studio 2.8.0" 2000