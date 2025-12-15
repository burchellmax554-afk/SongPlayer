#include "stm32l4xx.h"
#include "sine_table.h"
#include <stdint.h>

// Global variable to hold the current angle
volatile int angle = 0;

// Function prototypes
void DAC_Init(void);
void TIM4_Init(void);
void GPIO_Init(void);
int lookup_sine(int angle);
void TIM4_IRQHandler(void);

int main(void) {
    // Initialize GPIO, DAC, and Timer
    GPIO_Init();
    DAC_Init();
    TIM4_Init();

    // Main loop - The DAC output is handled in the interrupt handler (no need for code here)
    while (1) {
    	//Nothing
    }
}

// Initialize GPIOA for DAC output (PA4)
void GPIO_Init(void) {
    // Enable clock for GPIOA (PA4 for DAC output)
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;

    // Set PA4 as analog mode (to output DAC signal)
    GPIOA->MODER &= ~(GPIO_MODER_MODE4);
    GPIOA->MODER |= GPIO_MODER_MODE4_0 | GPIO_MODER_MODE4_1;  // PA4 in analog mode
}

// Initialize DAC Channel 1
void DAC_Init(void) {
    // Enable clock for DAC
    RCC->APB1ENR1 |= RCC_APB1ENR1_DAC1EN;

    // Disable DAC channels for configuration
    DAC->CR &= ~(DAC_CR_EN1 | DAC_CR_EN2);

    // Set DAC to normal mode (no buffer)
    DAC->MCR &= ~DAC_MCR_MODE1;

    // Set DAC trigger source (using TIM4 TRGO as the trigger)
    DAC->CR |= DAC_CR_TSEL1;

    // Enable DAC Channel 1
    DAC->CR |= DAC_CR_EN1;
}

// Initialize Timer 4 to trigger DAC updates at 44.1 kHz
void TIM4_Init(void) {
    // Enable clock for TIM4
    RCC->APB1ENR1 |= RCC_APB1ENR1_TIM4EN;

    // Set TIM4 to edge-aligned mode and enable update interrupt
    TIM4->CR1 &= ~TIM_CR1_CMS;
    TIM4->CR2 &= ~TIM_CR2_MMS;
    TIM4->CR2 |= TIM_CR2_MMS_1;
    TIM4->DIER |= TIM_DIER_UIE;

    // Set the timer prescaler for a 44.1 kHz frequency (using HSI16 = 16 MHz clock)
    uint32_t prescaler = (16000000 / 44100) - 1;
    TIM4->PSC = prescaler;

    // Set the auto-reload value for the timer (this could be set to 1 for a 1-to-1 match with the prescaler)
    TIM4->ARR = 1;

    // Enable timer interrupt and start TIM4
    NVIC_SetPriority(TIM4_IRQn, 1);
    NVIC_EnableIRQ(TIM4_IRQn);
    TIM4->CR1 |= TIM_CR1_CEN; // Enable the timer
}

// Timer interrupt handler for TIM4
void TIM4_IRQHandler(void) {
    if (TIM4->SR & TIM_SR_UIF) {  // If update interrupt flag is set
        TIM4->SR &= ~TIM_SR_UIF;  // Clear the interrupt flag

        // Lookup the sine value corresponding to the current angle
        int sine_value = lookup_sine(angle / 1000);  // Convert angle to degrees

        // Output the sine value to the DAC
        DAC->DHR12R1 = sine_value;

        // Increment the angle for the next sample (wrap-around at 360 degrees)
        angle += 36000 / 44100;  // Increment angle for the next sample
        if (angle >= 360000) {
            angle -= 360000;  // Wrap around at 360 degrees
        }
    }
}

// Look up the sine value based on the current angle
int lookup_sine(int angle) {
    // Ensure the angle is within the valid range of 0 to 360 degrees
    angle = angle % 360;

    int index = angle % 90;  // Map angle to the first quadrant (0-90 degrees)

    // Use symmetry to mirror the sine wave:
    if (angle >= 0 && angle < 90) {
        return sin_table[index];  // 0 to 90 degrees
    } else if (angle >= 90 && angle < 180) {
        return 0x1000 - sin_table[180 - angle];  // 90 to 180 degrees
    } else if (angle >= 180 && angle < 270) {
        return 0x1000 - sin_table[angle - 180];  // 180 to 270 degrees
    } else {
        return sin_table[360 - angle];  // 270 to 360 degrees
    }
}
