/**
 * @file mxc6655.h
 * @author Kwon Jinwoo (jinwoo@xslab.co.kr)
 * @brief Memsic MXC6655XA Three-Axis Accelerometer
 * @date 2025-08-22
 */

#ifndef INCLUDE_MXC6655
#define INCLUDE_MXC6655

#include <stdio.h>
#include <stdint.h>
#include <board_io.h>
#include <stm32g0xx.h>
#include <math.h>


class MXC6655
{
public:
	MXC6655(I2C_HandleTypeDef *i2c, uint8_t target_address) : i2c(i2c), target_address(target_address) {}

	struct [[gnu::packed]] Value {
		float x_g;
		float y_g;
		float z_g;
	};

	struct [[gnu::packed]] Angular_Displacement {
		float roll;
		float pitch;
	};

	/**
	 * @brief 가속도 값을 읽습니다
	 * 
	 * @param[out]	output	가속도 값입니다
	 * @retval 0	성공
	 * @retval 1	실패
	 */
	uint8_t get_Value(Value *output);

	/**
	 * @brief 각 변위 값을 읽습니다
	 * 
	 * @param[out]	output	각 변위 값입니다
	 * @retval 0	성공
	 * @retval 1	실패
	 */
	uint8_t get_Angular_Displacement(Angular_Displacement *output);

private:
	I2C_HandleTypeDef *i2c;
	uint8_t target_address;
};

#endif /* INCLUDE_MXC6655 */
