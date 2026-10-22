#ifndef __KEY_H
#define __KEY_H

#include "main.h"

#define KEY0_GPIO_Port  GPIOE
#define KEY0_Pin        GPIO_PIN_2
#define KEY1_GPIO_Port  GPIOE
#define KEY1_Pin        GPIO_PIN_3
#define KEY2_GPIO_Port  GPIOE
#define KEY2_Pin        GPIO_PIN_4

/* 按键数量。 */
#define KEY_COUNT                 3U

/*
 * 按键时间参数，单位：ms。
 * KEY_Process需要在无阻塞主循环中持续调用，内部每10ms扫描一次。
 */
#define KEY_SCAN_PERIOD_MS       10U
#define KEY_DEBOUNCE_COUNT        2U
#define KEY_LONG_PRESS_MS      2000U
#define KEY_REPEAT_START_MS     600U
#define KEY_REPEAT_PERIOD_MS    100U

/*
 * 按键逻辑编号。
 * 使用CubeMX生成的SWx_Pin和SWx_GPIO_Port，不在驱动内重新配置GPIO。
 *
 * KEY_ID_1：SW1(PE7)，PRG/返回
 * KEY_ID_2：SW2(PB1)，向上/增加
 * KEY_ID_3：SW3(PB0)，向下/减少
 * KEY_ID_4：SW4(PC5)，ENTER/确认
 */
typedef enum
{
    KEY_ID_1 = 0,
    KEY_ID_2,
    KEY_ID_3,
    KEY_ID_4,
    KEY_ID_COUNT
} KEY_Id;

/* 按键事件类型。 */
typedef enum
{
    KEY_EVENT_NONE = 0,      /* 无按键事件 */
    KEY_EVENT_PRESS,         /* 消抖确认后的按下事件 */
    KEY_EVENT_RELEASE,       /* 消抖确认后的释放事件 */
    KEY_EVENT_SHORT,         /* 短按并释放 */
    KEY_EVENT_LONG,          /* 长按2秒，仅触发一次 */
    KEY_EVENT_REPEAT         /* 加减键按住后的连续触发 */
} KEY_EventType;

/* 按键事件数据。 */
typedef struct
{
    KEY_Id id;               /* 产生事件的按键 */
    KEY_EventType type;      /* 事件类型 */
} KEY_Event;

/*
 * 函数：KEY_Init
 * 功能：初始化按键软件状态和事件队列，不配置GPIO。
 */
void KEY_Init(void);

/*
 * 函数：KEY_Process
 * 功能：扫描按键、消抖并产生短按、长按和连续触发事件。
 * 调用：放在主循环持续调用，函数内部自动控制10ms扫描周期。
 */
void KEY_Process(void);

/*
 * 函数：KEY_GetEvent
 * 功能：从事件队列取出一个按键事件。
 * 返回：有事件返回1，队列为空或参数错误返回0。
 */
uint8_t KEY_GetEvent(KEY_Event *event);

/*
 * 函数：KEY_IsPressed
 * 功能：获取指定按键消抖后的稳定状态。
 * 返回：按下返回1，释放或按键编号错误返回0。
 */
uint8_t KEY_IsPressed(KEY_Id id);

/*
 * 函数：KEY_ClearEvents
 * 功能：清空尚未处理的全部按键事件。
 */
void KEY_ClearEvents(void);

#endif /* __KEY_H */
