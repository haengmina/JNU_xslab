#include <board_io.h>
#include <switch_fan.h>

#define ONBOARD_SWITCH_PORT GPIOF

#define ONBOARD_SWITCH_PIN  GPIO_PIN_3

#define ONBOARD_12V_PORT GPIOD

#define ONBOARD_12V_PIN  GPIO_PIN_15


void Switch_Fan_init()
{
    GPIO_InitTypeDef gpio_cfg{0};
    gpio_cfg.Pin  = ONBOARD_SWITCH_PIN;
    HAL_GPIO_Init(ONBOARD_SWITCH_PORT, &gpio_cfg);
}

void Switch_Fan()
{

    if (HAL_GPIO_ReadPin(ONBOARD_SWITCH_PORT, ONBOARD_SWITCH_PIN) == 0){
        HAL_GPIO_WritePin(ONBOARD_12V_PORT, ONBOARD_12V_PIN, GPIO_PIN_SET);
    } else {
        HAL_GPIO_WritePin(ONBOARD_12V_PORT, ONBOARD_12V_PIN, GPIO_PIN_RESET);
    }

}
