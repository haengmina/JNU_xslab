/**
 * @file protocol.h
 * @author Kwon Jinwoo (jinwoo@xslab.co.kr)
 * @brief Protocol
 * @date 2025-09-02
 */

#ifndef INCLUDE_PROTOCOL
#define INCLUDE_PROTOCOL

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <checksum.h>

/*!
 * @code
 * Packet Format
 * 
 * [0] : Start Byte (0x7E)
 * [1] : Command Type
 * [2] : Data Length
 * [3 ~ N - 1] : Data
 * [N] : XOR Checksum
 *
 * |  7  |  6  |  5  |  4  |  3  |  2  |  1  |  0  |
 * +-----+-----+-----+-----+-----+-----+-----+-----+----
 * |                  Start Byte                   | [0]
 * +-----+-----+-----+-----+-----+-----+-----+-----+----
 * |                    Command                    | [1]
 * +-----+-----+-----+-----+-----+-----+-----+-----+----
 * |                  Data Length                  | [2]
 * +-----+-----+-----+-----+-----+-----+-----+-----+----
 * |                                               | [3]
 * ~                      Data                     ~
 * |                                               | [N - 1]
 * +-----+-----+-----+-----+-----+-----+-----+-----+----
 * |                 XOR Checksum                  | [N]
 * +-----+-----+-----+-----+-----+-----+-----+-----+----
 * @code
 */
namespace Protocol
{									// , REG_READ = 0xB0, REG_READ_RESPONSE = 0xB1, REG_WRITE = 0xB2  RS-485 통신 위해 추가
	enum class COMMAND : uint8_t {NODE_SELECT_REQUEST = 0x20, NODE_SELECT_RESPONSE = 0x21, ERROR_INVALID_REQUEST = 0x22, REQUEST_FAILED = 0x23, REQUEST_SUCCESS = 0x24, 
									DI_READ = 0x30, DI_THRESHOLD_WRITE = 0x31, DO_READ = 0x32, DO_WRITE = 0x33, DI_RESPONSE = 0x40, DO_RESPONSE = 0x41, 
									DI_DO_READ_ALL = 0x42, DI_DO_READ_ALL_RESPONSE = 0x43, DO_WRITE_ALL = 0x44,
									ANALOG_READ = 0x50, ANALOG_READ_ALL = 0x51, ANALOG_RESPONSE = 0x60, ANALOG_ALL_RESPONSE = 0x61,
									SERIAL_SETUP = 0x70, SERIAL_SETUP_READ =  0x71, SERIAL_WIRTE = 0x80, SERIAL_WRITE_RESPONSE = 0x81, SERIAL_SETUP_READ_RESPONSE = 0x82,
									ACCEL_READ = 0x90, ACCEL_RESPONSE = 0x91, GPS_READ = 0x92, GPS_RESPONSE = 0x93,
									FIRMWARE_VERSION_READ = 0xA0, FIRMWARE_VERSION_RESPONSE = 0xA1, FIRMWARE_REQUEST = 0xA2, REG_READ = 0xB0, REG_READ_RESPONSE = 0xB1, REG_WRITE = 0xB2
								};															
	enum class SELECT_STATUS : uint8_t {DENIED = 0x00, OK};
	struct Interface {
		enum class TYPE : uint8_t {RS232 = 0x01, RS422, RS485, DDI};
		enum class CHANNEL : uint8_t {CH1 = 0x01, CH2};
		enum class DATABIT : uint8_t {BIT7 = 7, BIT8};
		enum class PARITY : uint8_t {NONE = 0x00, ODD, EVEN};
		enum class STOP_BIT : uint8_t {B1 = 0x01, B2};
		enum class FLOW_CONTROL : uint8_t {NONE = 0x00, CTS_RTS, XON_XOFF};

		struct [[gnu::packed]] Setting_Buffer
		{
			TYPE type = TYPE::RS232;
			CHANNEL channel;
			uint32_t baudrate = 9600;
			DATABIT databit = DATABIT::BIT8;
			PARITY parity = PARITY::NONE;
			STOP_BIT stop_bit = STOP_BIT::B1;
			FLOW_CONTROL flow_control = FLOW_CONTROL::NONE;
		};

		struct [[gnu::packed]] Data_Buffer
		{
			CHANNEL channel;
			uint16_t receive_timeout_ms;
			uint8_t data[];
		};
	};
	
	const uint8_t start_byte_value = 0x7E;

	struct [[gnu::packed]] Header
	{
		const uint8_t start_byte = start_byte_value;
		COMMAND command;
		uint8_t data_length;
	};

	struct [[gnu::packed]] Packet_Ptr
	{
		Header header;
		uint8_t data[];
	};

	struct [[gnu::packed]] Node_Select_Reuqest
	{
		const Header header = { .command = COMMAND::NODE_SELECT_REQUEST, .data_length = sizeof(serial_number) };
		uint64_t serial_number;
		uint8_t checksum;
	};

	struct [[gnu::packed]] Node_Select_Response
	{
		const Header header = { .command = COMMAND::NODE_SELECT_RESPONSE, .data_length = sizeof(status) };
		SELECT_STATUS status;
		uint8_t checksum;
	};

	struct [[gnu::packed]] Command_Response_Packet
	{
		Header header;
		uint8_t *data;
		uint8_t checksum;

		~Command_Response_Packet() { deinit(); }
		void deinit() { if(header.data_length) { delete[] data; header.data_length = 0; } }

		void init_Data(uint8_t *data, uint8_t length);

		/**
		 * @brief 지정된 Buffer에 Packet을 저장합니다
		 * 
		 * @param[out]	buffer			Buffer의 Pointer입니다
		 * @param[in]	buffer_size		Buffer의 Size입니다
		 * @retval 0	성공
		 * @retval 1	실패
		 */
		uint8_t get_Buffer(uint8_t *buffer, uint16_t buffer_size);

		/**
		 * @brief Packet의 크기를 읽습니다
		 * 
		 * @return 크기입니다
		 */
		uint16_t get_Size();
	};
};

#endif /* INCLUDE_PROTOCOL */
