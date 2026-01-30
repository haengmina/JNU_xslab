/**
 * @file gps_parser.h
 * @author Kwon Jinwoo (jinwoo@xslab.co.kr)
 * @brief GPS Data Parser
 * @date 2025-09-08
 */

#ifndef INCLUDE_GPS_PARSER
#define INCLUDE_GPS_PARSER

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <checksum.h>

class GPS_Parser
{
public:
	struct [[gnu::packed]] Data
	{
		uint8_t hour;
		uint8_t minute;
		uint8_t second;
		double latitude;
		bool south_flag;
		double longitude;
		bool west_flag;
		bool position_fix_flag;
	};

	/**
	 * @brief 수신된 GPS 데이터를 설정합니다
	 *
	 * @param[in]	data	데이터입니다
	 */
	void set_Data(uint8_t data);

	/**
	 * @brief 현재 GPS 데이터를 읽습니다
	 *
	 * @return GPS 데이터입니다
	 */
	const Data& get_Data();

	/**
	 * @brief GPS 데이터 상태를 업데이트합니다
	 *
	 */
	void update();

private:
	Data gps_data;

	const char token = ',';
	const char checksum_token = '*';
	const char start_char = '$';
	const char *gngga_id = "GNGGA,";
	bool gngga_start_flag;
	uint8_t id_buffer[8];
	uint8_t id_buffer_count = 0;

	uint8_t data_buffer[128];
	uint8_t data_buffer_count = 0;
	uint8_t checksum_start_index = 0;
};

#endif /* INCLUDE_GPS_PARSER */
