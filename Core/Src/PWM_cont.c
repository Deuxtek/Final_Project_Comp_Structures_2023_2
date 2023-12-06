#include "PWM_cont.h"
#include "stm32l4xx_hal.h"


// Time definitions for Morse code signaling.
#define DOT_DURATION 150 	// Duration of a dot in Morse code
#define DASH_DURATION 800	// Duration of a dash in Morse code
#define ELEMENT_GAP 500		 // Gap between elements (dot or dash)
#define LETTER_GAP 1000		// Gap between letters
#define WORD_GAP 1400		// Gap between words


// Enum for the state machine of the PWM Morse signaling.
typedef enum {
    PWM_STATE_IDLE, // Idle state
    PWM_STATE_DOT,  // State for signaling a dot
    PWM_STATE_DASH,	// State for signaling a dash
    PWM_STATE_GAP,	// State for handling the gap between elements
    PWM_STATE_END	// End of the Morse sequence
} PWM_State;

// Structure for managing the Morse sequence.
typedef struct {
    PWM_State state;			// Current state of the Morse controller
    uint32_t lastUpdateTime;	// Last update time
    const char *sequence;		// Pointer to the Morse code sequence
    uint32_t sequenceIndex;		// Current index in the Morse code sequence
} PWM_Morse;

static PWM_Morse morseController = {PWM_STATE_IDLE, 0, NULL, 0};


// Function prototypes for internal use.
static void PWM_SetDutyCycle(uint32_t dutyCycle);
static void PWM_UpdateState(void);


/**
 * @brief Initializes the PWM controller for Morse code signaling
 */
void PWM_cont_Init(void) {
	 // Enables the clock for the GPIO port and the timer TIM2.
	    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
	    RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;

	    // Configs PA0 as an alternative output for TIM2_CH1.
	    GPIOA->MODER &= ~GPIO_MODER_MODE0;  // Clears mode bits for PA0
	    GPIOA->MODER |= GPIO_MODER_MODE0_1; // Configs as alternative mode.
	    GPIOA->AFR[0] |= (1 << (4 * 0));    // Configs AF1 for PA0 (TIM2_CH1).

	    // Configs the timer TIM2.
	    TIM2->PSC = 79;                     // Establish the pre scaler.
	    TIM2->ARR = 999;                  // Establish the period.
	    TIM2->CCMR1 |= TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2; // PWM mode for CH1.
	    TIM2->CCER |= TIM_CCER_CC1E;        // Enables the channel 1.

	    // Starts the timer.
	    TIM2->CR1 |= TIM_CR1_CEN;
}


/**
 * @brief Starts sending an SOS signal using PWM.
 *    Initializes the Morse code controller for the SOS sequence.
 */
void PWM_SendSOS(void) {

    morseController.sequence = "...---..."; // Set Morse code for SOS ("...---...")
    morseController.sequenceIndex = 0;
    morseController.state = PWM_STATE_DOT;
    PWM_SetDutyCycle(60000); // Start PWM with the first symbol.
}


/**
 * @brief Sends the Morse code for "OPEN".
 */
void PWM_SendOpen(void) {
    morseController.sequence = "--- .--. . -."; // Set Morse code sequence for "OPEN".

    morseController.sequenceIndex = 0;
    morseController.state = PWM_STATE_DOT;
    PWM_SetDutyCycle(60000); // Start PWM with the first symbol.
}

/**
 * @brief Sends the Morse code for "CLOSE".
 */
void PWM_SendClose(void) {
    morseController.sequence = "-.-. .-.. --- ... ."; // Set Morse code sequence for "CLOSE".
    morseController.sequenceIndex = 0;
    morseController.state = PWM_STATE_DOT;
    PWM_SetDutyCycle(60000); // Start PWM with the first symbol.
}


/**
 * @brief Stops the PWM signaling.
 */
void PWM_Stop(void) {
	PWM_SetDutyCycle(0);  // Turns off  PWM.
	morseController.state = PWM_STATE_END;
}


/**
 * @brief Updates the state of the Morse code PWM signaling
 */
void PWM_Morse_Update(void) {

	// Get current time and calculate elapsed time since last update.
	    uint32_t currentTime = HAL_GetTick();
	    uint32_t elapsed = currentTime - morseController.lastUpdateTime;
	    char currentSymbol = morseController.sequence[morseController.sequenceIndex];

	    // Check if it's time to update the state based on the Morse code duration.

	    if ((morseController.state == PWM_STATE_DOT && elapsed >= DOT_DURATION) ||
	        (morseController.state == PWM_STATE_DASH && elapsed >= DASH_DURATION) ||
	        (morseController.state == PWM_STATE_GAP && elapsed >= ELEMENT_GAP)) {
	        PWM_UpdateState(); // Update the state of the Morse code controller.
	        morseController.lastUpdateTime = currentTime;
	    }
	}

static void PWM_UpdateState(void) {
	// Get the current symbol from the Morse code sequence.
	  char currentSymbol = morseController.sequence[morseController.sequenceIndex];

	  // Switch case for handling different states of the Morse code controller.
	    switch (morseController.state) {
	        case PWM_STATE_DOT:
	        case PWM_STATE_DASH:
	            PWM_SetDutyCycle(0); // Turn off PWM.
	            morseController.state = PWM_STATE_GAP; // Change to GAP state.
	            morseController.lastUpdateTime = HAL_GetTick();
	            break;

	        case PWM_STATE_GAP:

	        	// Process the next symbol if it exists.
	            if (currentSymbol != '\0') {
	                morseController.sequenceIndex++; // Move to the next symbol.
	                currentSymbol = morseController.sequence[morseController.sequenceIndex];
	            }

	            // Handle different symbols (dot, dash, end of sequence).
	            if (currentSymbol == '.') {
	                PWM_SetDutyCycle(60000); // Turn on for a dot.
	                morseController.state = PWM_STATE_DOT;
	            } else if (currentSymbol == '-') {
	                PWM_SetDutyCycle(60000); // Turn on for a dash.
	                morseController.state = PWM_STATE_DASH;
	            } else if (currentSymbol == '\0') { // End of the sequence.
	                morseController.state = PWM_STATE_END;
	            }
	            morseController.lastUpdateTime = HAL_GetTick();
	            break;

	        case PWM_STATE_END:
	            break;

	        default:
	            morseController.state = PWM_STATE_IDLE; // Default to idle state.
	            break;
	    }
	}

/**
 * @brief Sets the duty cycle for the PWM signal
 * @param dutyCycle: the duty cycle value to be set
 */
void PWM_SetDutyCycle(uint32_t dutyCycle) {
    // Clamp duty cycle to maximum value if needed
    if (dutyCycle > 65535) {
        dutyCycle = 65535;
    }

    // Set the duty cycle for TIM2 Channel 1
    TIM2->CCR1 = dutyCycle;
}
