/**
 * @file checksum.cpp
 * @author Kwon Jinwoo (jinwoo@xslab.co.kr)
 * @brief XOR Checksum
 * @date 2025-09-03
 */

#include <checksum.h>

uint8_t get_XOR_Checksum(uint8_t *data, uint16_t length, uint8_t init_value)
{
	uint8_t checksum = init_value;
	for(uint16_t i = 0; i < length; i++)
	{
		checksum ^= data[i];
	}

	return checksum;	
}