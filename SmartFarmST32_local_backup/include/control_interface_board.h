/**
 * @file control_interface_board.h
 * @author Kwon Jinwoo (jinwoo@xslab.co.kr)
 * @brief C-IF (Control Interface) Board Handling
 * @date 2025-08-22
 */

#ifndef INCLUDE_CONTROL_INTERFACE_BOARD
#define INCLUDE_CONTROL_INTERFACE_BOARD

#include <stdio.h>
#include <stdint.h>
#include <stm32g0xx.h>
#include <board_io.h>
#include <tla2528.h>
#include <packet_manager.h>

class Control_Interface_Board
{
public:
	enum class DI_THRESHOLD : uint8_t { P18V = 0, P24V, P39V };

	/**
	 * @brief Control Interface Board를 초기화합니다
	 * 
	 */
	static void init();

	/**
	 * @brief DI 입력에 대한 Threshold 값을 설정합니다
	 * 
	 * @param[in]	value	Threshold 값입니다
	 * @retval 0	성공
	 * @retval 1	실패
	 */
	static uint8_t set_DI_Threshold_Voltage(DI_THRESHOLD value);

	/**
	 * @brief DO (Digital Output)을 제어합니다
	 * 
	 * @param[in]	index	대상 채널 Index 입니다 (0 ~ 7)
	 * @param[in]	enable	활성화 여부입니다
	 * @retval 0	설정했습니다
	 * @retval 1	올바르지 않은 index입니다
	 */
	static uint8_t set_DO_Status(uint8_t index, bool enable);

	/**
	 * @brief DO (Digital Output) 제어를 테스트합니다
	 * 
	 */
	static void set_DO_Status_Test();

	/**
	 * @brief DI (Digital Input) 제어를 테스트합니다
	 * 
	 */
	static void set_DI_Status_Test();

	/**
	 * @brief DI (Digital Input) 제어를 테스트합니다
	 * 
	 */
	static void set_Manual_Control_Switch_Status_Test();

	/**
	 * @brief DO (Digital Output)의 소비 전류 측정 값의 RAW Voltage 상태를 테스트합니다
	 * 
	 */
	static void set_DO_Current_RAW_Test();

	/**
	 * @brief DO (Digital Output)의 소비 전류 측정 값을 mA 단위로 테스트 합니다
	 * 
	 */
	static void set_DO_Current_mA_Test();

	/**
	 * @brief DO (Digital Output)의 상태를 읽습니다
	 * 
	 * @param[in]	index	대상 채널 Index 입니다 (0 ~ 7)
	 * @return 활성화 상태입니다
	 */
	static bool get_DO_Status(uint8_t index);

	/**
	 * @brief DI (Digital Input)의 상태를 읽습니다
	 * 
	 * @param[in]	index	대상 채널 Index 입니다 (0 ~ 7)
	 * @return 상태입니다
	 */
	static bool get_DI_Status(uint8_t index);

	/**
	 * @brief DO (Digital Output)의 Manual Control Switch의 상태를 읽습니다
	 * 
	 * @param[in]	index	대상 채널 Index 입니다 (0 ~ 7)
	 * @return 상태입니다
	 */
	static bool get_Manual_Control_Status(uint8_t index);

	/**
	 * @brief DO (Digital Output)의 소비 전류 측정 값의 RAW Voltage를 읽습니다
	 * 
	 * @param[in]	index	대상 채널 Index 입니다 (0 ~ 7)
	 * @return 측정 결과 전압입니다
	 */
	static float get_DO_Current_RAW(uint8_t index);

	/**
	 * @brief DO (Digital Output)의 소비 전류 측정 값을 mA 단위로 읽습니다
	 * 
	 * @param[in]	index	대상 채널 Index 입니다 (0 ~ 7)
	 * @return 측정 전류 값입니다
	 */
	static int16_t get_DO_Current_mA(uint8_t index);

	/**
	 * @brief ADC 값을 업데이트합니다
	 * 
	 * @retval 0	성공
	 * @retval 1	실패
	 */
	static uint8_t update_ADC_Value();

	/**
	 * @brief DI (Digital Input)의 상태를 업데이트합니다
	 * 
	 * @note TODO: Interrupt 방식으로 전환
	 */
	static void update_DI_Status();

	/**
	 * @brief DO (Digital Output)의 상태를 업데이트합니다
	 * 
	 */
	static void update_DO_Status();

	/**
	 * @brief DO (Digital Output) Manual Switch의 상태를 업데이트합니다
	 * 
	 * @note TODO: Interrupt 방식으로 전환
	 */
	static void update_Manual_Switch_Status();

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
	static uint8_t Received_Packet_Handler(Packet_Manager* const manager, const Protocol::Packet_Ptr* const request, Protocol::Command_Response_Packet &response);

private:
	static GPIO_Init_Info digital_output_info[];
	static GPIO_Init_Info digital_input_info[];
	static GPIO_Init_Info manual_control_input_info[];

	static constexpr uint8_t digital_channel_count = 8;

	static TLA2528 adc;
	static float do_current_raw_volt[digital_channel_count];
	static uint16_t do_current_zero_point_mv[digital_channel_count];
	static uint16_t led_value;
	static uint16_t last_switch_value;
	static uint16_t do_load_detect_threshold_ma;
};


#endif /* INCLUDE_CONTROL_INTERFACE_BOARD */
