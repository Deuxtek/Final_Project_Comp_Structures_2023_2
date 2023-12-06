#include "lock.h"
#include "ring_buffer.h"
#include "keypad.h"
#include "main.h"
#include "gui.h"
#include "PWM_cont.h"
#include "ssd1306.h"

#include <stdio.h>
#include <string.h>


#define MAX_PASSWORD 12 // Define the maximum length of the password


uint8_t password[MAX_PASSWORD] = "2000"; // Default password
uint8_t keypad_buffer[MAX_PASSWORD]; // Buffer for keypad input
uint8_t message[MAX_PASSWORD]; // Buffer for messages

ring_buffer_t keypad_rb; // Ring buffer for keypad
ring_buffer_t Rx_Data; // Ring buffer for USART data

extern volatile uint16_t keypad_event; // External variable for keypad event
/*
 * @breif Function to receive data from USART1
 */
uint8_t Rx_USART1(void) {
    if (LL_USART_IsActiveFlag_RXNE(USART1)) {
        uint8_t received_data = LL_USART_ReceiveData8(USART1);
        ring_buffer_put(&Rx_Data, received_data); // Put data into ring buffer
    }
    return 1;
}
/*
 * @breif Function to get a key press or USART data
 * This function checks for input either from a keypad or via USART,
 * and returns the key pressed or data received.
 */
static uint8_t lock_get_passkey(void) {
    uint8_t key_pressed = KEY_PRESSED_NONE; // Variable to store the key pressed, initialized to no key pressed
    uint8_t Data_Rx = KEY_PRESSED_NONE;   // Variable to store data received from USART, initialized to no data
    uint8_t valid_input_received = 0;  // Flag to indicate if valid input is received

    while (!valid_input_received) {
    	// Handling keypad input
        key_pressed = keypad_run(&keypad_event); // Check if a key is pressed on the keypad
        if (key_pressed != KEY_PRESSED_NONE) {
            ring_buffer_put(&keypad_rb, key_pressed); // Put the key pressed into the ring buffer
            valid_input_received = 1; // Set the flag indicating valid input is received
        }

        // Handling USART input
        Rx_USART1(); // Check for data received via USART
        if (ring_buffer_size(&Rx_Data) > 0) {
            ring_buffer_get(&Rx_Data, &Data_Rx);  // Get the data from USART buffer
            ring_buffer_put(&keypad_rb, Data_Rx);  // Put the data received into the ring buffer
            valid_input_received = 1;   // Set the flag indicating valid input is received
        }
    }
    // Retrieve the pressed key or USART data from the buffer
    if (ring_buffer_size(&keypad_rb) > 0) {
        ring_buffer_get(&keypad_rb, &key_pressed);
    }

    // Check for special characters such as '*' or '#'
    if (key_pressed == '*' || key_pressed == '#' || Data_Rx == '*' || Data_Rx == '#') {
        return 0xFF;
    }
    return key_pressed;
}
/*
 * @brief Function to get a new password from the user.
 * This function prompts the user to enter a new password, captures the input,
 * and updates the stored password if the input is valid.
 */
static uint8_t lock_get_password(void)
{
    ring_buffer_reset(&keypad_rb); // Reset the keypad ring buffer to start fresh
    uint8_t idx = 0;               // Index for iterating through password characters
    uint8_t passkey = 0;           // Variable to store each input character
    uint8_t new_password[MAX_PASSWORD]; // Array to store the new password
    memset(new_password, 0, MAX_PASSWORD); // Initialize the new_password array with 0s

    uint8_t password_shadow[MAX_PASSWORD + 1] = {
        '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '\0'
    };

    while (passkey != 0xFF) { // Continue until a special character (0xFF) is received
        GUI_update_password(password_shadow); // Update the GUI to show the password shadow
        passkey = lock_get_passkey();         // Get the next key press or USART input
        password_shadow[idx] = '*';           // Replace the shadow character with '*' to indicate input
        new_password[idx++] = passkey;        // Store the actual input in the new_password array
        GUI_update_password(new_password);    // Update the GUI to show the new password
        HAL_Delay(200);
    }

    if (idx > 1) {
        memcpy(password, new_password, MAX_PASSWORD);
        GUI_update_password_success(); // Show success message on the GUI
        HAL_Delay(500);
        GUI_Kirby();                   // Return to the Kirby screen
    } else {
        GUI_locked();
        HAL_Delay(500);
        GUI_Kirby();
        return 0;
    }
    return 1; // Return 1 indicating success
}

/*
 * Function to validate the entered password.
 * This function checks if the sequence of inputs in the keypad ring buffer
 * matches the stored password.
 *
 * @return uint8_t - Returns 1 if the password is correct, 0 otherwise.
 */
