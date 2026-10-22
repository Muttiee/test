#ifndef __LED_H
#define __LED_H

#include "main.h"

#define LED0_GPIO_Port  GPIOE
#define LED0_Pin        GPIO_PIN_0
#define LED1_GPIO_Port  GPIOE
#define LED1_Pin        GPIO_PIN_1

#define LED0_ON()      HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, GPIO_PIN_RESET)
#define LED0_OFF()     HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, GPIO_PIN_SET)
#define LED1_ON()      HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET)
#define LED1_OFF()     HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET)


#endif
