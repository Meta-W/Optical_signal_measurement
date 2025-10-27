#include "key.h"

// 全局变量
static key_control_t keys[KEY_MAX_NUM];
static uint8_t key_num = 0;
static volatile uint32_t systick_count = 0;
static key_event_callback_t key_event_callback = NULL;

// 静态函数声明
static bool key_read_pin(uint8_t key_id);
static void key_state_machine(uint8_t key_id);

/**
 * @brief 按键系统初始化
 */
void key_init(void)
{
    // 初始化按键数组
    for (int i = 0; i < KEY_MAX_NUM; i++) {
        keys[i].state = KEY_STATE_IDLE;
        keys[i].event = KEY_EVENT_NONE;
        keys[i].timer = 0;
        keys[i].last_level = false;
        keys[i].current_level = false;
        keys[i].click_count = 0;
        keys[i].config.gpio_port = NULL;
        keys[i].config.gpio_pin = 0;
        keys[i].config.active_low = true;
        keys[i].config.debounce_time = KEY_DEBOUNCE_TIME / SYSTICK_PERIOD_MS;
        keys[i].config.long_press_time = KEY_LONG_PRESS_TIME / SYSTICK_PERIOD_MS;
        keys[i].config.double_click_time = KEY_DOUBLE_CLICK_TIME / SYSTICK_PERIOD_MS;
    }
    
    key_num = 0;
    systick_count = 0;
    key_event_callback = NULL;
}

/**
 * @brief 注册按键
 * @param gpio_port GPIO端口指针
 * @param gpio_pin GPIO引脚掩码
 * @param active_low 是否低电平有效
 * @return 按键ID，如果注册失败返回0xFF
 */
uint8_t key_register(GPIO_TypeDef *gpio_port, uint32_t gpio_pin, bool active_low)
{
    if (key_num >= KEY_MAX_NUM || gpio_port == NULL) {
        return 0xFF; // 注册失败
    }
    
    uint8_t key_id = key_num++;
    
    // 配置按键参数
    keys[key_id].config.gpio_port = gpio_port;
    keys[key_id].config.gpio_pin = gpio_pin;
    keys[key_id].config.active_low = active_low;
    keys[key_id].config.debounce_time = KEY_DEBOUNCE_TIME / SYSTICK_PERIOD_MS;
    keys[key_id].config.long_press_time = KEY_LONG_PRESS_TIME / SYSTICK_PERIOD_MS;
    keys[key_id].config.double_click_time = KEY_DOUBLE_CLICK_TIME / SYSTICK_PERIOD_MS;
    
    // 初始化按键状态
    keys[key_id].state = KEY_STATE_IDLE;
    keys[key_id].event = KEY_EVENT_NONE;
    keys[key_id].timer = 0;
    keys[key_id].last_level = key_read_pin(key_id);
    keys[key_id].current_level = keys[key_id].last_level;
    keys[key_id].click_count = 0;
    
    return key_id;
}

/**
 * @brief 设置按键事件回调函数
 * @param callback 回调函数指针
 */
void key_set_callback(key_event_callback_t callback)
{
    key_event_callback = callback;
}

/**
 * @brief 按键扫描函数（在SysTick中断中调用）
 */
void key_scan(void)
{
    for (uint8_t i = 0; i < key_num; i++) {
        // 读取当前按键状态
        keys[i].current_level = key_read_pin(i);
        
        // 运行状态机
        key_state_machine(i);
        
        // 更新上次状态
        keys[i].last_level = keys[i].current_level;
    }
}

/**
 * @brief SysTick处理函数，需要在SysTick_Handler中调用
 */
void key_systick_handler(void)
{
    systick_count++;
    key_scan();
}

/**
 * @brief 获取系统滴答计数
 * @return 系统滴答计数
 */
uint32_t key_get_systick_count(void)
{
    return systick_count;
}

/**
 * @brief 读取按键引脚状态
 * @param key_id 按键ID
 * @return 按键逻辑状态
 */
static bool key_read_pin(uint8_t key_id)
{
    if (key_id >= key_num || keys[key_id].config.gpio_port == NULL) {
        return false;
    }
    
    bool pin_state = (HAL_GPIO_ReadPin(keys[key_id].config.gpio_port, 
                                       keys[key_id].config.gpio_pin) != 0);
    
    // 根据按键极性返回逻辑状态
    if (keys[key_id].config.active_low) {
        return !pin_state; // 低电平有效时取反
    } else {
        return pin_state;  // 高电平有效时直接返回
    }
}

/**
 * @brief 按键状态机
 * @param key_id 按键ID
 */
