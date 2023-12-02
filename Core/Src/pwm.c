#include "pwm.h"

#define DOT_DURATION 200    // Duration of a dot.
#define DASH_DURATION 800   // Duration of a slash.
#define ELEMENT_GAP 1000     // Space between elements (dot/slash) of a letter.
#define LETTER_GAP 1000      // Space Between letters.
#define WORD_GAP 1400       // Space between words.


void PWM_cont_Init(void) {
    // Enables the clock for the GPIO port and the timer TIM2.
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
    RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;

    // Configs PA1 as an alternative output for TIM2_CH2.
    GPIOA->MODER &= ~GPIO_MODER_MODE0;  // Cleans the bits of mode for PA1.
    GPIOA->MODER |= GPIO_MODER_MODE0_1; // Configs as alternative mode.
    GPIOA->AFR[0] |= (1 << (4 * 0));    // Configs AF1 for PA1 (TIM2_CH2).

    // Configs the timer TIM2.
    TIM2->PSC = 80-1;                     // Establish the pre scaler.
    TIM2->ARR = 65535;                  // Establish the period.
    TIM2->CCMR1 |= TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2; //  PWM mode.
    TIM2->CCER |= TIM_CCER_CC1E;        // Enables the channel 2.

    // Starts the timer.
    TIM2->CR1 |= TIM_CR1_CEN;
}

/**
  * @brief  PWM_SendDot() and PWM_SendDash() control the duration of the Morse code dot and dash, respectively.
  * PWM_SendSOS(), PWM_SendOpen(), and PWM_SendClose() generate the Morse code sequence for the respective words
  * using dots and dashes.
  * PWM_SetDutyCycle() is used to adjust the duty cycle of the PWM signal for creating the Morse code sound or light pattern.
  * The Morse code representations are as follows:
  * SOS: ... --- ...
  * OPEN: --- .--. . -.
  * CLOSE: -.-. .-.. --- ... .
  */



// Function to send a Morse code dot (.)
static void PWM_SendDot(void) {
    PWM_SetDutyCycle(60000); // Adjust the duty cycle for the dot.
    HAL_Delay(DOT_DURATION);
    PWM_SetDutyCycle(0); // Off.
    HAL_Delay(ELEMENT_GAP);
}

// Function to send a Morse code dash (-)
static void PWM_SendDash(void) {
    PWM_SetDutyCycle(60000); // Adjust the duty cycle for the slash.
    HAL_Delay(DASH_DURATION);
    PWM_SetDutyCycle(0); // Off.
    HAL_Delay(ELEMENT_GAP);
}

// Function to send the Morse code for SOS (...)
void PWM_SendSOS(void) {
    // Sends  S (...)
    PWM_SendDot();
    PWM_SendDot();
    PWM_SendDot();
    HAL_Delay(LETTER_GAP - ELEMENT_GAP); // Gap between letters.
    //Sends  O (---)
    PWM_SendDash();
    PWM_SendDash();
    PWM_SendDash();
    HAL_Delay(LETTER_GAP - ELEMENT_GAP);

    // Sends S (...)
    PWM_SendDot();
    PWM_SendDot();
    PWM_SendDot();
    HAL_Delay(WORD_GAP - ELEMENT_GAP); // Gap between words.
}


// Function to send the Morse code for OPEN (--- .--. . -.)
void PWM_SendOpen(void) {
    // Sends O (---)
    PWM_SendDash();
    PWM_SendDash();
    PWM_SendDash();
    HAL_Delay(LETTER_GAP - ELEMENT_GAP);

    // Sends P (.--.)
    PWM_SendDot();
    PWM_SendDash();
    PWM_SendDash();
    PWM_SendDot();
    HAL_Delay(LETTER_GAP - ELEMENT_GAP);

    // Sends E (.)
    PWM_SendDot();
    HAL_Delay(LETTER_GAP - ELEMENT_GAP);

    // Sends N (-.)
    PWM_SendDash();
    PWM_SendDot();
    HAL_Delay(WORD_GAP - ELEMENT_GAP); // Gap between words.
}


// Function to send the Morse code for CLOSE (-.-. .-.. --- ...)
void PWM_SendClose(void) {
    // Sends C (-.-.)
    PWM_SendDash();
    PWM_SendDot();
    PWM_SendDash();
    PWM_SendDot();
    HAL_Delay(LETTER_GAP - ELEMENT_GAP);

    // Sends L (.-..)
    PWM_SendDot();
    PWM_SendDash();
    PWM_SendDot();
    PWM_SendDot();
    HAL_Delay(LETTER_GAP - ELEMENT_GAP);

    // Sends O (---)
    PWM_SendDash();
    PWM_SendDash();
    PWM_SendDash();
    HAL_Delay(LETTER_GAP - ELEMENT_GAP);

    // Sends S (...)
    PWM_SendDot();
    PWM_SendDot();
    PWM_SendDot();
    HAL_Delay(LETTER_GAP - ELEMENT_GAP);

    // Sends E (.)
    PWM_SendDot();
    HAL_Delay(WORD_GAP - ELEMENT_GAP);
}

// Function to set the duty cycle for the PWM signal
void PWM_SetDutyCycle(uint32_t dutyCycle) {

	// Clamp duty cycle to maximum value if needed
    if (dutyCycle > 65535) {
        dutyCycle = 65535;
    }

    // Set the duty cycle for TIM2 Channel 2
    TIM2->CCR1 = dutyCycle;
}