static uint8_t lock_validate_password(void)
{
	uint8_t sequence[MAX_PASSWORD]; // Array to store the input sequence
	uint8_t seq_len = ring_buffer_size(&keypad_rb); // Get the number of elements in the ring buffer
	for (uint8_t idx = 0; idx < seq_len; idx++) {
		ring_buffer_get(&keypad_rb, &sequence[idx]);
	}
	if (memcmp(sequence, password, 4) == 0) {

		return 1; // Return 1 if the first 4 characters of the entered sequence match the password
	}
	return 0; // Return 0 if the password does not match
}
/*
 * @brief Function to update the password.
 * This function first validates the current password. If valid, it allows the user to set a new password.
 * Otherwise, it indicates that the lock is still locked and closes the PWM (Pulse Width Modulation) control.
 */
static void lock_update_password(void)
{
	if (lock_validate_password() != 0) {
		GUI_update_password_init();
		lock_get_password();
		PWM_SendOpen();
	} else {
		GUI_locked();
		PWM_SendClose();
		HAL_Delay(500);

	}
}
/*
 * @brief Function to open the lock if the password is valid.
 * This function attempts to validate the entered password. If the password is correct,
 * it triggers actions to unlock the lock. Otherwise, it maintains the lock in a locked state.
 */
static void lock_open_lock(void)
{
	if (lock_validate_password() != 0) {
		GUI_unlocked();
		PWM_SendOpen();

	} else {
		GUI_locked();
		PWM_SendClose();
		HAL_Delay(500);
		GUI_Kirby();
}
}
/*
 * @brief Function to open the lock if the password is valid.
 * This function attempts to validate the entered password. If the password is correct,
 * it triggers actions to unlock the lock. Otherwise, it maintains the lock in a locked state.
 */
void lock_init(void)
{
	LL_USART_EnableIT_RXNE(USART1);
	ring_buffer_init(&Rx_Data, message, 12);
	ring_buffer_init(&keypad_rb, keypad_buffer, 12);
	GUI_init();
	PWM_cont_Init();

}
/*
 * @brief Handle the sequence of key presses.
 * This function processes key presses for different functionalities like updating the password,
 * opening the lock, or stopping the PWM. It also updates the display based on the key pressed.
 */
void lock_sequence_handler(uint8_t key)
{


	if (key == '*') {
		lock_update_password();

	} else if (key == '#') {
		lock_open_lock();



	}else if(key == 'D'){
		PWM_Stop(); // Call function to stop PWM control
	}

}

uint8_t ultrasonicSensorEnabled = 0; // Variable to check the state of the ultrasonic sensor.
/*
 * @brief Control the ultrasonic sensor based on key input.
 * This function enables or disables the ultrasonic sensor depending on the key input received.
 *
 * @param key The key input used to control the ultrasonic sensor.
 */
void lock_control_ultrasonic_sensor(uint8_t key) {
    if (key == 'B') {
    	// If the key 'B' is pressed, enable the ultrasonic sensor
    	ultrasonicSensorEnabled = 1; // Set the flag to indicate the ultrasonic sensor is enabled
        return ;
    } else if (key == 'C') { // If the key 'C' is pressed, turn off the ultrasonic sensor
        // turn off the sensor
    	PWM_Stop();
    	ultrasonicSensorEnabled = 0;
        return ;
    }
    // Note: The function assumes the existence of a global variable 'ultrasonicSensorEnabled'
    // that tracks the state of the ultrasonic sensor.
}
/*
 * @brief Check if there is data in the USART1 buffer.
 * This function checks whether there is any data received and stored in the USART1 buffer.
 *
 * @return uint8_t - Returns 1 if there is data in the buffer, 0 otherwise.
 */
uint8_t Flag_USART1(void){
	uint8_t key_Rx;
	if (ring_buffer_get(&Rx_Data, &key_Rx)) {
			return 1;
	}
	return 0; // Si hay datos en el buffer
}
/*
 * @brief Handle lock sequence based on USART data.
 * This function retrieves a key from the USART data buffer and then processes it
 * through the lock sequence handler and ultrasonic sensor control function.
 */
void lock_sequence(void){
	uint8_t key_R;
	ring_buffer_get(&Rx_Data, &key_R);
	lock_sequence_handler(key_R);
	 // Process the key through the lock sequence handler
	 // This function handles different functionalities based on the key input, such as updating
	 // the password, opening the lock, or updating the display.
	lock_control_ultrasonic_sensor(key_R);
	 // Control the ultrasonic sensor based on the key input
	 // This function enables or disables the ultrasonic sensor depending on the received key.
}




