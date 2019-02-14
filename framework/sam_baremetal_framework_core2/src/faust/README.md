 # Faust Support for the SHARC Audio Module

[Faust](http://faust.grame.fr/) is an optional, high-level software framework for audio effects and audio synthesis.

The Faust application will generate a number of source files that should be placed in this directory:

* `fast_pow2.h`
* `samFaustDSP.cpp`
* `samFaustDSP.h`
* `samFaustDSPCore.h`

In addition there is a header file that is common across all cores called audio_system_config.h. In this file pre-processor macros should be set in the following way. The example below indicates that a Faust algorithm will only be running on Core1 and that Core2 will be simply passing audio to the codec. 

```cpp
// set to true if using the DIY Fin board
#define SAM_DIY_FIN_BOARD_PRESENT           TRUE
…
// set to true when using faust
#define FAUST_INSTALLED                     TRUE
…
// set to true to enable faust on SHARC cores 1 and/or 2
#define USE_FAUST_ALGORITHM_CORE1           TRUE
#define USE_FAUST_ALGORITHM_CORE2           FALSE
…
// enable MIDI on the SHARC core if Faust is being used
#define MIDI_UART_MANAGED_BY_SHARC1_CORE    TRUE
````

For more information on using Faust with the SHARC Audio Module please see [Faust Integration with the SHARC Audio Module](https://wiki.analog.com/resources/tools-software/sharc-audio-module/faust) on the [SHARC Audio Module Wiki](https://wiki.analog.com/resources/tools-software/sharc-audio-module).