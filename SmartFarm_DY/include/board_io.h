/**
 * @file board_io.h
 * @author Kwon Jinwoo (jinwoo@xslab.co.kr)
 * @brief SmartFarmST32 Board IO
 * @date 2025-08-18
 */

#ifndef INCLUDE_BOARD_IO
#define INCLUDE_BOARD_IO

#include <stm32g0xx.h>
#include <mxc6655.h>
#include <protocol.h>
#include <packet_manager.h>

enum class STATUS_LED_TYPE : uint8_t { INPUT_POWER, V24_STATUS, V3P3_STATUS, GREEN, BLUE, YELLOW, RS485_ACT };

enum class INTERFACE_BOARD_TYPE : uint8_t { SENSOR, CONTROL };

struct GPIO_Init_Info
{
	GPIO_TypeDef* gpio;
	GPIO_InitTypeDef init;
};


/**
 * @brief System Clock Configuration
 * 
 */
void init_Clock();

/**
 * @brief GPIO를 초기화 합니다
 * 
 */
void init_GPIO();

/**
 * @brief DAC를 초기화합니다
 * 
 */
void init_DAC();

/**
 * @brief I2C를 초기화합니다
 * 
 */
void init_I2C();

/**
 * @brief UART를 초기화합니다
 * 
 */
void init_UART();

/**
 * @brief Watchdog을 초기화합니다
 * 
 */
void init_Watchdog();

/**
 * @brief UART의 데이터 수신을 시작합니다
 * 
 * @param[in]	index	대상 UART Index입니다 (0: UART1, 1: UART2, 2: UART3, 3: UART5)
 */
void set_UART_Receive_Start(uint8_t index);

/**
 * @brief Watchdog의 Timer를 초기화합니다
 * 
 */
void set_Watchdog_Timer_Reset();

/**
 * @brief I2C의 HAL Handle을 읽습니다
 * 
 * @param[in]	i2c_index	대상 I2C Index입니다 (0: I2C1, 1: I2C2)
 * @return Handle의 Pointer입니다 (nullptr: 올바르지 않은 Index)
 */
I2C_HandleTypeDef* get_I2C_Handle(uint8_t i2c_index);

/**
 * @brief UART의 HAL Handle을 읽습니다
 * 
 * @param[in]	uart_index	대상 UART Index입니다 (0: UART1, 1: UART2, 2: UART3, 3: UART5)
 * @return Handle의 Pointer입니다 (nullptr: 올바르지 않은 Index)
 */
UART_HandleTypeDef* get_UART_Handle(uint8_t uart_index);

class MXC6655;
/**
 * @brief 가속도 센서의 객체를 읽습니다
 * 
 * @return Object입니다
 */
MXC6655* get_Accel_Object();

/**
 * @brief Interface Board Type을 읽습니다
 * 
 * @return 연결되어있는 Board의 종류입니다
 */
INTERFACE_BOARD_TYPE get_Interface_Board_Type();

/**
 * @brief Interface Board Version을 읽습니다
 * 
 * @return 연결되어있는 Board의 버전 정보입니다
 */
uint8_t get_Interface_Board_Version();

/**
 * @brief +24V PG의 상태를 읽습니다
 * 
 * @return 상태 입니다
 */
bool get_P24V_PG_Status();

/**
 * @brief OnBoard LED Driver의 On/Off 상태를 읽습니다
 * 
 * @return 현재 값입니다 (0 Bit: LED0 ...)
 */
uint16_t get_LED_Driver_Value();

/**
 * @brief 수신된 데이터를 읽습니다
 * 
 * @param[in]	index	대상 UART Index입니다 (0: UART1, 1: UART2, 2: UART3, 3: UART5)
 * @return 데이터 입니다
 */
uint8_t get_UART_Received_Byte(uint8_t index);

/**
 * @brief Status LED를 설정합니다
 * 
 * @param[in]	type		대상 LED Type입니다
 * @param[in]	enable		활성화 여부입니다
 */
void set_Status_LED(STATUS_LED_TYPE type, bool enable);

/**
 * @brief OnBoard LED Driver를 제어합니다
 * 
 * @param[in]	enable		활성화 여부입니다
 */
void set_LED_Driver_Enable(bool enable);

/**
 * @brief OnBoard LED Driver의 On/Off를 제어합니다
 * 
 * @param[in]	value	설정할 값입니다 (0 Bit: LED0 ...)
 */
void set_LED_Driver_Value(uint16_t value);

/**
 * @brief Buzzer (Fault) 를 설정합니다
 * 
 * @param[in]	enable	활성화 여부입니다
 */
void set_Buzzer(bool enable);

/**
 * @brief DAC 출력을 설정합니다
 * 
 * @param[in]	value	출력 값입니다(0 ~ 4095)
 */
void set_DAC_Value(uint16_t value);

/**
 * @brief DAC 출력을 설정합니다
 * 
 * @param[in]	value	출력 전압입니다(0 ~ 3.3)
 */
void set_DAC_Voltage(float value);

/**
 * @brief GPS의 Reset 상태를 설정합니다
 * 
 * @param[in]	enable	reset 상태입니다
 */
void set_GPS_Reset(bool enable);

/**
 * @brief OnBoard Isolated RS485 TX의 활성화 여부를 설정합니다
 * 
 * @param[in]	enable	활성화 여부입니다
 */
void set_Isolated_RS485_TX_Enable(bool enable);

/**
 * @brief 가속도 값 읽기를 테스트합니다
 * 
 */
void set_Accel_Value_Test();

/**
 * @brief Status LED의 상태를 업데이트 합니다
 * 
 */
void update_Status_LED();

#endif /* INCLUDE_BOARD_IO */
