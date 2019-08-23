# FLASH programming utility for the  SHARC Audio Module board #

This is a flash programming utility for the SHARC Audio Module board.

Use of this software: 

 * Build your project as you normally would.  CCES will create executable files in the Debug or Release folders in each of the three projects (ARM, SHARC Core 1, SHARC Core 2).
 * Copy your 2 x DXE (SHARC) and 1 x ARM application files (from the /Debug or /Release folders within the three project folder) into the Input_DXE_Files directory
 * Quit CCES so there is no USB contention for the JTAG emulator.  
 * Ensure the ICE-1000 or 2000 emulator is connected to the SHARC Audio Module board.  
 * Run this Prog_SAM_flash command 
   * Use the root name of your DXE files as the 1st argument.  For example, if your DXE is SHARC_App_Core1.DXE the root name is SHARC_App.
   * Use the CCES path as the 2nd argument.  For example, "C:\Analog Devices\Crosscore Embedded Studio 2.8.3"
   * Use the emulator type as the 3rd argument. For example, if using an ICE-1000 the type is 1000.

 The flash programming script is called like so.

`Prog_SAM_flash SHARC_App "C:\Analog Devices\Crosscore Embedded Studio 2.8.3"`