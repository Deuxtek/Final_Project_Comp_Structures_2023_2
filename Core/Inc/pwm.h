/*
 * pwm.h
 *
 *  Created on: Nov 26, 2023
 *      Author: braya
 */

#ifndef INC_PWM_H_
#define INC_PWM_H_

#include "stm32l4xx_hal.h"

void PWM_cont_Init(void);
void PWM_SendSOS(void);
void PWM_SendOpen(void);
void PWM_SendClose(void);
void PWM_SetDutyCycle(uint32_t dutyCycle);


#endif /* INC_PWM_H_ */
