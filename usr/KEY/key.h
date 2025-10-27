#ifndef __KEY_H__
#define __KEY_H__


#include <stdint.h>
#include <stdbool.h>
#include "main.h"
#ifdef __cplusplus
extern "C" {
#endif

// 按键事件类型定义
typedef enum {
    KEY_EVENT_NONE = 0,
    KEY_EVENT_PRESS,
    KEY_EVENT_RELEASE,
    KEY_EVENT_LONG_PRESS,
    KEY_EVENT_CLICK,
    KEY_EVENT_DOUBLE_CLICK
} key_event_t;

// 按键状态定义
typedef enum {
    KEY_STATE_IDLE = 0,
    KEY_STATE_DEBOUNCE,
    KEY_STATE_PRESSED,
    KEY_STATE_LONG_PRESS,
    KEY_STATE_RELEASED,
    KEY_STATE_CLICK_WAIT
} key_state_t;

// 按键配置结构
typedef struct {
    GPIO_TypeDef  *gpio_port;       // GPIO端口指针
    uint32_t gpio_pin;          // GPIO引脚掩码
    bool active_low;            // 是否低电平有效
    uint16_t debounce_time;     // 消抖时间(ms)
    uint16_t long_press_time;   // 长按时间(ms)
    uint16_t double_click_time; // 双击间隔时间(ms)
} key_config_t;

// 按键控制结构
typedef struct {
    key_config_t config;        // 按键配置
    key_state_t state;          // 当前状态
    key_event_t event;          // 当前事件
    uint16_t timer;             // 状态计时器
    bool last_level;            // 上次电平状态
    bool current_level;         // 当前电平状态
    uint8_t click_count;        // 点击计数
} key_control_t;

// 按键数量定义
#define KEY_MAX_NUM     3

// 时间常数定义(ms)
#define KEY_DEBOUNCE_TIME       20      // 消抖时间
#define KEY_LONG_PRESS_TIME     1000    // 长按时间
#define KEY_DOUBLE_CLICK_TIME   300     // 双击间隔时间
#define SYSTICK_PERIOD_MS       5       // SysTick中断周期

// 按键事件回调函数类型
typedef void (*key_event_callback_t)(uint8_t key_id, key_event_t event);

// 函数声明
void key_init(void);
uint8_t key_register(GPIO_TypeDef *gpio_port, uint32_t gpio_pin, bool active_low);
void key_set_callback(key_event_callback_t callback);
void key_scan(void);
void key_systick_handler(void);
uint32_t key_get_systick_count(void);

// 获取按键状态的函数
key_state_t key_get_state(uint8_t key_id);
key_event_t key_get_last_event(uint8_t key_id);
bool key_is_pressed(uint8_t key_id);

// 设置按键参数的函数
void key_set_debounce_time(uint8_t key_id, uint16_t time_ms);
void key_set_long_press_time(uint8_t key_id, uint16_t time_ms);
void key_set_double_click_time(uint8_t key_id, uint16_t time_ms);

#ifdef __cplusplus
}
#endif

#endif /* __KEY_H__ */