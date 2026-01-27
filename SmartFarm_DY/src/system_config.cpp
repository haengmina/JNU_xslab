/**
 * @file system_config.cpp
 * @author Kwon Jinwoo (jinwoo@xslab.co.kr)
 * @brief System Configuration
 * @date 2025-09-12
 */

#include <system_config.h>

extern uint32_t __firmware_size;
extern uint32_t __flash_start_address;
extern uint32_t __config_start_address;
extern uint32_t __downloaded_firmware_start_address;
extern uint32_t __main_firmware_start_address;
SystemConfig::Config_Data *SystemConfig::config = (SystemConfig::Config_Data*)&__config_start_address;

bool SystemConfig::reset_request_flag = false;

uint32_t SystemConfig::get_Firmware_Size()
{
	return (uint32_t)&__firmware_size;
}

uint32_t SystemConfig::get_Flash_Start_Address()
{
	return (uint32_t)&__flash_start_address;
}

uint32_t SystemConfig::get_Downloaded_Firmware_Start_Address()
{
	return (uint32_t)&__downloaded_firmware_start_address;
}

uint32_t SystemConfig::get_Main_Firmware_Start_Address()
{
	return (uint32_t)&__main_firmware_start_address;
}

bool SystemConfig::get_Valid_Status()
{
	return (config->valid_code == SystemConfig::valid_code_value);
}

bool SystemConfig::get_Update_Request_Flag()
{
	if(!get_Valid_Status()) return false;

	return config->update_request_flag;
}

bool SystemConfig::get_Reset_Request_Flag()
{
	return reset_request_flag;
}

void SystemConfig::set_Reset_Request_Flag(bool enable)
{
	reset_request_flag = enable;
}

uint8_t SystemConfig::set_Update_Request_Flag(bool flag)
{
	if(set_Erase_Flash((uint32_t)config)) return 1;

	Config_Data new_config(*config);
	new_config.valid_code = valid_code_value;
	new_config.update_request_flag = flag;

	static_assert(sizeof(SystemConfig::Config_Data) <= sizeof(uint64_t), "Config_Data size more then 8 bytes, need to update code");

	uint64_t flash_data;
	auto target_ptr = (uint8_t*)&flash_data;
	auto tmp_ptr = (uint8_t*)&new_config;
	for(uint8_t i = 0; i < sizeof(SystemConfig::Config_Data); i++)
	{
		target_ptr[i] = tmp_ptr[i];
	}

	if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, (uint32_t)config, flash_data) != HAL_OK)
	{
		HAL_FLASH_Lock();
		return 1;
	}

	HAL_FLASH_Lock();
	return 0;
}

uint8_t SystemConfig::set_Erase_Flash(uint32_t address)
{
	uint32_t address_offset = get_Flash_Start_Address() + FLASH_BANK_SIZE;
	if(address % FLASH_PAGE_SIZE) return 0;
	
	FLASH_EraseInitTypeDef erase_init = {};
	erase_init.Banks = (address < address_offset) ? FLASH_BANK_1 : FLASH_BANK_2; //Flash Bank Start at 
	erase_init.TypeErase = FLASH_TYPEERASE_PAGES;
	erase_init.NbPages = 1;
	erase_init.Page = (address - ((erase_init.Banks == FLASH_BANK_1) ? get_Flash_Start_Address() : address_offset)) / FLASH_PAGE_SIZE;
	uint32_t page_error = 0;
	HAL_StatusTypeDef ret;
	printf("Erase: 0x%08lX\n", address);
	printf("FLASH->SR: 0x%08lX, FLASH->CR: 0x%08lX\n", FLASH->SR, FLASH->CR);

	if(HAL_FLASH_Unlock() != HAL_OK)
	{
		printf("Flash Unlock Failed\n");
		return 1;
	}
	if((FLASH->CR & FLASH_CR_OPTLOCK) && (HAL_FLASH_OB_Unlock() != HAL_OK))
	{
		printf("Flash Option Bit Unlock Failed\n");
		return 1;
	}
	if((ret = HAL_FLASHEx_Erase(&erase_init, &page_error)) != HAL_OK)
	{
		printf("Flash Erase Failed, %d, %ld, 0x%08lX\n", (uint8_t)ret, page_error, pFlash.ErrorCode);
		if(ret != HAL_TIMEOUT)
		{
			printf("FLASH->SR: 0x%08lX, FLASH->CR: 0x%08lX\n", FLASH->SR, FLASH->CR);
			return 1;
		}
	}

	return 0;
}

uint8_t SystemConfig::set_Flash_Data(uint32_t address, uint64_t data_address)
{
	if(set_Erase_Flash(address))
	{
		printf("set_Erase_Flash() Error\n");
		return 1;
	}

	if(HAL_StatusTypeDef ret; (ret = HAL_FLASH_Program(FLASH_TYPEPROGRAM_FAST, address, data_address)) != HAL_OK)
	{
		printf("FLASH->SR: 0x%08lX, FLASH->CR: 0x%08lX\n", FLASH->SR, FLASH->CR);
		printf("Flash Program Failed: %d, 0x%08lX\n", ret, address);
		return 1;
	}

	return 0;
}