/**
 * @file packet_manager.h
 * @author Kwon Jinwoo (jinwoo@xslab.co.kr)
 * @brief Packet Manager
 * @date 2025-09-03
 */

#ifndef INCLUDE_PACKET_MANAGER
#define INCLUDE_PACKET_MANAGER

#include <stdint.h>
#include <protocol.h>
#include <board_io.h>

class Packet_Manager
{
public:
	struct Buffer
	{
		uint16_t count;
		uint8_t data[256];
	};
	typedef uint8_t (*Packet_Handler)(Packet_Manager* const manager, const Protocol::Packet_Ptr* const request, Protocol::Command_Response_Packet &response);

	/**
	 * @brief Packet Manager를 초기화합니다
	 * 
	 * @param[in]	uart			사용할 UART Handle입니다
	 * @param[in]	serial_number	Node의 Serial Number입니다
	 * @param[in]	handler			수신된 Packet을 처리할 Handler입니다
	 */
	void init(UART_HandleTypeDef *uart, uint64_t serial_number, Packet_Handler handler);

	/**
	 * @brief RS485를 통해 수신된 데이터를 설정합니다
	 * 
	 * @param[in]	data	수신된 데이터입니다
	 */
	void set_Received_Data(uint8_t data);

	/**
	 * @brief 현재 Node가 선택된 상태인지를 읽습니다
	 * 
	 * @return 선택 여부입니다
	 */
	bool get_Node_Selected_Status();

	/**
	 * @brief 전송할 Packet의 상태를 업데이트합니다
	 * 
	 */
	void update_Transmit_Packet();

	/**
	 * @brief 수신된 Packet의 상태를 업데이트합니다
	 * 
	 */
	void update_Received_Packet();

private:
	bool packet_started_flag = false;
	bool packet_received_flag = false;
	uint8_t packet_data_length = 0;
	Buffer receive_buffer;
	Buffer transmit_buffer;

	bool node_selected = false;

	UART_HandleTypeDef *target_uart;
	uint64_t node_serial_number;
	Packet_Handler packet_handler = nullptr;

	/**
	 * @brief RS485를 통해 지정된 데이터를 전송합니다
	 * 
	 * @param[in]	data		전송할 데이터의 Pointer입니다
	 * @param[in]	length		데이터의 길이입니다
	 */
	void set_Transmit_Data(uint8_t *data, uint16_t length);

	/**
	 * @brief Status LED의 상태를 업데이트 합니다
	 * 
	 */
	void update_Status_LED();
};

#endif /* INCLUDE_PACKET_MANAGER */
