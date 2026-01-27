/**
 * @file tla2528.h
 * @author Kwon Jinwoo (jinwoo@xslab.co.kr)
 * @brief TI TLA2528 Analog to Digital Converter
 * @date 2025-08-22
 */

#ifndef INCLUDE_TLA2528
#define INCLUDE_TLA2528

#include <stdio.h>
#include <stdint.h>
#include <board_io.h>
#include <stm32g0xx.h>

class TLA2528
{
public:
	TLA2528() = default;
	TLA2528(I2C_HandleTypeDef *i2c, uint8_t target_address, float vref_volt);

	void init(I2C_HandleTypeDef *i2c, uint8_t target_address, float vref_volt);

	/**
	 * @brief 지정된 채널의 전압을 읽습니다
	 * 
	 * @param[in]	index	대상 채널입니다 (0 ~ 7)
	 * @return 전압입니다
	 */
	float get_Voltage(uint8_t index);

	/**
	 * @brief ADC 채널의 전압 값을 읽고, 상태를 업데이트합니다
	 * 
	 * @retval 0	성공
	 * @retval 1	실패
	 */
	uint8_t update_Value();

private:
	I2C_HandleTypeDef *i2c;
	uint8_t target_address;
	float vref_volt;

	static constexpr uint8_t channel_count = 8;
	static constexpr uint8_t average_count = 200;

	float input_voltage[channel_count] = {};
	uint32_t raw_value[channel_count] = {};
	uint8_t sample_count = 0;
};

#endif /* INCLUDE_TLA2528 */
