/**
 * @file gps_parser.cpp
 * @author Kwon Jinwoo (jinwoo@xslab.co.kr)
 * @brief GPS Data Parser
 * @date 2025-09-08
 */

#include <gps_parser.h>

void GPS_Parser::set_Data(uint8_t data)
{
	if(data == start_char)
	{
		id_buffer_count = 0;
		gngga_start_flag = false;
		return;
	}

	if(!gngga_start_flag)
	{
		id_buffer[id_buffer_count++] = data;
		if(data != token) return;
		if(!strncmp((char*)id_buffer, gngga_id, id_buffer_count))
		{
			data_buffer_count = 0;
			gngga_start_flag = true;
		}

		return;
	}

	data_buffer[data_buffer_count++] = data;
	if(data == checksum_token) { checksum_start_index = data_buffer_count; }
	if(data != '\n') return;

	data_buffer[data_buffer_count -1] = 0;
	data_buffer[checksum_start_index - 1] = 0;
	gngga_start_flag = false;

	uint8_t checksum = get_XOR_Checksum(id_buffer, id_buffer_count, 0x00);
	checksum = get_XOR_Checksum(data_buffer, checksum_start_index, checksum);

	char *checksum_ptr = (char*)&data_buffer[checksum_start_index];
	uint8_t checksum_value = strtol(checksum_ptr, NULL, 16);
	if(checksum != checksum_value)
	{
		id_buffer_count = 0;
		data_buffer_count = 0;
		// printf("%s: GPS Checksum Failed: 0x%02X != 0x%02X\n", __FUNCTION__, checksum, checksum_value);
		return;
	}

	// printf("GNGGA End: %02X == %02X, %s\n", checksum, checksum_value, (char*)data_buffer);
	update();
}

const GPS_Parser::Data& GPS_Parser::get_Data()
{
	return gps_data;
}

void GPS_Parser::update()
{
	if(!data_buffer_count) return;
	for(auto &target : data_buffer)
	{
		if(target != ',') { continue; }

		target = 0x00;
	}

	char *tmp_ptr = (char*)data_buffer;
	gps_data.hour = (tmp_ptr[0] - '0') * 10 + (tmp_ptr[1] - '0');
	tmp_ptr += 2;
	gps_data.minute = (tmp_ptr[0] - '0') * 10 + (tmp_ptr[1] - '0');
	tmp_ptr += 2;
	gps_data.second = (tmp_ptr[0] - '0') * 10 + (tmp_ptr[1] - '0');
	tmp_ptr += 2;
	while(*tmp_ptr++);
	// printf("%02d:%02d:%02d\n", gps_data.hour, gps_data.minute, gps_data.second);

	uint16_t tmp_latitude = (tmp_ptr[0] - '0') * 10 + (tmp_ptr[1] - '0');
	tmp_ptr += 2;
	gps_data.latitude = atof(tmp_ptr) / 60 + tmp_latitude;
	while(*tmp_ptr++);

	gps_data.south_flag = (*tmp_ptr == 'S');
	while(*tmp_ptr++);

	uint16_t tmp_longitude = (tmp_ptr[0] - '0') * 100 + (tmp_ptr[1] - '0') * 10 + (tmp_ptr[2] - '0');
	tmp_ptr += 3;
	gps_data.longitude = atof(tmp_ptr) / 60 + tmp_longitude;
	while(*tmp_ptr++);

	gps_data.west_flag = (*tmp_ptr == 'W');
	while(*tmp_ptr++);

	gps_data.position_fix_flag = (*tmp_ptr != '0');
	while(*tmp_ptr++);

	// printf("%f %c, %f %c, %d\n", gps_data.latitude, (gps_data.south_flag ? 'S' : 'N'), gps_data.longitude, (gps_data.west_flag ? 'W' : 'E'), gps_data.position_fix_flag);
}