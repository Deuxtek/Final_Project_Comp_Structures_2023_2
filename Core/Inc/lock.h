/*
 * lock.h
 *
 *  Created on: Nov 20, 2023
 *      Author: saaci
 */

#ifndef INC_LOCK_H_
#define INC_LOCK_H_


#include <stdint.h>


void lock_init(void);
void lock_sequence_handler(uint8_t key);


#endif /* INC_LOCK_H_ */
