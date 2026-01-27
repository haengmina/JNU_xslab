/**
 * @file packet_manager.cpp
 * @author Kwon Jinwoo (jinwoo@xslab.co.kr)
 * @brief Packet Manager
 * @date 2025-09-03
 */

#include <packet_manager.h>
#include <sensor_interface_board.h>

void Packet_Manager::init(UART_HandleTypeDef *uart, uint64_t serial_number, Packet_Handler handler)
{
	target_uart = uart;
	node_serial_number = serial_number;
	packet_handler = handler;
	set_Isolated_RS485_TX_Enable(false);
	if(target_uart && target_uart->Instance == USART5)
	{
		Sensor_Interface_Board::set_Interface_TX(0, false);
	}
}

void Packet_Manager::set_Received_Data(uint8_t data)
{
	static uint32_t last_received_tick = HAL_GetTick();
	uint32_t current_tick = HAL_GetTick();
	if(packet_started_flag && ((current_tick - last_received_tick) > 100))
	{
		//Byte간의 간격이 100ms 를 초과하는 경우, 초기화
		packet_started_flag = false;
	}
	last_received_tick = current_tick;
	
	if(!packet_started_flag)
	{
		if(data != Protocol::start_byte_value) return;

		packet_started_flag = true;
		receive_buffer.count = 0;
	}
	receive_buffer.data[receive_buffer.count++] = data;

	if(receive_buffer.count == sizeof(Protocol::Header))
	{
		packet_data_length = ((Protocol::Header*)receive_buffer.data)->data_length;
		return;
	}
	if(receive_buffer.count != (sizeof(Protocol::Header) + packet_data_length + 1)) return; //Header + Data Length + Checksum
	packet_started_flag = false;

	uint8_t checksum = get_XOR_Checksum(receive_buffer.data, receive_buffer.count - 1);
	if(checksum != receive_buffer.data[receive_buffer.count - 1])
	{
		printf("CheckSum Failed, 0x%02X != 0x%02X\n", checksum, receive_buffer.data[receive_buffer.count - 1]);
		return;
	}

	auto header = ((Protocol::Header*)receive_buffer.data);
	switch(header->command)
	{
		case Protocol::COMMAND::NODE_SELECT_REQUEST:
		{
			Protocol::Node_Select_Reuqest *request = (Protocol::Node_Select_Reuqest*)receive_buffer.data;
			if(request->serial_number != node_serial_number)
			{
				node_selected = false;
				update_Status_LED();
				return;
			}
			else if(!node_selected)
			{
				node_selected = true;
				update_Status_LED();
			}
			else
			{
				printf("Node Already Selected\n");
			}

			Protocol::Node_Select_Response response;
			response.status = Protocol::SELECT_STATUS::OK;
			response.checksum = get_XOR_Checksum((uint8_t*)&response, (sizeof(response) - 1));
			set_Transmit_Data((uint8_t*)&response, sizeof(response));
			break;
		}

		default:
			if(!node_selected) return;
			packet_received_flag = true;
			break;
	}
}

bool Packet_Manager::get_Node_Selected_Status()
{
	return node_selected;
}

void Packet_Manager::set_Transmit_Data(uint8_t *data, uint16_t length)
{
	bool use_db9 = (target_uart && target_uart->Instance == USART5);
	if(use_db9)
	{
		Sensor_Interface_Board::set_Interface_TX(0, true);
	}
	else
	{
		set_Isolated_RS485_TX_Enable(true);
	}
	volatile uint16_t delay = 0xFFF;
	while(delay--); //wait for switching

	HAL_UART_Transmit(target_uart, data, length, -1);
	if(use_db9)
	{
		set_UART_Receive_Start(3);
		Sensor_Interface_Board::set_Interface_TX(0, false);
	}
	else
	{
		set_UART_Receive_Start(0);
		set_Isolated_RS485_TX_Enable(false);
	}
}

void Packet_Manager::update_Transmit_Packet()
{
	if(!transmit_buffer.count) return;

	transmit_buffer.data[transmit_buffer.count - 1] = get_XOR_Checksum(transmit_buffer.data, (transmit_buffer.count - 1));

	set_Transmit_Data(transmit_buffer.data, transmit_buffer.count);
	transmit_buffer.count = 0;
	node_selected = false;
	update_Status_LED();
}

void Packet_Manager::update_Received_Packet()
{
	if(!packet_received_flag || !packet_handler) return;
	auto request = (Protocol::Packet_Ptr*)receive_buffer.data;

	Protocol::Command_Response_Packet response;
	if(packet_handler(this, request, response))
	{
		response.header.command = Protocol::COMMAND::ERROR_INVALID_REQUEST;
	}
	response.get_Buffer(transmit_buffer.data, sizeof(transmit_buffer.data));
	transmit_buffer.count = response.get_Size();
	packet_received_flag = false;
}

void Packet_Manager::update_Status_LED()
{
	set_Status_LED(STATUS_LED_TYPE::BLUE, node_selected);
	set_Status_LED(STATUS_LED_TYPE::RS485_ACT, node_selected);
}
