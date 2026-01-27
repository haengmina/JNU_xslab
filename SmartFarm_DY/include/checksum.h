/**
 * @file checksum.h
 * @author Kwon Jinwoo (jinwoo@xslab.co.kr)
 * @brief XOR Checksum
 * @date 2025-09-03
 */

#ifndef INCLUDE_CHECKSUM
#define INCLUDE_CHECKSUM

#include <stdint.h>

/**
 * @brief XOR 연산을 통한 Checksum을 읽습니다
 * 
 * @param[in]	data		대상 데이터의 Pinter입니다
 * @param[in]	length		데이터의 길이입니다
 * @param[in]	init_value	Checksum 초기값입니다
 * @return Checksum 값입니다
 */
uint8_t get_XOR_Checksum(uint8_t *data, uint16_t length, uint8_t init_value = 0x00);

#endif /* INCLUDE_CHECKSUM */
