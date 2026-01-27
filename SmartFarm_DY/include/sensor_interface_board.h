/**
 * @file sensor_interface_board.h
 * @author Kwon Jinwoo (jinwoo@xslab.co.kr)
 * @brief S-IF (Sensor Interface) Board Handling
 * @date 2025-08-22
 */

#ifndef INCLUDE_SENSOR_INTERFACE_BOARD
#define INCLUDE_SENSOR_INTERFACE_BOARD

#include <stdio.h>
#include <stdint.h>
#include <stm32g0xx.h>
#include <board_io.h>
#include <tla2528.h>
#include <packet_manager.h>
#include <util.h>

class Sensor_Interface_Board
{
public:
	enum class INTERFACE_MODE : uint8_t { RS232, RS422, RS485, DDI, SDI };

	/**
	 * @brief Sensor Interface Board를 초기화합니다
	 * 
	 */
	static void init();

	/**
	 * @brief +5V, +12V, +24V 터미널 출력을 설정합니다
	 * 
	 * @param[in]	enable	활성화 여부입니다
	 */
	static void set_Power_Output(bool enable);

	/**
	 * @brief +12V 전원 출력 활성화 상태를 읽습니다
	 * 
	 * @return 활성화 여부입니다
	 */
	static bool get_Power_Output_Status();

	/**
	 * @brief +12V 전원 출력 상태를 설정합니다
	 *
	 * @param[in]	enable	활성화 여부입니다
	 */
	static void set_12V_Output(bool enable);

	/**
	 * @brief 지정된 통신 채널의 Mode를 설정합니다
	 * 
	 * @param[in]	index	대상 채널입니다 (0 ~ 1)
	 * @param[in]	mode	통신 모드입니다
	 * @retval 0	성공
	 * @retval 1	실패
	 */
	static uint8_t set_Interface_Mode(uint8_t index, INTERFACE_MODE mode);

	/**
	 * @brief 지정된 통신 채널의 Transmit Mode를 설정합니다
	 * 
	 * @param[in]	index	대상 채널입니다 (0 ~ 1)
	 * @param[in]	enable	활성화 여부입니다
	 * @retval 0	성공
	 * @retval 1	실패
	 */
	static uint8_t set_Interface_TX(uint8_t index, bool enable);

	/**
	 * @brief 지정된 통신 채널의 RTS (Request To Send)를 설정합니다
	 * 
	 * @param[in]	index	대상 채널입니다 (0 ~ 1)
	 * @param[in]	enable	활성화 여부입니다
	 * @retval 0	성공
	 * @retval 1	실패
	 * 
	 * @note 대상 채널이 RS232 Mode로 설정되어 있어야합니다
	 */
	static uint8_t set_Interface_RTS(uint8_t index, bool enable);

	/**
	 * @brief 지정된 통신 채널의 설정을 변경합니다
	 * 
	 * @param[in]	param	대상 설정 값입니다
	 * @retval 0	성공
	 * @retval 1	실패
	 */
	static uint8_t set_Interface_Setting(const Protocol::Interface::Setting_Buffer &param);

	/**
	 * @brief 지정된 통신 채널의 수신 데이터를 설정합니다
	 * 
	 * @param[in]	index	대상 채널입니다 (0 ~ 1)
	 * @param[in]	data	수신 데이터 입니다
	 * @retval 0	성공
	 * @retval 1	실패
	 * 
	 * @note Protocol을 통해 데이터를 수신하도록 설정된 경우에만 데이터를 처리합니다
	 */
	static uint8_t set_Interface_RX_Data(uint8_t index, uint8_t data);

	/**
	 * @brief 지정된 통신 채널의 데이터 수신 시작합니다
	 * 
	 * @param[in]	index	대상 채널입니다 (0 ~ 1)
	 */
	static void set_Interface_RX_Start(uint8_t index);

	/**
	 * @brief 전압 측정 기능을 테스트합니다
	 * 
	 */
	static void set_Voltage_Measuremnet_Test();

	/**
	 * @brief 전류 측정 기능을 테스트합니다
	 * 
	 */
	static void set_Current_Measuremnet_Test();

	/**
	 * @brief 지정된 채널의 RAW Voltage를 읽습니다
	 * 
	 * @param[in]	index	대상 채널입니다 (0 ~ 3)
	 * @return volt 단위의 값입니다
	 */
	static float get_Voltage_RAW(uint8_t index);

	/**
	 * @brief 지정된 채널의 전압 측정 값을 mV 단위로 읽습니다
	 * 
	 * @param[in]	index	대상 채널입니다 (0 ~ 3)
	 * @return mV 단위의 값입니다
	 */
	static int16_t get_Voltage_mV(uint8_t index);

	/**
	 * @brief 지정된 채널의 소비 전류 측정 값의 RAW Voltage를 읽습니다
	 * 
	 * @param[in]	index	대상 채널입니다 (0 ~ 3)
	 * @return volt 단위의 값입니다
	 */
	static float get_Current_RAW(uint8_t index);

