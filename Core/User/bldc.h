#ifndef __BLDC_H
#define __BLDC_H

#include "main.h"

#define MOTOR_1   1


#define M1_UL_PORT GPIOB
#define M1_UL_PIN  GPIO_PIN_13
#define M1_VL_PORT GPIOB
#define M1_VL_PIN  GPIO_PIN_14
#define M1_WL_PORT GPIOB
#define M1_WL_PIN  GPIO_PIN_15
/***************************************** 半桥芯片***************************************************/

#define SHUTDOWN_PIN                      GPIO_PIN_10     /* PF10 */
#define SHUTDOWN_PIN_GPIO                 GPIOF
#define SHUTDOWN_PIN_GPIO_CLK_ENABLE()    do{  __HAL_RCC_GPIOF_CLK_ENABLE(); }while(0)    /* PF¿ÚÊ±ÖÓÊ¹ÄÜ */

#define SHUTDOWN_EN                       HAL_GPIO_WritePin(SHUTDOWN_PIN_GPIO,SHUTDOWN_PIN,GPIO_PIN_SET);   /* Ê¹ÄÜ°ëÇÅÐ¾Æ¬µÄSDÒý½Å */
#define SHUTDOWN_OFF                      HAL_GPIO_WritePin(SHUTDOWN_PIN_GPIO,SHUTDOWN_PIN,GPIO_PIN_RESET); /* Ê§ÄÜ°ëÇÅÐ¾Æ¬µÄSDÒý½Å */


uint8_t BLDC_Process(void);
void Start_Motor1(void);
void BLDC_Init(void);

#endif
