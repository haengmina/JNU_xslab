/**
 * @file control_interface_board.cpp
 * @author Kwon Jinwoo (jinwoo@xslab.co.kr)
 * @brief C-IF (Control Interface) Board Handling
 * @date 2025-08-22
 */

#include <control_interface_board.h>
#define _BV(x) (1 << x)

TLA2528 Control_Interface_Board::adc = TLA2528();
uint16_t Control_Interface_Board::do_current_zero_point_mv[] = {};

GPIO_Init_Info Control_Interface_Board::digital_output_info[] = {
	{ GPIOC, {.Pin = _BV(8), GPIO_MODE_OUTPUT_PP, GPIO_NOPULL }}, //DO1_ON
	{ GPIOC, {.Pin = _BV(9), GPIO_MODE_OUTPUT_PP, GPIO_NOPULL }}, //DO2_ON
	{ GPIOD, {.Pin = _BV(0), GPIO_MODE_OUTPUT_PP, GPIO_NOPULL }}, //DO3_ON
	{ GPIOD, {.Pin = _BV(1), GPIO_MODE_OUTPUT_PP, GPIO_NOPULL }}, //DO4_ON
	{ GPIOD, {.Pin = _BV(2), GPIO_MODE_OUTPUT_PP, GPIO_NOPULL }}, //DO5_ON
	{ GPIOD, {.Pin = _BV(7), GPIO_MODE_OUTPUT_PP, GPIO_NOPULL }}, //DO6_ON
	{ GPIOF, {.Pin = _BV(9), GPIO_MODE_OUTPUT_PP, GPIO_NOPULL }}, //DO7_ON
	{ GPIOF, {.Pin = _BV(10), GPIO_MODE_OUTPUT_PP, GPIO_NOPULL }}, //DO8_ON
};

GPIO_Init_Info Control_Interface_Board::digital_input_info[] = {
	{ GPIOD, {.Pin = _BV(12), GPIO_MODE_INPUT, GPIO_NOPULL }}, //INT_DI1
	{ GPIOD, {.Pin = _BV(13), GPIO_MODE_INPUT, GPIO_NOPULL }}, //INT_DI2
	{ GPIOD, {.Pin = _BV(14), GPIO_MODE_INPUT, GPIO_NOPULL }}, //INT_DI3
	{ GPIOD, {.Pin = _BV(15), GPIO_MODE_INPUT, GPIO_NOPULL }}, //INT_DI4
	{ GPIOA, {.Pin = _BV(10), GPIO_MODE_INPUT, GPIO_NOPULL }}, //INT_DI5
	{ GPIOA, {.Pin = _BV(11), GPIO_MODE_INPUT, GPIO_NOPULL }}, //INT_DI6
	{ GPIOA, {.Pin = _BV(12), GPIO_MODE_INPUT, GPIO_NOPULL }}, //INT_DI7
	{ GPIOF, {.Pin = _BV(8), GPIO_MODE_INPUT, GPIO_NOPULL }}, //INT_DI8
};

GPIO_Init_Info Control_Interface_Board::manual_control_input_info[] = {
	{ GPIOF, {.Pin = _BV(11), GPIO_MODE_INPUT, GPIO_NOPULL }}, //MANUAL_DO1_ON
	{ GPIOF, {.Pin = _BV(12), GPIO_MODE_INPUT, GPIO_NOPULL }}, //MANUAL_DO2_ON
	{ GPIOF, {.Pin = _BV(13), GPIO_MODE_INPUT, GPIO_NOPULL }}, //MANUAL_DO3_ON
	{ GPIOB, {.Pin = _BV(3), GPIO_MODE_INPUT, GPIO_NOPULL }}, //MANUAL_DO4_ON
	{ GPIOB, {.Pin = _BV(4), GPIO_MODE_INPUT, GPIO_NOPULL }}, //MANUAL_DO5_ON
	{ GPIOB, {.Pin = _BV(5), GPIO_MODE_INPUT, GPIO_NOPULL }}, //MANUAL_DO6_ON
	{ GPIOE, {.Pin = _BV(0), GPIO_MODE_INPUT, GPIO_NOPULL }}, //MANUAL_DO7_ON
	{ GPIOE, {.Pin = _BV(1), GPIO_MODE_INPUT, GPIO_NOPULL }}, //MANUAL_DO8_ON
};
uint16_t Control_Interface_Board::led_value = 0x00;
uint16_t Control_Interface_Board::last_switch_value = 0xFFFF; //Active Low
uint16_t Control_Interface_Board::do_load_detect_threshold_ma = 300; //DO가 Open 상태일 때, 150 mA 이상 흐르는 것으로 측정 결과가 나올 수 있습니다

