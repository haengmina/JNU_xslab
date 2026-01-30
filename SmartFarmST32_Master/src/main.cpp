/**
 * @file main.cpp
 * @author Kwon Jinwoo (jinwoo@xslab.co.kr)
 * @brief SmartFarmST32
 * @date 2025-08-18
 */

#include <stdio.h>
#include <stm32g0xx.h>
#include <board_io.h>
#include <string.h>
#include <onboard_packet_handler.h>
#include <control_interface_board.h>
#include <sensor_interface_board.h>
#include <firmware_manager.h>
#include <system_config.h>
#include <packet_manager.h>
#ifndef STDOUT_TO_UART_ENABLE
#include <SEGGER_RTT.h>
#endif

#define _BV(x) (1 << x)
const char *uart_test_message = "Hello\n";

Packet_Manager manager;

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance == USART1)
	{
		set_UART_Receive_Start(0);
		manager.set_Received_Data(get_UART_Received_Byte(0));
	}
	if(huart->Instance == USART2)
	{
		set_UART_Receive_Start(1);
		OnBoard_Packet_Handler::set_GPS_Data(get_UART_Received_Byte(1));
	}
	else if(huart->Instance == USART3)
	{
		set_UART_Receive_Start(2);
		auto data = get_UART_Received_Byte(2);
		Sensor_Interface_Board::set_Interface_RX_Data(1, data);
	}
	else if(huart->Instance == USART5)
	{
		set_UART_Receive_Start(3);
		auto data = get_UART_Received_Byte(3);
		Sensor_Interface_Board::set_Interface_RX_Data(0, data);
	}
}

#ifdef STATUS_TEST_LOOP_ENABLE
void update_Test_Loop()
{
	static uint32_t last_loop_tick = HAL_GetTick();
	uint32_t current_tick = HAL_GetTick();

	if((current_tick - last_loop_tick) < 1000) return;
	last_loop_tick = current_tick;

	auto led_driver = HAL_I2C_IsDeviceReady(get_I2C_Handle(0), 0xC0, 10, 100);
	auto accel_driver = HAL_I2C_IsDeviceReady(get_I2C_Handle(0), 0x2A, 10, 100);
	auto adc_driver = HAL_I2C_IsDeviceReady(get_I2C_Handle(1), (get_Interface_Board_Type() == INTERFACE_BOARD_TYPE::CONTROL) ? 0x20 : 0x28, 10, 100);
	printf("LED Sink Driver ACK: %d, ADC Converter ACK: %d, Accel Device ACK: %d\n", (led_driver == HAL_OK), (adc_driver == HAL_OK), (accel_driver == HAL_OK));

	switch(get_Interface_Board_Type())
	{
		case INTERFACE_BOARD_TYPE::CONTROL:
			Control_Interface_Board::set_DI_Status_Test();
			Control_Interface_Board::set_Manual_Control_Switch_Status_Test();
			Control_Interface_Board::set_DO_Current_RAW_Test();
			Control_Interface_Board::set_DO_Current_mA_Test();
			break;

		case INTERFACE_BOARD_TYPE::SENSOR:
			Sensor_Interface_Board::set_Voltage_Measuremnet_Test();
			Sensor_Interface_Board::set_Current_Measuremnet_Test();
			break;

		default:
			break;
	}

	set_Accel_Value_Test();	
}
#endif

#ifdef STDOUT_TO_UART_ENABLE
extern "C" int _write(int file, char *ptr, int len) {
    HAL_UART_Transmit(get_UART_Handle(0), (uint8_t*)ptr, len, HAL_MAX_DELAY);
    return len;
}
#endif

uint8_t Received_Packet_Handler(Packet_Manager* const manager, const Protocol::Packet_Ptr* const request, Protocol::Command_Response_Packet &response)
{
	uint8_t handler_ret = 2;
	switch(get_Interface_Board_Type())
	{
		case INTERFACE_BOARD_TYPE::CONTROL:
			handler_ret = Control_Interface_Board::Received_Packet_Handler(manager, request, response);
			break;

		case INTERFACE_BOARD_TYPE::SENSOR:
			handler_ret = Sensor_Interface_Board::Received_Packet_Handler(manager, request, response);
			break;
	}
	if(handler_ret == 2) { handler_ret = Firmware_Manager::Received_Packet_Handler(manager, request, response); }
	if(handler_ret == 2) { handler_ret = OnBoard_Packet_Handler::Handler(manager, request, response); }

	switch(handler_ret)
	{
		case 1:
			response.header.command = Protocol::COMMAND::REQUEST_FAILED;
			break;

		case 2:
			return 1;

		default:
			break;
	}
	return 0;
}

int main()
{
#ifndef STDOUT_TO_UART_ENABLE
	SEGGER_RTT_Init();
	setbuf(stdout, NULL);
#endif

	HAL_Init();
	init_Clock();
	init_GPIO();
	init_DAC();
	init_I2C();
	init_UART();
#ifndef DEBUG
	init_Watchdog();
#endif
	uint64_t serial_number = HAL_GetUIDw1();
	serial_number <<= 32;
	serial_number |= HAL_GetUIDw0();
	manager.init(get_UART_Handle(0), serial_number, Received_Packet_Handler);
	printf("SmartFarm ST32 %s - %s\n", VERSION, COMMIT_HASH);
	printf("System Clock: %ldMhz, Node SN (HEX): %08lX%08lX\n",  (HAL_RCC_GetSysClockFreq() / 1000000), (uint32_t)(serial_number >> 32), (uint32_t)(serial_number));

	printf("System Configuration Vaild Status: %d\n", SystemConfig::get_Valid_Status());

	set_GPS_Reset(false);
	set_UART_Receive_Start(0);
	set_UART_Receive_Start(1);

	if(get_Interface_Board_Type() == INTERFACE_BOARD_TYPE::CONTROL)
	{
		printf("Control Interface Board Connected, Version: %d\n", get_Interface_Board_Version());
		Control_Interface_Board::init();
	}
	else
	{
		printf("Sensor Interface Board Connected, Version: %d\n", get_Interface_Board_Version());
		Sensor_Interface_Board::init();
	}

	set_LED_Driver_Enable(true);
	while(1)
	{
#ifndef DEBUG
		set_Watchdog_Timer_Reset();
#endif
		if(SystemConfig::get_Reset_Request_Flag())
		{
			init_Watchdog();
			while(1); //Wait For Reset by WDT
		}

		update_Status_LED();
		manager.update_Received_Packet();
		manager.update_Transmit_Packet();
		switch(get_Interface_Board_Type())
		{
			case INTERFACE_BOARD_TYPE::CONTROL:
				Control_Interface_Board::update_DO_Status();
				Control_Interface_Board::update_DI_Status();
				Control_Interface_Board::update_Manual_Switch_Status();
				Control_Interface_Board::update_ADC_Value();
				break;
			
			case INTERFACE_BOARD_TYPE::SENSOR:
				Sensor_Interface_Board::update_ADC_Value();
				break;

		}
#ifdef STATUS_TEST_LOOP_ENABLE
		update_Test_Loop();
#endif
	}
}	