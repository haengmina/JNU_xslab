/**
 * @file onboard_packet_hadnler.cpp
 * @author Kwon Jinwoo (jinwoo@xslab.co.kr)
 * @brief OnBoard Packet Handler
 * @date 2025-09-05
 */

#include <onboard_packet_handler.h>

GPS_Parser OnBoard_Packet_Handler::gps;

void OnBoard_Packet_Handler::set_GPS_Data(uint8_t data)
{
	gps.set_Data(data);
}

uint8_t OnBoard_Packet_Handler::Handler(Packet_Manager* const manager, const Protocol::Packet_Ptr* const request, Protocol::Command_Response_Packet &response)
{
	switch(request->header.command)
	{
		case Protocol::COMMAND::ACCEL_READ:
		{
			if(request->header.data_length) return 1;

			auto target = get_Accel_Object();
			MXC6655::Angular_Displacement value = {};
			if(target->get_Angular_Displacement(&value)) return 1;

			response.init_Data((uint8_t*)&value, sizeof(value));
			response.header.command = Protocol::COMMAND::ACCEL_RESPONSE;
			break;
		}

		case Protocol::COMMAND::GPS_READ:
		{
			if(request->header.data_length) return 1;

			auto value = gps.get_Data();

			response.init_Data((uint8_t*)&value, sizeof(value));
			response.header.command = Protocol::COMMAND::GPS_RESPONSE;
			break;	
		}

		default:
			return 2;
	}
	return 0;
}