void Control_Interface_Board::init()
{
	for(auto &target : digital_output_info)
	{
		HAL_GPIO_Init(target.gpio, &target.init);
	}
	for(auto &target : digital_input_info)
	{
		HAL_GPIO_Init(target.gpio, &target.init);
	}
	for(auto &target : manual_control_input_info)
	{
		HAL_GPIO_Init(target.gpio, &target.init);
	}

	set_DI_Threshold_Voltage(DI_THRESHOLD::P18V);
	adc.init(get_I2C_Handle(1), 0x20, 3.3);
	for(auto &target : do_current_zero_point_mv)
	{
		target = 1650; //1.65V
	}
}

uint8_t Control_Interface_Board::set_DI_Threshold_Voltage(DI_THRESHOLD value)
{
	switch(value)
	{
		case DI_THRESHOLD::P18V:
			set_DAC_Voltage(1.5);
			break;

		case DI_THRESHOLD::P24V:
			set_DAC_Voltage(2.0);
			break;

		case DI_THRESHOLD::P39V:
			set_DAC_Voltage(3.2);
			break;

		default:
			return 1;
	}
	return 0;
}

uint8_t Control_Interface_Board::set_DO_Status(uint8_t index, bool enable)
{
	if(index >= digital_channel_count) return 1;

	auto &target = digital_output_info[index];
	
	if(enable)
	{
		target.gpio->ODR |= target.init.Pin;
		led_value |= _BV(index);
	}
	else
	{
		target.gpio->ODR &= ~target.init.Pin;
		led_value &= ~_BV(index);
	}
	set_LED_Driver_Value(led_value);

	return 0;
}

void Control_Interface_Board::set_DO_Status_Test()
{
	for(uint8_t i = 0; i < digital_channel_count; i++)
	{
		set_DO_Status(i, true);
		HAL_Delay(1000);
	}
}

void Control_Interface_Board::set_DI_Status_Test()
{
	for(uint8_t i = 0; i < digital_channel_count; i++)
	{		
		printf("DI%d: %d ", (i + 1), get_DI_Status(i));
	}
	printf("\n");
}

void Control_Interface_Board::set_Manual_Control_Switch_Status_Test()
{
	for(uint8_t i = 0; i < digital_channel_count; i++)
	{
		printf("Manual%d: %d ", (i + 1), get_Manual_Control_Status(i));
	}
	printf("\n");
}

void Control_Interface_Board::set_DO_Current_RAW_Test()
{
	for(uint8_t i = 0; i < digital_channel_count; i++)
	{
		printf("DO%d: %.3fV ", (i + 1), get_DO_Current_RAW(i));
	}
	printf("\n");
}

void Control_Interface_Board::set_DO_Current_mA_Test()
{
	for(uint8_t i = 0; i < digital_channel_count; i++)
	{
		printf("DO%d: %dmA ", (i + 1), get_DO_Current_mA(i));
	}
	printf("\n");
}

bool Control_Interface_Board::get_DI_Status(uint8_t index)
{
	if(index >= digital_channel_count) return 1;

	auto &target = digital_input_info[index];

	return (target.gpio->IDR & target.init.Pin);
}

bool Control_Interface_Board::get_DO_Status(uint8_t index)
{
	if(index >= digital_channel_count) return false;

	auto &target = digital_output_info[index];	

	return (target.gpio->ODR & target.init.Pin);
}

bool Control_Interface_Board::get_Manual_Control_Status(uint8_t index)
{
	if(index >= digital_channel_count) return 1;

	auto &target = manual_control_input_info[index];

	return (target.gpio->IDR & target.init.Pin);
}

float Control_Interface_Board::get_DO_Current_RAW(uint8_t index)
{
	return adc.get_Voltage(digital_channel_count - index - 1);
}

int16_t Control_Interface_Board::get_DO_Current_mA(uint8_t index)
{
	int16_t raw_mv = get_DO_Current_RAW(index) * 1000;
	if(!get_DO_Status(index))
	{
		do_current_zero_point_mv[index] = raw_mv;
	}

	float current_a = ((float)raw_mv - do_current_zero_point_mv[index]) / 132;
	return (current_a * 1000);
}

uint8_t Control_Interface_Board::update_ADC_Value()
{
	return adc.update_Value();
}

void Control_Interface_Board::update_DI_Status()
{
	uint8_t offset = 15;
	for(uint8_t i = 0; i < digital_channel_count; i++)
	{
		if(get_DI_Status(i))
		{
			led_value |= _BV((offset - i));
		}
		else
		{
			led_value &= ~_BV((offset - i));
		}
	}
	set_LED_Driver_Value(led_value);
}

