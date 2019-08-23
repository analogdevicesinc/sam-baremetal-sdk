/*****************************************************************************
 * Copyright(c) 2018-2019 Analog Devices, Inc. All Rights Reserved.
 *****************************************************************************/

#include <sys/platform.h>
#include <sys/adi_core.h>
#include "adi_initialize.h"
#include "sam_dpia_Core0.h"

/** 
 * If you want to use command program arguments, then place them in the following string. 
 */
char __argv_string[] = "";

int main(int argc, char *argv[])
{
    /**
     * Initialize managed drivers and/or services that have been added to 
     * the project.
     * @return zero on success 
     */
    adi_initComponents();
    
    /**
     * The default startup code does not include any functionality to allow
     * core 0 to enable core 1 and core 2. A convenient way to enable
     * core 1 and core 2 is to use the adi_core_enable function. 
     */
    adi_core_enable(ADI_CORE_SHARC0);
    adi_core_enable(ADI_CORE_SHARC1);

    return 0;
}

