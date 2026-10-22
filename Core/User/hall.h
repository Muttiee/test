#ifndef __HALL_H
#define __HALL_H

#include "main.h"

#define HALLU_GPIO_Port  GPIOH
#define HALLU_Pin        GPIO_PIN_10
#define HALLV_GPIO_Port  GPIOH
#define HALLV_Pin        GPIO_PIN_11
#define HALLW_GPIO_Port  GPIOH
#define HALLW_Pin        GPIO_PIN_12

#define HALLU_READ()      HAL_GPIO_ReadPin(HALLU_GPIO_Port, HALLU_Pin)
#define HALLV_READ()      HAL_GPIO_ReadPin(HALLV_GPIO_Port, HALLV_Pin)
#define HALLW_READ()      HAL_GPIO_ReadPin(HALLW_GPIO_Port, HALLW_Pin)





uint8_t GetHall_Value(void);

#endif
