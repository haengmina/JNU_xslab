/**
 * @file mxc6655.cpp
 * @author Kwon Jinwoo (jinwoo@xslab.co.kr)
 * @brief Memsic MXC6655XA Three-Axis Accelerometer
 * @date 2025-08-22
 */

#include <mxc6655.h>

uint8_t MXC6655::get_Value(Value *output)
{
	uint8_t read_start_address = 0x03;
	uint8_t rx_buffer[6] = {};

	if(HAL_I2C_Master_Transmit(get_I2C_Handle(0), target_address, &read_start_address, 1, 100) != HAL_OK)
	{
		printf("%s: ERROR\n", __FUNCTION__);
		return 1;
	}
	if(HAL_I2C_Master_Receive(get_I2C_Handle(0), target_address, rx_buffer, sizeof(rx_buffer), 100) != HAL_OK)
	{
		printf("%s: ERROR\n", __FUNCTION__);
		return 1;
	}

	for(uint8_t i = 0; i < (sizeof(rx_buffer) / 2); i++)
	{
		uint8_t offset = i * 2;
		uint16_t value = (uint16_t)(rx_buffer[offset] << 8) | rx_buffer[offset + 1];
		float value_g = (float)((int16_t)value) / 1024;

		switch(i)
		{
			case 0:
				output->x_g = value_g;
				break;
			case 1:
				output->y_g = value_g;
				break;
			case 2:
				output->z_g = value_g;
				break;
		}
	}
	return 0;
}

uint8_t MXC6655::get_Angular_Displacement(Angular_Displacement *output)
{
	Value value = {};
	if(!output || get_Value(&value)) return 1;

	output->roll = 180 * atan2(value.y_g, sqrt(value.x_g*value.x_g + value.z_g*value.z_g)) / M_PI;
	output->pitch = 180 * atan2(value.x_g, sqrt(value.y_g*value.y_g + value.z_g*value.z_g)) / M_PI;

	return 0;
}
