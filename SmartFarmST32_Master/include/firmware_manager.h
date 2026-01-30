/**
 * @file firmware_manager.h
 * @author Kwon Jinwoo (jinwoo@xslab.co.kr)
 * @brief Firmware Manager
 * @date 2025-09-15
 */

#ifndef INCLUDE_FIRMWARE_MANAGER
#define INCLUDE_FIRMWARE_MANAGER

#include <stdio.h>
#include <stdint.h>
#include <system_config.h>
#include <packet_manager.h>

class Firmware_Manager
{
public:
	struct [[gnu::packed]] Intel_HEX_Record
	{
		enum class TYPE : uint8_t { DATA = 0x00, END_OF_FILE = 0x01, EXTENDED_SEGMENT_ADDRESS = 0x02, START_SEGMENT_ADDRESS = 0x03, 
									EXTENDED_LINEAR_ADDRESS = 0x04, START_LINEAR_ADDRESS = 0x05 };
		uint8_t length;
		uint16_t address_msb_first;
		TYPE type;
		uint8_t data[];
	};

	/**
	 * @brief Packet을 처리합니다
	 * 
	 * @param[in]	manager		Packet Manager의 Pointer입니다
	 * @param[in]	request		Request입니다
	 * @param[out]	response	Response입니다
	 * @retval 0	성공
	 * @retval 1	실패
	 * @retval 2	유효하지 않은 Command입니다
	 */
	static uint8_t Received_Packet_Handler(Packet_Manager* const manager, const Protocol::Packet_Ptr* const request, Protocol::Command_Response_Packet &response);

private:
	static uint16_t record_count;
	static uint32_t address_offset;
	static const uint16_t sector_size = 256;
	static uint8_t buffer[sector_size + 32] [[gnu::aligned(8)]];
	static uint16_t buffer_count;
};

#endif /* INCLUDE_FIRMWARE_MANAGER */
