#include <board_io.h>
#include <led_switch.h>

/** 포트 묶음 F를 ONBOARD_SWITCH_PORT로 별칭을 설정합니다. */
#define ONBOARD_SWITCH_PORT GPIOF

/** pin 3를 ONBOARD_SWITCH_PIN로 별칭을 설정합니다. */
#define ONBOARD_SWITCH_PIN  GPIO_PIN_3

void Flash_LED(uint32_t led_period_ms){
	static bool led_status = false;
	static uint32_t last_tick_led=0;
	uint32_t current_tick_led=HAL_GetTick();
	if (current_tick_led-last_tick_led>=led_period_ms){
		led_status ^= true;
		last_tick_led = current_tick_led;
		set_Status_LED(STATUS_LED_TYPE::YELLOW, led_status);
	}

}

void led_switch_init()
{
/** GPIO 설정용 구조체 내의 값 초기화 및 핀을 설정합니다. */
    GPIO_InitTypeDef gpio_cfg{0};
    gpio_cfg.Pin  = ONBOARD_SWITCH_PIN;

/** 포트 설정 및 설정 구조체의 주소를 인가합니다. */
    HAL_GPIO_Init(ONBOARD_SWITCH_PORT, &gpio_cfg);

}

void led_switch_hold()
{
/** PF3 핀의 값을 읽은 후 0과 비교 연산하여  hold를 조절합니다 */
    bool hold= (HAL_GPIO_ReadPin(ONBOARD_SWITCH_PORT, ONBOARD_SWITCH_PIN) == 0);

/** hold에 따라 OnBoard Led를 조절합니다. */
    set_Status_LED(STATUS_LED_TYPE::YELLOW, hold);
}

void led_switch_toggle()
{

/** Yellow_Led_State 초기 상태 및 토글 상태 확인을 위한
 *  Last_Toggle_State 초기 상태를 설정합니다. */    
    static bool Yellow_Led_State=false;
    static bool Last_Toggle_State=false;

/** PF3 핀의 값을 읽은 후 0과 비교 연산하여 스위치가 눌렸는지 여부를
 *  toggle 에 저장합니다. */
    bool toggle = (HAL_GPIO_ReadPin(ONBOARD_SWITCH_PORT, ONBOARD_SWITCH_PIN) == 0);
 
/** switch가 계속 on 인 상태에서 Yellow_Led_State가 여러번 반전되지 않기 위해
 *  toggle과 Last_Toggle_State가 다를 경우에만 Yellow_Led_State를
 *  반전시킵니다. */
    if (toggle && !Last_Toggle_State) {
        Yellow_Led_State = !Yellow_Led_State;
    }

/** 이후에 비교할 수 있도록 현재 스위치의 상태를 저장합니다. */
    Last_Toggle_State=toggle;

/** Yellow_Led_State 값에 따라 OnBoard Led를 조절합니다. */
    set_Status_LED(STATUS_LED_TYPE::YELLOW, Yellow_Led_State);

    
}