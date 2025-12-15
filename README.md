This project is an STM32 embedded systems lab that configures the microcontroller to run at 80 MHz using the internal PLL, initializes the Cortex-M 
SysTick timer, and implements precise timing delays through interrupt handling. Using these timing primitives, the program drives an onboard LED with a 
consistent blink pattern and generates audio output through a piezo buzzer connected to a GPIO pin.

The project also demonstrates structured embedded design by representing musical notes and songs as data structures and generating square wave audio 
signals based on note frequency and duration. A complete melody (“Mary Had a Little Lamb”) is played using timer-based delays and GPIO toggling. This 
project highlights low level register configuration, interrupt driven timing, and real time hardware control on an STM32 microcontroller.
