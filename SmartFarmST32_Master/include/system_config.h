/**
 * @file system_config.h
 * @author Kwon Jinwoo (jinwoo@xslab.co.kr)
 * @brief System Configuration
 * @date 2025-09-12
 */

#ifndef INCLUDE_SYSTEM_CONFIG
#define INCLUDE_SYSTEM_CONFIG

#include <stdio.h>
#include <stdint.h>
#include <stm32g0xx.h>

class SystemConfig
{
public:	
	struct [[gnu::packed]] Config_Data
	{
		uint8_t valid_code;
		bool update_request_flag;
	};

	/**
	 * @brief Firmware 영역의 전체 크기를 읽습니다
	 * 
	 * @return size입니다
	 */
	static uint32_t get_Firmware_Size();

	/**
	 * @brief Flash Memory의 Start Address를 읽습니다
	 * 
	 * @return 주소입니다
	 */
	static uint32_t get_Flash_Start_Address();

	/**
	 * @brief Downloaded Firmware 영역의 시작 주소를 읽습니다
	 * 
	 * @return 주소입니다
	 */
	static uint32_t get_Downloaded_Firmware_Start_Address();

	/**
	 * @brief Main Firmware 영역의 시작 주소를 읽습니다
	 * 
	 * @return 주소입니다
	 */
	static uint32_t get_Main_Firmware_Start_Address();

	/**
	 * @brief 현재 System Configuration이 유효한지 확인합니다
	 * 
	 * @return 유효한지 여부입니다
	 */
	static bool get_Valid_Status();

	/**
	 * @brief Firmware Update Request Flag를 읽습니다
	 * 
	 * @return Flag 상태입니다
	 */
	static bool get_Update_Request_Flag();

	/**
	 * @brief Reset이 필요한 상태인지 여부를 확인합니다
	 * 
	 * @return 상태입니다
	 */
	static bool get_Reset_Request_Flag();

	/**
	 * @brief Reset이 필요한 상태인지 여부를 설정합니다
	 * 
	 */
	static void set_Reset_Request_Flag(bool enable);

	/**
	 * @brief Firmware Update Request Flag를 설정합니다
	 * 
	 * @param[in]	flag	설정할 Flag 상태입니다
	 * @retval 0	성공
	 * @retval 1	실패
	 */
	static uint8_t set_Update_Request_Flag(bool flag);

	/**
	 * @brief 지정된 Downloaded Firmware 영역을 지웁니다
	 * 
	 * @param[in]	address		대상 주소입니다
	 * @retval 0	성공
	 * @retval 1	실패
	 */
	static uint8_t set_Erase_Flash(uint32_t address);

	/**
	 * @brief 8Byte의 새로운 Firmware 데이터를 Downloaded Firmware 영역에 기록합니다
	 * 
	 * @param[in]	address		대상 주소입니다
	 * @param[in]	data		데이터의 Pointer입니다
	 * @retval 0	성공
	 * @retval 1	실패
	 */
	static uint8_t set_Flash_Data(uint32_t address, uint64_t data);

private:
	static constexpr uint8_t valid_code_value = 0x55;
	static Config_Data *config;
	static bool reset_request_flag;
};

#endif /* INCLUDE_SYSTEM_CONFIG */
