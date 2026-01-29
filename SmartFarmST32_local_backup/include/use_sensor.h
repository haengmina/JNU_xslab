/**
 * @file use_sensor.h
 * @author Han YoungSeo (gdt00@naver.com)
 * @brief 온습도 센서로 팬 제어
 * @version 1.0
 * @date 2025-12-02
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#ifndef USE_SENSOR
#define USE_SENSOR

#include <board_io.h>
#include <sensor_interface_board.h>
#include <cmath>

/**
 * @brief 온습도에 따라 팬 on, off를 조절합니다
 * 
 * @param v1 0, 1, 2, 3 을 넣어 vin 포트 매칭
 * @param v2 0, 1, 2, 3 을 넣어 vin 포트 매칭
 * @param t 원하는 온도값
 * @param p 원하는 습도값
 */
void use_sensor(int v1, float t, int v2, float p);


#endif
