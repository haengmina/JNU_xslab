/**
 * @file protocol.cpp
 * @author Kwon Jinwoo (jinwoo@xslab.co.kr)
 * @brief 
 * @date 2025-09-03
 */

#include <protocol.h>

using namespace Protocol;

void Command_Response_Packet::init_Data(uint8_t *data, uint8_t length)
{
	deinit();
	if(!length) return;

	this->header.data_length = length;
	this->data = new uint8_t[length];
	memcpy(this->data, data, length);

	checksum = 0x00;
	checksum = get_XOR_Checksum((uint8_t*)&header, sizeof(header));
	checksum = get_XOR_Checksum(data, header.data_length, checksum);
}

uint8_t Command_Response_Packet::get_Buffer(uint8_t *buffer, uint16_t buffer_size)
{
	if(!buffer || (buffer_size <= get_Size())) return 1;

	auto tmp_ptr = buffer;
	memcpy(tmp_ptr, &header, sizeof(header));
	tmp_ptr += sizeof(header);
	memcpy(tmp_ptr, data, header.data_length);
	tmp_ptr += header.data_length;
	memcpy(tmp_ptr, &checksum, sizeof(checksum));

	return 0;
}

uint16_t Command_Response_Packet::get_Size()
{
	return (sizeof(header) + header.data_length + sizeof(checksum));
}