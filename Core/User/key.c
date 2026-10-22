#include "key.h"

#include <stddef.h>

/* 按键事件队列容量。 */
#define KEY_QUEUE_SIZE           16U

/* 按键硬件信息，仅引用CubeMX生成的端口和引脚宏。 */
typedef struct
{
    GPIO_TypeDef *port;          /* GPIO端口 */
    uint16_t pin;                /* GPIO引脚 */
    uint8_t repeat_enabled;      /* 是否允许按住连续触发 */
} KEY_Hardware;

/* 单个按键的软件运行状态。 */
typedef struct
{
    uint8_t sample;              /* 当前采样状态：1按下，0释放 */
    uint8_t stable;              /* 消抖后的稳定状态 */
    uint8_t filter_count;        /* 连续相同采样计数 */
    uint8_t wait_release;        /* 上电时按住则等待首次释放 */
    uint8_t long_sent;           /* 长按事件已经发送 */
    uint8_t repeat_started;      /* 已经进入连续触发状态 */
    uint32_t press_tick;         /* 确认按下的时间 */
    uint32_t repeat_tick;        /* 上一次连续触发的时间 */
} KEY_State;

/*
 * 按键硬件映射表。
 * SW1：PRG/返回；SW2：向上/增加；SW3：向下/减少；SW4：ENTER/确认。
 * 只有SW2和SW3允许按住连续触发。
 */
static const KEY_Hardware s_hardware[KEY_ID_COUNT] =
{
    {KEY0_GPIO_Port, KEY0_Pin, 0U},
    {KEY1_GPIO_Port, KEY1_Pin, 1U},
    {KEY2_GPIO_Port, KEY2_Pin, 1U},
};

/* 按键状态和事件队列。 */
static KEY_State s_keys[KEY_ID_COUNT];
static KEY_Event s_queue[KEY_QUEUE_SIZE];
static uint8_t s_queue_head;
static uint8_t s_queue_tail;
static uint32_t s_last_scan_tick;

/*
 * 函数：key_read
 * 功能：读取指定按键的原始状态。
 * 说明：按键低电平有效，因此按下返回1，释放返回0。
 */
static uint8_t key_read(KEY_Id id)
{
    return (HAL_GPIO_ReadPin(s_hardware[id].port,
                             s_hardware[id].pin) == GPIO_PIN_RESET) ? 1U : 0U;
}

/*
 * 函数：key_push_event
 * 功能：向事件队列写入一个按键事件。
 * 说明：队列满时丢弃最早事件，优先保留最新操作。
 */
static void key_push_event(KEY_Id id, KEY_EventType type)
{
    uint8_t next = (uint8_t)((s_queue_head + 1U) % KEY_QUEUE_SIZE);

    if (next == s_queue_tail)
    {
        s_queue_tail = (uint8_t)((s_queue_tail + 1U) % KEY_QUEUE_SIZE);
    }

    s_queue[s_queue_head].id = id;
    s_queue[s_queue_head].type = type;
    s_queue_head = next;
}

/*
 * 函数：key_on_pressed
 * 功能：记录按下时间并产生按下事件。
 */
static void key_on_pressed(KEY_Id id, uint32_t now_tick)
{
    KEY_State *key = &s_keys[id];

    key->press_tick = now_tick;
    key->repeat_tick = now_tick;
    key->long_sent = 0U;
    key->repeat_started = 0U;
    key_push_event(id, KEY_EVENT_PRESS);
}

/*
 * 函数：key_on_released
 * 功能：产生释放事件，并判断是否属于短按操作。
 */
static void key_on_released(KEY_Id id)
{
    KEY_State *key = &s_keys[id];

    key_push_event(id, KEY_EVENT_RELEASE);

    /* 长按或连续触发结束后，不再额外产生短按事件。 */
    if ((key->long_sent == 0U) && (key->repeat_started == 0U))
    {
        key_push_event(id, KEY_EVENT_SHORT);
    }
}

/*
 * 函数：key_process_hold
 * 功能：处理长按和加减键的连续触发。
 */
