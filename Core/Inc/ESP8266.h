/*
 * esp8266.h
 *
 *  Created on: Nov 26, 2023
 *      Author: braya
 */

#ifndef INC_ESP8266_H_
#define INC_ESP8266_H_
#include <stdint.h>

typedef struct {
	uint8_t  *buffer;  /*!\ Pointer to memory location */
	uint16_t head;     /*!\ To write new data */
	uint16_t tail;     /*!\ To read old data */

	uint16_t capacity;
	uint8_t  is_full;

} USART1_t;

void Rx_USART1_init(USART1_t *ring_buffer, uint8_t *buffer, uint16_t capacity);
uint8_t Rx_USART1_put(USART1_t *ring_buffer, uint8_t data);
void Rx_USART1(USART1_t *ring_buffer);

#endif /* INC_ESP8266_H_ */
