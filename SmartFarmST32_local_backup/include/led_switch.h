#ifndef LED_SWITCH_H
#define LED_SWITCH_H
/**
 * @brief 초기화 및 GPIO를 설정합니다.
 * 
 */
void led_switch_init();
/**
 * @brief ONBOARD_SWITCH 홀드에 따라 LED를 동작합니다.
 * 
 */
void led_switch_hold();
/**
 * @brief ONBOARD_SWITCH 토글에 따라 LED를 동작합니다.
 * 
 */
void led_switch_toggle();
/**
 * @brief LED를 일정 주기로 점멸시킵니다.
 * 
 */
void Flash_LED();


#endif