static void key_process_hold(KEY_Id id, uint32_t now_tick)
{
    KEY_State *key = &s_keys[id];
    uint32_t elapsed = (uint32_t)(now_tick - key->press_tick);

    if ((key->long_sent == 0U) && (elapsed >= KEY_LONG_PRESS_MS))
    {
        key->long_sent = 1U;
        key_push_event(id, KEY_EVENT_LONG);
    }

    if ((s_hardware[id].repeat_enabled != 0U) &&
        (elapsed >= KEY_REPEAT_START_MS) &&
        ((uint32_t)(now_tick - key->repeat_tick) >= KEY_REPEAT_PERIOD_MS))
    {
        key->repeat_tick = now_tick;
        key->repeat_started = 1U;
        key_push_event(id, KEY_EVENT_REPEAT);
    }
}

/*
 * 函数：key_scan
 * 功能：扫描全部按键，完成消抖、边沿和长按检测。
 */
static void key_scan(uint32_t now_tick)
{
    KEY_Id id;

    for (id = KEY_ID_1; id < KEY_ID_COUNT; id = (KEY_Id)(id + 1))
    {
        KEY_State *key = &s_keys[id];
        uint8_t sample = key_read(id);

        /* 连续采样一致达到设定次数后，才确认状态改变。 */
        if (sample == key->sample)
        {
            if (key->filter_count < KEY_DEBOUNCE_COUNT)
            {
                key->filter_count++;
            }
        }
        else
        {
            key->sample = sample;
            key->filter_count = 1U;
        }

        if ((key->filter_count >= KEY_DEBOUNCE_COUNT) &&
            (key->stable != key->sample))
        {
            key->stable = key->sample;

            /* 上电时已经按住的按键必须先释放，防止误触发。 */
            if (key->wait_release != 0U)
            {
                if (key->stable == 0U)
                {
                    key->wait_release = 0U;
                }
            }
            else if (key->stable != 0U)
            {
                key_on_pressed(id, now_tick);
            }
            else
            {
                key_on_released(id);
            }
        }

        if ((key->stable != 0U) && (key->wait_release == 0U))
        {
            key_process_hold(id, now_tick);
        }
    }
}

/*
 * 函数：KEY_Init
 * 功能：初始化按键软件状态和事件队列。
 * 说明：GPIO已经由CubeMX初始化，本函数不再配置GPIO。
 */
void KEY_Init(void)
{
    KEY_Id id;

    s_queue_head = 0U;
    s_queue_tail = 0U;
    s_last_scan_tick = HAL_GetTick();

    for (id = KEY_ID_1; id < KEY_ID_COUNT; id = (KEY_Id)(id + 1))
    {
        KEY_State *key = &s_keys[id];
        uint8_t pressed = key_read(id);

        key->sample = pressed;
        key->stable = pressed;
        key->filter_count = KEY_DEBOUNCE_COUNT;
        key->wait_release = pressed;
        key->long_sent = 0U;
        key->repeat_started = 0U;
        key->press_tick = s_last_scan_tick;
        key->repeat_tick = s_last_scan_tick;
    }
}

/*
 * 函数：KEY_Process
 * 功能：每10ms执行一次实际按键扫描，调用本身不会阻塞。
 */
void KEY_Process(void)
{
    uint32_t now_tick = HAL_GetTick();

    if ((uint32_t)(now_tick - s_last_scan_tick) < KEY_SCAN_PERIOD_MS)
    {
        return;
    }

    s_last_scan_tick = now_tick;
    key_scan(now_tick);
}

/*
 * 函数：KEY_GetEvent
 * 功能：按产生顺序取出一个事件，读取后自动从队列删除。
 */
uint8_t KEY_GetEvent(KEY_Event *event)
{
    if ((event == NULL) || (s_queue_head == s_queue_tail))
    {
        return 0U;
    }

    *event = s_queue[s_queue_tail];
    s_queue_tail = (uint8_t)((s_queue_tail + 1U) % KEY_QUEUE_SIZE);
    return 1U;
}

/*
 * 函数：KEY_IsPressed
 * 功能：返回指定按键消抖后的稳定状态。
 */
uint8_t KEY_IsPressed(KEY_Id id)
{
    if ((uint32_t)id >= KEY_ID_COUNT)
    {
        return 0U;
    }

    return s_keys[id].stable;
}

/*
 * 函数：KEY_ClearEvents
 * 功能：清空所有尚未处理的事件。
 */
void KEY_ClearEvents(void)
{
    s_queue_tail = s_queue_head;
}
