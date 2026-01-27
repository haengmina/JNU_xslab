/**
 * @file onboard_packet_handler.h
 * @author Kwon Jinwoo (jinwoo@xslab.co.kr)
 * @brief OnBoard Component Packet Handler
 * @date 2025-09-05
 */

#include <protocol.h>
#include <packet_manager.h>
#include <gps_parser.h>

class OnBoard_Packet_Handler
{
public:
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
	static uint8_t Handler(Packet_Manager* const manager, const Protocol::Packet_Ptr* const request, Protocol::Command_Response_Packet &response);

	/**
	 * @brief 수신된 GPS 데이터를 설정합니다
	 * 
	 * @param[in]	data	데이터입니다
	 */
	static void set_GPS_Data(uint8_t data);

private:
	static GPS_Parser gps;
};