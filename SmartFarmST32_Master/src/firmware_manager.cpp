/**
 * @file firmware_manager.cpp
 * @author Kwon Jinwoo (jinwoo@xslab.co.kr)
 * @brief Firmware Manager
 * @date 2025-09-15
 */

#include <firmware_manager.h>

uint16_t Firmware_Manager::record_count = 0;
uint32_t Firmware_Manager::address_offset = 0;
uint8_t Firmware_Manager::buffer[sector_size + 32] = {};
uint16_t Firmware_Manager::buffer_count = 0;

uint8_t Firmware_Manager::Received_Packet_Handler(Packet_Manager* const manager, const Protocol::Packet_Ptr* const request, Protocol::Command_Response_Packet &response)
{
	switch(request->header.command)
	{
		case Protocol::COMMAND::FIRMWARE_VERSION_READ:
		{
			if(request->header.data_length) return 1;

			uint8_t buffer[] = {VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH};

			response.init_Data(buffer, sizeof(buffer));
			response.header.command = Protocol::COMMAND::FIRMWARE_VERSION_RESPONSE;
			break;
		}

		case Protocol::COMMAND::FIRMWARE_REQUEST:
		{
			response.header.command = Protocol::COMMAND::REQUEST_SUCCESS;
			Intel_HEX_Record *record = (Intel_HEX_Record*)request->data;
			switch(record->type)
			{
				case Intel_HEX_Record::TYPE::DATA:
				{
					uint32_t target_address = address_offset;
					target_address |= __builtin_bswap16(record->address_msb_first);
					if(target_address == SystemConfig::get_Main_Firmware_Start_Address())
					{
						buffer_count = 0;
						record_count = 1;
						address_offset = target_address;
						printf("Firmware Download Start, Firmware Area Size: 0x%04lX\n", SystemConfig::get_Firmware_Size());
					}
					else if(!record_count)
					{
						printf("Invalid Start Address\n");
						return 1;
					}
					else if((target_address < SystemConfig::get_Main_Firmware_Start_Address()) || (target_address > SystemConfig::get_Downloaded_Firmware_Start_Address()))
					{
						record_count = 0;
						printf("Invalid Address, Firmware Download Cancel\n");
						return 1;
					}
					else if(auto padding = (target_address - address_offset) - buffer_count; padding)
					{
						memset((void*)&buffer[buffer_count], 0x00, padding);
						buffer_count += padding;
					}
				
					memcpy((void*)&buffer[buffer_count], record->data, record->length);
					buffer_count += record->length;
					record_count++;
					if(buffer_count >= sector_size)
					{
						uint32_t tmp_address = (address_offset + SystemConfig::get_Firmware_Size());

						printf("Flash Write: 0x%08lX, length: %d\n", tmp_address, sector_size);
						if(SystemConfig::set_Flash_Data(tmp_address, (uint64_t)buffer))
						{
							record_count = 0;
							printf("Flash Write Failed, Firmware Download Cancel\n");
							return 1;
						}

						buffer_count -= sector_size;
						address_offset += sector_size;
						if(buffer_count)
						{
							memcpy((void*)buffer, (void*)&buffer[sector_size], buffer_count);
							printf("Page Overflow: %d\n", buffer_count);
						}
					}
					break;
				}

				case Intel_HEX_Record::TYPE::END_OF_FILE:
				{
					if(!record_count) return 1;
					record_count = 0;

					if(buffer_count)
					{
						memset(&buffer[buffer_count], 0xFF, sizeof(buffer) - buffer_count);
						uint32_t tmp_address = (address_offset + SystemConfig::get_Firmware_Size());

						printf("Flash Write: 0x%08lX, length: %d\n", tmp_address, sector_size);
						if(SystemConfig::set_Flash_Data(tmp_address, (uint64_t)buffer)) return 1;
					}
					SystemConfig::set_Update_Request_Flag(true);
					SystemConfig::set_Reset_Request_Flag(true);
					printf("Firmware Firmware Complete: %d, Update Request and Reset Request Flag Set\n", record_count);
					break;
				}

				case Intel_HEX_Record::TYPE::EXTENDED_LINEAR_ADDRESS:
				{
					uint16_t new_address = record->data[0];
					new_address |= (uint16_t)record->data[1] << 8;

					address_offset = (uint32_t)__builtin_bswap16(new_address) << 16;
					record_count++;
					break;
				}

				default:
					break;
			}
			break;
		}

		default:
			return 2;
	}

	return 0;
}