	/**
	 * @brief 지정된 채널의 소비 전류 측정 값을 uA 단위로 읽습니다
	 * 
	 * @param[in]	index	대상 채널입니다 (0 ~ 3)
	 * @return uA 단위의 값입니다
	 */
	static uint16_t get_Current_uA(uint8_t index);

	/**
	 * @brief ADC 값을 업데이트합니다
	 * 
	 * @retval 0	성공
	 * @retval 1	실패
	 */
	static uint8_t update_ADC_Value();

	/**
	 * @brief 지정된 통신 채널의 CTS (Clear To Send)를 읽습니다
	 * 
	 * @param[in]	index	대상 채널입니다 (0 ~ 1)
	 * @return 상태입니다
	 * 
	 * @note 대상 채널이 RS232 Mode로 설정되어 있어야합니다
	 */
	static bool get_Interface_CTS(uint8_t index);

	/**
	 * @brief 지정된 통신 채널의 Handle을 읽습니다
	 * 
	 * @param[in]	index	(0 ~ 1)
	 * @return Handle의 Pointer입니다 (nullptr: 올바르지 않은 Index)
	 */
	static UART_HandleTypeDef* get_Interface_Handle(uint8_t index);

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

	/**
	 * @brief DB9 커넥터에 12V 출력을 설정합니다
	 * 
	 * @param[in]	enable	활성화 여부입니다
	 */
	static void set_DB9_12V_Output(bool enable);

private:
	static constexpr uint8_t uart_channel_count = 2;
	static constexpr uint8_t vi_channel_count = 4;
	static constexpr uint16_t rx_led_pos[uart_channel_count] = {0x4000, 0x0002};
	static constexpr uint16_t tx_led_pos[uart_channel_count] = {0x8000, 0x0001};
	static constexpr uint16_t relay_led_pos = 0x00E0;
	static uint32_t last_rx_tick_ms[uart_channel_count];
	static uint16_t led_value;
	static bool power_12v_enabled;
	static Protocol::Interface::Setting_Buffer uart_setting[uart_channel_count];

	static GPIO_Init_Info power_relay_info[];
	static GPIO_Init_Info power_generate_info[];
	static GPIO_Init_Info uart_mux_enable_info[];
	static GPIO_Init_Info uart_mux_select_info[];
	static GPIO_Init_Info db9_mux_enable_info[];
	static GPIO_Init_Info db9_mux_select_info[];
	static GPIO_Init_Info db9_12v_output_enable_info[];

	static GPIO_Init_Info db9_rs232_rtscts_enable_info[];
	static GPIO_Init_Info db9_rs485_txrx_enable_info[];
	static GPIO_Init_Info db9_rs422_txrx_enable_info[];
	static GPIO_Init_Info db9_sdi_tx_enable_info[];

	static INTERFACE_MODE serial_interface[uart_channel_count];
	struct Buffer
	{
		bool enable;
		uint16_t count;
		uint8_t buffer[256];
	};
	static Buffer serial_interface_received_buffer[uart_channel_count];

	static TLA2528 adc;

	/**
	 * @brief Sensor Interface의 System 전원을 설정합니다
	 * 
	 * @param[in]	enable	활성화 여부입니다
	 * @retval 0	성공
	 * @retval 1	PG 확인 실패
	 */
	static uint8_t set_System_Power(bool enable);

	/**
	 * @brief UART MUX의 활성화 여부를 설정합니다
	 * 
	 * @param[in]	index		대상 채널입니다 (0 ~ 1)
	 * @param[in]	enable		활성화 여부입니다
	 * @retval 0	성공
	 * @retval 1	실패
	 */
	static uint8_t set_UART_Mux(uint8_t index, bool enable);

	/**
	 * @brief UART MUX의 Target을 설정합니다
	 * 
	 * @param[in]	index			대상 채널입니다 (0 ~ 1)
	 * @param[in]	target_index	Target Index입니다 (0 ~ 3)
	 * @retval 0	성공
	 * @retval 1	실패
	 */
	static uint8_t set_UART_Mux_Target(uint8_t index, uint8_t target_index);

	/**
	 * @brief DB9 MUX의 활성화 여부를 설정합니다
	 * 
	 * @param[in]	index		대상 채널입니다 (0 ~ 1)
	 * @param[in]	enable		활성화 여부입니다
	 * @retval 0	성공
	 * @retval 1	실패
	 */
	static uint8_t set_DB9_Mux(uint8_t index, bool enable);

	/**
	 * @brief DB9 MUX의 Target을 설정합니다
	 * 
	 * @param[in]	index			대상 채널입니다 (0 ~ 1)
	 * @param[in]	target_index	Target Index입니다 (0 ~ 3)
	 * @retval 0	성공
	 * @retval 1	실패
	 */
	static uint8_t set_DB9_Mux_Target(uint8_t index, uint8_t target_index);

	/**
	 * @brief RX LED의 상태를 업데이트합니다
	 * 
	 */
	static void update_RX_LED_Status();
};

#endif /* INCLUDE_SENSOR_INTERFACE_BOARD */
