/**
 * @file tla2528.h
 * @author Kwon Jinwoo (jinwoo@xslab.co.kr)
 * @brief TI TLA2528 Analog to Digital Converter
 * @date 2025-08-22
 */

#include <tla2528.h>

TLA2528::TLA2528(I2C_HandleTypeDef *i2c, uint8_t target_address, float vref_volt) : i2c(i2c), target_address(target_address), vref_volt(vref_volt)
{
	init(i2c, target_address, vref_volt);
}

void TLA2528::init(I2C_HandleTypeDef *i2c, uint8_t target_address, float vref_volt)
{
	this->i2c = i2c;
	this->target_address = target_address;
	this->vref_volt = vref_volt;

	uint8_t buffer[] = {0x08, 0x12, 0xFF}; //ADC auto sequence mode target channel: ALL
	if(HAL_I2C_Master_Transmit(i2c, target_address, buffer, sizeof(buffer), 100) != HAL_OK)
	{
		printf("ADC %s: ERROR\n", __FUNCTION__);
	}

	buffer[1] = 0x02;
	buffer[2] = 0x10; //Channel ID APPEND
	if(HAL_I2C_Master_Transmit(i2c, target_address, buffer, sizeof(buffer), 100) != HAL_OK)
	{
		printf("ADC %s: ERROR\n", __FUNCTION__);
	}
	
	buffer[1] = 0x10;
	buffer[2] = 0x11; //ADC auto sequence mode, start
	if(HAL_I2C_Master_Transmit(i2c, target_address, buffer, sizeof(buffer), 100) != HAL_OK)
	{
		printf("ADC %s: ERROR\n", __FUNCTION__);
	}
}

float TLA2528::get_Voltage(uint8_t index)
{
	if(index >= channel_count) return 0;

	return input_voltage[index];
}

uint8_t TLA2528::update_Value()
{
	uint8_t receive_buffer[2 * channel_count] = {};
	if(HAL_I2C_Master_Receive(i2c, target_address, receive_buffer, sizeof(receive_buffer), 100) != HAL_OK)
	{
		printf("ADC %s: ERROR\n", __FUNCTION__);
		return 1;
	}
	if(++sample_count == average_count)
	{
		sample_count = 1;
		for(auto &target : raw_value)
		{
			target = 0;
		}
	}
	for(uint8_t i = 0; i < sizeof(receive_buffer) / 2; i++)
	{
		uint8_t offset = i * 2;
		uint16_t value = (uint16_t)(receive_buffer[offset] << 8) | receive_buffer[offset + 1];

		uint8_t target_channel = (value & 0x000F);

		raw_value[target_channel] += (value >> 4);
		uint16_t target_value = raw_value[target_channel] / sample_count;

		input_voltage[target_channel] = (float)target_value / 4095 * vref_volt;
	}
	return 0;
}