void Control_Interface_Board::update_DO_Status()
{
	static uint32_t last_check_tick = HAL_GetTick();
	uint32_t current_tick = HAL_GetTick();
	if((current_tick - last_check_tick) < 300) return;
	last_check_tick = current_tick;

	static bool blink_status = false;
	blink_status ^= true;

	for(uint8_t i = 0; i < digital_channel_count; i++)
	{
		if(!get_DO_Status(i)) { continue; }

		int16_t value = get_DO_Current_mA(i);
		if(value < 0) { value *= -1; }

		//Digital Output Enable, Load NOT Detected, LED Blink
		if(value <= do_load_detect_threshold_ma)
		{
			if(blink_status)
			{
				led_value |= _BV(i);
			}
			else
			{
				led_value &= ~_BV(i);
			}
			set_LED_Driver_Value(led_value);
		}
		else
		{
			led_value |= _BV(i);
			set_LED_Driver_Value(led_value);
		}
	}
}

void Control_Interface_Board::update_Manual_Switch_Status()
{
	static uint32_t last_check_tick = HAL_GetTick();
	uint32_t current_tick = HAL_GetTick();
	if((current_tick - last_check_tick) < 100) return;
	last_check_tick = current_tick;

	for(uint8_t i = 0; i < digital_channel_count; i++)
	{
		if(get_Manual_Control_Status(i))
		{
			if(!(last_switch_value & _BV(i)))
			{
				set_DO_Status(i, !get_DO_Status(i));
			}
			last_switch_value |= _BV(i);
		}
		else
		{
			last_switch_value &= ~_BV(i);
		}
	}
}

uint8_t Control_Interface_Board::Received_Packet_Handler(Packet_Manager* const manager, const Protocol::Packet_Ptr* const request, Protocol::Command_Response_Packet &response)
{
	switch(request->header.command)
	{
		case Protocol::COMMAND::DI_READ:
		{
			if(request->header.data_length != 1) return 1;

			uint8_t number = request->data[0];
			if(!number || (number > digital_channel_count)) return 1;

			response.header.command = Protocol::COMMAND::DI_RESPONSE;
			bool value = Control_Interface_Board::get_DI_Status(number - 1);
			response.init_Data((uint8_t*)&value, 1);

			printf("DI%d Read Request, Status: %d\n", number, value);
			break;
		}

		case Protocol::COMMAND::DI_THRESHOLD_WRITE:
		{
			if(request->header.data_length != 1) return 1;
			uint8_t value = request->data[0];
			if(Control_Interface_Board::set_DI_Threshold_Voltage((Control_Interface_Board::DI_THRESHOLD)value)) return 1;

			response.header.command = Protocol::COMMAND::REQUEST_SUCCESS;

			printf("DI Threshold Write Request, Value: %d\n", value);
			break;
		}

		case Protocol::COMMAND::DO_READ:
		{
			if(request->header.data_length != 1) return 1;
			uint8_t number = request->data[0];
			if(!number || (number > digital_channel_count)) return 1;

			response.header.command = Protocol::COMMAND::DO_RESPONSE;
			uint8_t buffer[3] = {};

			buffer[0] = Control_Interface_Board::get_DO_Status(number - 1);
			*((int16_t*)&buffer[1]) = get_DO_Current_mA(number - 1);
			response.init_Data(buffer, sizeof(buffer));

			printf("DO%d Read Request, Status: %d, %d\n", number, buffer[0], *((int16_t*)&buffer[1]));
			break;
		}

		case Protocol::COMMAND::DO_WRITE:
		{
			if(request->header.data_length != 2) return 1;
			uint8_t number = request->data[0];
			bool value = request->data[1];

			if(Control_Interface_Board::set_DO_Status(number - 1, value)) return 1;
			response.header.command = Protocol::COMMAND::REQUEST_SUCCESS;

			printf("DO%d Write Request, Status: %d\n", number, value);
			break;
		}

		case Protocol::COMMAND::DI_DO_READ_ALL:
		{
			if(request->header.data_length) return 1;

			response.header.command = Protocol::COMMAND::DI_DO_READ_ALL_RESPONSE;
			uint8_t buffer[(digital_channel_count * 2) + (digital_channel_count * sizeof(uint16_t))] = {}; //(DI * 8) + (DO * 8) + (DO Current(uint16_t) * 8)

			for(uint8_t i = 0; i < digital_channel_count; i++)
			{
				buffer[i] = Control_Interface_Board::get_DI_Status(i);
				buffer[i + digital_channel_count] = Control_Interface_Board::get_DO_Status(i);
				int16_t *current_data_ptr = (int16_t*)(buffer + (digital_channel_count * 2));
				current_data_ptr[i] = get_DO_Current_mA(i);
			}
			
			response.init_Data(buffer, sizeof(buffer));

			printf("DI DO DO_Current Read All Request\n");
			break;
		}

		case Protocol::COMMAND::DO_WRITE_ALL:
		{
			if(request->header.data_length != digital_channel_count) return 1;

			for(uint8_t i = 0; i < digital_channel_count; i++)
			{
				if(Control_Interface_Board::set_DO_Status(i, request->data[i])) return 1;
			}
			response.header.command = Protocol::COMMAND::REQUEST_SUCCESS;
			printf("DO Write All Request\n");
			break;
		}

		default:
			return 2;
	}
	return 0;
}