static void key_state_machine(uint8_t key_id)
{
    if (key_id >= key_num) {
        return;
    }
    
    key_control_t *key = &keys[key_id];
    
    // 清除事件
    key->event = KEY_EVENT_NONE;
    
    switch (key->state) {
        case KEY_STATE_IDLE:
            if (key->current_level && !key->last_level) {
                // 检测到按键按下
                key->state = KEY_STATE_DEBOUNCE;
                key->timer = 0;
            }
            break;
            
        case KEY_STATE_DEBOUNCE:
            if (key->current_level) {
                key->timer++;
                if (key->timer >= key->config.debounce_time) {
                    // 消抖完成，确认按键按下
                    key->state = KEY_STATE_PRESSED;
                    key->timer = 0;
                    key->event = KEY_EVENT_PRESS;
                }
            } else {
                // 按键释放，回到空闲状态
                key->state = KEY_STATE_IDLE;
            }
            break;
            
        case KEY_STATE_PRESSED:
            if (key->current_level) {
                key->timer++;
                if (key->timer >= key->config.long_press_time) {
                    // 长按触发
                    key->state = KEY_STATE_LONG_PRESS;
                    key->event = KEY_EVENT_LONG_PRESS;
                }
            } else {
                // 按键释放
                key->state = KEY_STATE_RELEASED;
                key->timer = 0;
                key->event = KEY_EVENT_RELEASE;
            }
            break;
            
        case KEY_STATE_LONG_PRESS:
            if (!key->current_level) {
                // 长按后释放
                key->state = KEY_STATE_IDLE;
                key->event = KEY_EVENT_RELEASE;
            }
            break;
            
        case KEY_STATE_RELEASED:
            key->timer++;
            
            if (key->current_level && !key->last_level) {
                // 再次按下，可能是双击
                key->click_count++;
                if (key->click_count == 1) {
                    // 第二次点击
                    key->state = KEY_STATE_IDLE;
                    key->event = KEY_EVENT_DOUBLE_CLICK;
                    key->click_count = 0;
                } else {
                    key->state = KEY_STATE_DEBOUNCE;
                    key->timer = 0;
                }
            } else if (key->timer >= key->config.double_click_time) {
                // 超时，确认为单击
                key->state = KEY_STATE_IDLE;
                key->event = KEY_EVENT_CLICK;
                key->click_count = 0;
            }
            break;
            
        case KEY_STATE_CLICK_WAIT:
            // 预留状态，可用于更复杂的点击判断
            key->state = KEY_STATE_IDLE;
            break;
            
        default:
            key->state = KEY_STATE_IDLE;
            break;
    }
    
    // 如果有事件产生且回调函数已注册，则调用回调函数
    if (key->event != KEY_EVENT_NONE && key_event_callback != NULL) {
        key_event_callback(key_id, key->event);
    }
}

/**
 * @brief 获取按键状态
 * @param key_id 按键ID
 * @return 按键状态
 */
key_state_t key_get_state(uint8_t key_id)
{
    if (key_id >= key_num) {
        return KEY_STATE_IDLE;
    }
    return keys[key_id].state;
}

/**
 * @brief 获取按键最后一次事件
 * @param key_id 按键ID
 * @return 按键事件
 */
key_event_t key_get_last_event(uint8_t key_id)
{
    if (key_id >= key_num) {
        return KEY_EVENT_NONE;
    }
    return keys[key_id].event;
}

/**
 * @brief 判断按键是否处于按下状态
 * @param key_id 按键ID
 * @return true-按下，false-未按下
 */
bool key_is_pressed(uint8_t key_id)
{
    if (key_id >= key_num) {
        return false;
    }
    return keys[key_id].current_level;
}

/**
 * @brief 设置按键消抖时间
 * @param key_id 按键ID
 * @param time_ms 消抖时间(毫秒)
 */
void key_set_debounce_time(uint8_t key_id, uint16_t time_ms)
{
    if (key_id >= key_num) {
        return;
    }
    keys[key_id].config.debounce_time = time_ms / SYSTICK_PERIOD_MS;
}

/**
 * @brief 设置按键长按时间
 * @param key_id 按键ID
 * @param time_ms 长按时间(毫秒)
 */
void key_set_long_press_time(uint8_t key_id, uint16_t time_ms)
{
    if (key_id >= key_num) {
        return;
    }
    keys[key_id].config.long_press_time = time_ms / SYSTICK_PERIOD_MS;
}

/**
 * @brief 设置按键双击间隔时间
 * @param key_id 按键ID
 * @param time_ms 双击间隔时间(毫秒)
 */
void key_set_double_click_time(uint8_t key_id, uint16_t time_ms)
{
    if (key_id >= key_num) {
        return;
    }
    keys[key_id].config.double_click_time = time_ms / SYSTICK_PERIOD_MS;
}