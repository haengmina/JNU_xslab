/**
 * @file sensor_interface_board.cpp
 * @author Kwon Jinwoo (jinwoo@xslab.co.kr)
 * @brief S-IF (Sensor Interface) Board Handling
 * @date 2025-08-22
 */

#include <sensor_interface_board.h>
#define _BV(x) (1 << x)

GPIO_Init_Info Sensor_Interface_Board::power_relay_info[] = {
	{ GPIOD, {.Pin = _BV(14), GPIO_MODE_OUTPUT_PP, GPIO_NOPULL }}, //5V_OUT_CTRL
	{ GPIOD, {.Pin = _BV(15), GPIO_MODE_OUTPUT_PP, GPIO_NOPULL }}, //12V_OUT_CTRL
	{ GPIOA, {.Pin = _BV(10), GPIO_MODE_OUTPUT_PP, GPIO_NOPULL }}, //24V_OUT_CTRL
};

GPIO_Init_Info Sensor_Interface_Board::power_generate_info[] = {
	{ GPIOA, {.Pin = _BV(11), GPIO_MODE_INPUT, GPIO_NOPULL }}, //12V_SYS_PG
	{ GPIOF, {.Pin = _BV(13), GPIO_MODE_OUTPUT_PP, GPIO_NOPULL }}, //SYSTEM_POWER_ON
	{ GPIOB, {.Pin = _BV(3), GPIO_MODE_OUTPUT_PP, GPIO_NOPULL }}, //POSITIVE_12V_EN
	{ GPIOB, {.Pin = _BV(5), GPIO_MODE_OUTPUT_PP, GPIO_NOPULL }}, //NEGATIVE_12V_EN
	{ GPIOF, {.Pin = _BV(10), GPIO_MODE_OUTPUT_PP, GPIO_NOPULL }}, //NEGATIVE_12V_TIMER_EN
	{ GPIOB, {.Pin = _BV(4), GPIO_MODE_OUTPUT_PP, GPIO_NOPULL }}, //POSITIVE_PWR_SAVE
	{ GPIOE, {.Pin = _BV(0), GPIO_MODE_OUTPUT_PP, GPIO_NOPULL }}, //NEGATIVE_PWR_SAVE
};

GPIO_Init_Info Sensor_Interface_Board::uart_mux_enable_info[] = {
	{ GPIOD, {.Pin = _BV(0), GPIO_MODE_OUTPUT_PP, GPIO_NOPULL }}, //DB9_CH1_UART_MUX_EN
	{ GPIOD, {.Pin = _BV(1), GPIO_MODE_OUTPUT_PP, GPIO_NOPULL }}, //DB9_CH2_UART_MUX_EN
};

GPIO_Init_Info Sensor_Interface_Board::uart_mux_select_info[] = {
	{ GPIOA, {.Pin = _BV(9), GPIO_MODE_OUTPUT_PP, GPIO_NOPULL }}, //DB9_CH1_UART_A0
	{ GPIOC, {.Pin = _BV(6), GPIO_MODE_OUTPUT_PP, GPIO_NOPULL }}, //DB9_CH1_UART_A1
	{ GPIOC, {.Pin = _BV(7), GPIO_MODE_OUTPUT_PP, GPIO_NOPULL }}, //DB9_CH2_UART_A0
	{ GPIOD, {.Pin = _BV(8), GPIO_MODE_OUTPUT_PP, GPIO_NOPULL }}, //DB9_CH2_UART_A1
};

GPIO_Init_Info Sensor_Interface_Board::db9_mux_enable_info[] = {
	{ GPIOF, {.Pin = _BV(11), GPIO_MODE_OUTPUT_PP, GPIO_NOPULL }}, //DB9_CH1_MUX_EN_3P3
	{ GPIOF, {.Pin = _BV(12), GPIO_MODE_OUTPUT_PP, GPIO_NOPULL }}, //DB9_CH2_MUX_EN_3P3
};

GPIO_Init_Info Sensor_Interface_Board::db9_mux_select_info[] = {
	{ GPIOE, {.Pin = _BV(1), GPIO_MODE_OUTPUT_PP, GPIO_NOPULL }}, //DB9_CH1_TMUX_A0
	{ GPIOE, {.Pin = _BV(2), GPIO_MODE_OUTPUT_PP, GPIO_NOPULL }}, //DB9_CH1_TMUX_A1
	{ GPIOA, {.Pin = _BV(12), GPIO_MODE_OUTPUT_PP, GPIO_NOPULL }}, //DB9_CH2_TMUX_A0
	{ GPIOF, {.Pin = _BV(8), GPIO_MODE_OUTPUT_PP, GPIO_NOPULL }}, //DB9_CH2_TMUX_A1
};

GPIO_Init_Info Sensor_Interface_Board::db9_12v_output_enable_info[] = {
	{ GPIOD, {.Pin = _BV(12), GPIO_MODE_OUTPUT_PP, GPIO_NOPULL }}, //CH1_DB9_12V_EN
	{ GPIOD, {.Pin = _BV(13), GPIO_MODE_OUTPUT_PP, GPIO_NOPULL }}, //CH2_DB9_12V_EN
};

GPIO_Init_Info Sensor_Interface_Board::db9_rs232_rtscts_enable_info[] = {
	{ GPIOF, {.Pin = _BV(7), GPIO_MODE_OUTPUT_PP, GPIO_NOPULL }}, //DB9_CH1_UART_RTS
	{ GPIOE, {.Pin = _BV(7), GPIO_MODE_INPUT, GPIO_NOPULL }}, //DB9_CH1_UART_CTS
	{ GPIOA, {.Pin = _BV(6), GPIO_MODE_OUTPUT_PP, GPIO_NOPULL }}, //DB9_CH2_UART_RTS
	{ GPIOB, {.Pin = _BV(1), GPIO_MODE_INPUT, GPIO_NOPULL }}, //DB9_CH2_UART_CTS
};

GPIO_Init_Info Sensor_Interface_Board::db9_rs485_txrx_enable_info[] = {
	//D1.0, 설계상의 문제로 DB9_CH1_RS485_DE를 사용할 수 없으며, DB9_CH1_RS485_DE와 DB9_CH1_RS485_RE# 가 연결되어있습니다
	// { GPIOB, {.Pin = _BV(15), GPIO_MODE_OUTPUT_PP, GPIO_NOPULL }}, //DB9_CH1_RS485_DE
	{ GPIOA, {.Pin = _BV(8), GPIO_MODE_OUTPUT_PP, GPIO_NOPULL }}, //DB9_CH1_RS485_RE#
	{ GPIOC, {.Pin = _BV(8), GPIO_MODE_OUTPUT_PP, GPIO_NOPULL }}, //DB9_CH2_RS485_DE
	{ GPIOC, {.Pin = _BV(9), GPIO_MODE_OUTPUT_PP, GPIO_NOPULL }}, //DB9_CH2_RS485_RE#
};

GPIO_Init_Info Sensor_Interface_Board::db9_rs422_txrx_enable_info[] = {
	//D1.0, 회로도에서 DE/RE가 반대로 연결됨
	{ GPIOE, {.Pin = _BV(9), GPIO_MODE_OUTPUT_PP, GPIO_NOPULL }}, //DB9_CH1_RS422_DE
	{ GPIOE, {.Pin = _BV(12), GPIO_MODE_OUTPUT_PP, GPIO_NOPULL }}, //DB9_CH1_RS422_RE#
	{ GPIOA, {.Pin = _BV(7), GPIO_MODE_OUTPUT_PP, GPIO_NOPULL }}, //DB9_CH2_RS422_DE
	{ GPIOB, {.Pin = _BV(0), GPIO_MODE_OUTPUT_PP, GPIO_NOPULL }}, //DB9_CH2_RS422_RE#
};

GPIO_Init_Info Sensor_Interface_Board::db9_sdi_tx_enable_info[] = {
	{ GPIOE, {.Pin = _BV(8), GPIO_MODE_OUTPUT_PP, GPIO_NOPULL }}, //DB9_CH1_SDI_TX_EN
	{ GPIOA, {.Pin = _BV(5), GPIO_MODE_OUTPUT_PP, GPIO_NOPULL }}, //DB9_CH2_SDI_TX_EN
};

Sensor_Interface_Board::INTERFACE_MODE Sensor_Interface_Board::serial_interface[] = {
	Sensor_Interface_Board::INTERFACE_MODE::RS232,
	Sensor_Interface_Board::INTERFACE_MODE::RS232
};

Sensor_Interface_Board::Buffer Sensor_Interface_Board::serial_interface_received_buffer[uart_channel_count] = {
	{ .enable = false, .count = 0, .buffer = {} }, 
	{ .enable = false, .count = 0, .buffer = {} }, 
};

TLA2528 Sensor_Interface_Board::adc = TLA2528();
uint16_t Sensor_Interface_Board::led_value = 0x00;
uint32_t Sensor_Interface_Board::last_rx_tick_ms[uart_channel_count] = {};
Protocol::Interface::Setting_Buffer Sensor_Interface_Board::uart_setting[uart_channel_count] = {
	{
		.channel = Protocol::Interface::CHANNEL::CH1
	},
	{
		.channel = Protocol::Interface::CHANNEL::CH2
	}
};

void Sensor_Interface_Board::init()
{
	for(auto &target : power_relay_info)
	{
		HAL_GPIO_Init(target.gpio, &target.init);
	}
	for(auto &target : power_generate_info)
	{
		HAL_GPIO_Init(target.gpio, &target.init);
	}
	for(auto &target : uart_mux_enable_info)
	{
		HAL_GPIO_Init(target.gpio, &target.init);
	}
	for(auto &target : uart_mux_select_info)
	{
		HAL_GPIO_Init(target.gpio, &target.init);
	}
	for(auto &target : db9_mux_enable_info)
	{
		HAL_GPIO_Init(target.gpio, &target.init);
	}
	for(auto &target : db9_mux_select_info)
	{
		HAL_GPIO_Init(target.gpio, &target.init);
	}
	for(auto &target : db9_12v_output_enable_info)
	{
		HAL_GPIO_Init(target.gpio, &target.init);
	}

	for(auto &target : db9_rs232_rtscts_enable_info)
	{
		HAL_GPIO_Init(target.gpio, &target.init);
	}
	for(auto &target : db9_rs485_txrx_enable_info)
	{
		HAL_GPIO_Init(target.gpio, &target.init);
	}
	for(auto &target : db9_rs422_txrx_enable_info)
	{
		HAL_GPIO_Init(target.gpio, &target.init);
	}
	for(auto &target : db9_sdi_tx_enable_info)
	{
		HAL_GPIO_Init(target.gpio, &target.init);
	}

	set_System_Power(true);
	set_Power_Output(true);
	set_Interface_Mode(0, INTERFACE_MODE::RS232);
	set_Interface_Mode(1, INTERFACE_MODE::RS232);
}

void Sensor_Interface_Board::set_Power_Output(bool enable)
{
	for(auto &target : power_relay_info)
	{
		if(enable)
		{
			target.gpio->ODR |= target.init.Pin;
			led_value |= relay_led_pos;
		}
		else
		{
			target.gpio->ODR &= ~target.init.Pin;
			led_value &= ~relay_led_pos;
		}
	}

	set_LED_Driver_Value(led_value);
}


uint8_t Sensor_Interface_Board::set_System_Power(bool enable)
{
	for(uint8_t i = 1; i <= 4; i++)
	{
		auto &target = power_generate_info[i];
		if(enable)
		{
			target.gpio->ODR |= target.init.Pin;
		}
		else
		{
			target.gpio->ODR &= ~target.init.Pin;
		}
	}
	HAL_Delay(30);
	auto &sys_pg = power_generate_info[0];
	if(enable && !(sys_pg.gpio->IDR & sys_pg.init.Pin))
	{
		printf("%s: 12V_SYS_PG, Failed, PowerOff\n", __FUNCTION__);
		set_System_Power(false);
		return 1;
	}

	adc.init(get_I2C_Handle(1), 0x28, 5.0);
	
	return 0;
}

uint8_t Sensor_Interface_Board::set_UART_Mux(uint8_t index, bool enable)
{
	if(index >= uart_channel_count) return 1;

	auto &target = uart_mux_enable_info[index];
	if(enable)
	{
		target.gpio->ODR |= target.init.Pin;
	}
	else
	{
		target.gpio->ODR &= ~target.init.Pin;
	}
	return 0;
}

uint8_t Sensor_Interface_Board::set_UART_Mux_Target(uint8_t index, uint8_t target_index)
{
	if((index >= uart_channel_count) || (target_index >= 4)) return 1;
	
	uint8_t target_offset = (index == 0) ? 0 : 2;
	auto &target_a0 = uart_mux_select_info[target_offset];
	auto &target_a1 = uart_mux_select_info[target_offset + 1];
	switch(target_index)
	{
		case 0:
			target_a0.gpio->ODR &= ~target_a0.init.Pin;
			target_a1.gpio->ODR &= ~target_a1.init.Pin;
			break;

		case 1:
			target_a0.gpio->ODR |= target_a0.init.Pin;
			target_a1.gpio->ODR &= ~target_a1.init.Pin;
			break;

		case 2:
			target_a0.gpio->ODR &= ~target_a0.init.Pin;
			target_a1.gpio->ODR |= target_a1.init.Pin;
			break;

		case 3:
			target_a0.gpio->ODR |= target_a0.init.Pin;
			target_a1.gpio->ODR |= target_a1.init.Pin;
			break;
	}

	return 0;
}

uint8_t Sensor_Interface_Board::set_DB9_Mux(uint8_t index, bool enable)
{
	if(index >= uart_channel_count) return 1;

	auto &target = db9_mux_enable_info[index];
	if(enable)
	{
		target.gpio->ODR |= target.init.Pin;
	}
	else
	{
		target.gpio->ODR &= ~target.init.Pin;
	}

	return 0;
}

uint8_t Sensor_Interface_Board::set_DB9_Mux_Target(uint8_t index, uint8_t target_index)
{
	if((index >= uart_channel_count) || (target_index >= 4)) return 1;
	
	uint8_t target_offset = (index == 0) ? 0 : 2;
	auto &target_a0 = db9_mux_select_info[target_offset];
	auto &target_a1 = db9_mux_select_info[target_offset + 1];
	switch(target_index)
	{
		case 0:
			target_a0.gpio->ODR &= ~target_a0.init.Pin;
			target_a1.gpio->ODR &= ~target_a1.init.Pin;
			break;

		case 1:
			target_a0.gpio->ODR |= target_a0.init.Pin;
			target_a1.gpio->ODR &= ~target_a1.init.Pin;
			break;

		case 2:
			target_a0.gpio->ODR &= ~target_a0.init.Pin;
			target_a1.gpio->ODR |= target_a1.init.Pin;
			break;

		case 3:
			target_a0.gpio->ODR |= target_a0.init.Pin;
			target_a1.gpio->ODR |= target_a1.init.Pin;
			break;
	}

	return 0;
}

void Sensor_Interface_Board::set_DB9_12V_Output(bool enable)
{
	for(auto &target : db9_12v_output_enable_info)
	{
		if(enable)
		{
			target.gpio->ODR |= target.init.Pin;
		}
		else
		{
			target.gpio->ODR &= ~target.init.Pin;
		}
	}
}

uint8_t Sensor_Interface_Board::set_Interface_Mode(uint8_t index, INTERFACE_MODE mode)
{
	if(index >= uart_channel_count) return 1;

	switch(mode)
	{
		case INTERFACE_MODE::RS232:
			set_UART_Mux_Target(index, 0);
			set_DB9_Mux_Target(index, 0);
			break;

		case INTERFACE_MODE::RS422:
			set_UART_Mux_Target(index, 1);
			set_DB9_Mux_Target(index, 1);
			break;

		case INTERFACE_MODE::RS485:
			set_UART_Mux_Target(index, 2);
			set_DB9_Mux_Target(index, 2);
			break;

		case INTERFACE_MODE::SDI:
		case INTERFACE_MODE::DDI:
			set_UART_Mux_Target(index, 3);
			//D1.0, 설계상의 문제로, RX 사용시 MUX 2를 사용해야함
			//https://yona.xslab.co.kr/엑세스랩/9.DAEEUN/issue/5 참조
			set_DB9_Mux_Target(index, 2);
			break;
	}
	set_UART_Mux(index, true);
	set_DB9_Mux(index, true);
	serial_interface[index] = mode;

	return 0;
}

uint8_t Sensor_Interface_Board::set_Interface_TX(uint8_t index, bool enable)
{
	if(index >= uart_channel_count) return 1;

	if(enable)
	{
		led_value |= tx_led_pos[index];
	}
	else
	{
		led_value &= ~tx_led_pos[index];
	}
	set_LED_Driver_Value(led_value);

	switch(serial_interface[index])
	{
		case INTERFACE_MODE::RS232:
			//Always Enabled
			break;

		case INTERFACE_MODE::RS422:
		{
			uint8_t offset = (index == 0) ? 0 : 2;
			auto &target = db9_rs422_txrx_enable_info[offset]; //RS422 DE
			if(enable)
			{
				target.gpio->ODR |= target.init.Pin;
			}
			else
			{
				target.gpio->ODR &= ~target.init.Pin;
			}
			break;
		}

		case INTERFACE_MODE::RS485:
		{
			auto &target_de = db9_rs485_txrx_enable_info[index]; //RS485 DE
			auto &target_re = db9_rs485_txrx_enable_info[index + 1]; //RS485 RE#
			if(enable)
			{
				target_de.gpio->ODR |= target_de.init.Pin;
				if(index)
				{
					target_re.gpio->ODR |= target_re.init.Pin;
				}
			}
			else
			{
				target_de.gpio->ODR &= ~target_de.init.Pin;
				if(index)
				{
					target_re.gpio->ODR &= ~target_re.init.Pin;
				}
			}			
			break;
		}

		case INTERFACE_MODE::SDI:
		case INTERFACE_MODE::DDI:
		{
			auto &target = db9_sdi_tx_enable_info[index];
			if(enable)
			{
				target.gpio->ODR |= target.init.Pin;
			}
			else
			{
				target.gpio->ODR &= ~target.init.Pin;
			}
			break;
		}
	}

	return 0;
}

uint8_t Sensor_Interface_Board::set_Interface_RTS(uint8_t index, bool enable)
{
	if((index >= uart_channel_count) || (serial_interface[index] != INTERFACE_MODE::RS232)) return 1;

	uint8_t offset = (index == 0) ? 0 : 2;
	auto &target = db9_rs232_rtscts_enable_info[offset];
	if(enable)
	{
		target.gpio->ODR |= target.init.Pin;
	}
	else
	{
		target.gpio->ODR &= ~target.init.Pin;
	}

	return 0;
}

uint8_t Sensor_Interface_Board::set_Interface_Setting(const Protocol::Interface::Setting_Buffer &param)
{
	UART_HandleTypeDef *target = nullptr;
	uint8_t channel_index;
	switch(param.channel)
	{
		case Protocol::Interface::CHANNEL::CH1:
			channel_index = 0;
			break;

		case Protocol::Interface::CHANNEL::CH2:
			channel_index = 1;
			break;

		default:
			return 1;
	}
	target = get_Interface_Handle(channel_index);

	target->Init.BaudRate = param.baudrate;

	switch(param.databit)
	{
		case Protocol::Interface::DATABIT::BIT7:
			target->Init.WordLength = UART_WORDLENGTH_7B;
			break;

		case Protocol::Interface::DATABIT::BIT8:
			target->Init.WordLength = UART_WORDLENGTH_8B;
			break;

		default:
			return 1;
	}

	switch(param.parity)
	{
		case Protocol::Interface::PARITY::NONE:
			target->Init.Parity = UART_PARITY_NONE;
			break;

		case Protocol::Interface::PARITY::EVEN:
			target->Init.Parity = UART_PARITY_EVEN;
			break;

		case Protocol::Interface::PARITY::ODD:
			target->Init.Parity = UART_PARITY_ODD;
			break;

		default:
			return 1;
	}
	switch(param.stop_bit)
	{
		case Protocol::Interface::STOP_BIT::B1:
			target->Init.StopBits = UART_STOPBITS_1;
			break;

		case Protocol::Interface::STOP_BIT::B2:
			target->Init.StopBits = UART_STOPBITS_2;
			break;

		default:
			return 1;
	}
	switch(param.flow_control)
	{
		case Protocol::Interface::FLOW_CONTROL::NONE:
			target->Init.HwFlowCtl = UART_HWCONTROL_NONE;
			break;

		case Protocol::Interface::FLOW_CONTROL::CTS_RTS:
			target->Init.HwFlowCtl = UART_HWCONTROL_RTS_CTS;
			break;

		default:
			return 1;
	}

	target->AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_TXINVERT_INIT | UART_ADVFEATURE_RXINVERT_INIT;
	if(param.type == Protocol::Interface::TYPE::SDI)
	{
		target->AdvancedInit.RxPinLevelInvert = UART_ADVFEATURE_RXINV_ENABLE;
		target->AdvancedInit.TxPinLevelInvert = UART_ADVFEATURE_TXINV_ENABLE;
	}
	else
	{
		target->AdvancedInit.RxPinLevelInvert = UART_ADVFEATURE_RXINV_DISABLE;
		target->AdvancedInit.TxPinLevelInvert = UART_ADVFEATURE_TXINV_DISABLE;
	}

	target->Init.WordLength = UART_WORDLENGTH_8B;
	target->Init.Mode = UART_MODE_TX_RX;
	target->Init.OverSampling = UART_OVERSAMPLING_16;
	target->Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	target->Init.ClockPrescaler = UART_PRESCALER_DIV1;
	HAL_UART_AbortReceive(target);
 	if(HAL_UART_Init(target) != HAL_OK) return 1;
	if(HAL_UARTEx_SetTxFifoThreshold(target, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK) return 1;
	if(HAL_UARTEx_SetRxFifoThreshold(target, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK) return 1;
	if(HAL_UARTEx_DisableFifoMode(target) != HAL_OK) return 1;
	set_Interface_RX_Start(channel_index);

	switch(param.type)
	{
		case Protocol::Interface::TYPE::RS232:
			set_Interface_Mode(channel_index, Sensor_Interface_Board::INTERFACE_MODE::RS232);
			break;

		case Protocol::Interface::TYPE::RS422:
			set_Interface_Mode(channel_index, Sensor_Interface_Board::INTERFACE_MODE::RS422);
			break;

		case Protocol::Interface::TYPE::RS485:
			set_Interface_Mode(channel_index, Sensor_Interface_Board::INTERFACE_MODE::RS485);
			break;

		case Protocol::Interface::TYPE::DDI:
		case Protocol::Interface::TYPE::SDI:
			set_Interface_Mode(channel_index, Sensor_Interface_Board::INTERFACE_MODE::DDI);
			break;

		default:
			return 1;
	}

	memcpy(&uart_setting[channel_index], &param, sizeof(param));
	return 0;
}

uint8_t Sensor_Interface_Board::set_Interface_RX_Data(uint8_t index, uint8_t data)
{
	if((index >= uart_channel_count) || !serial_interface_received_buffer[index].enable) return 1;
	last_rx_tick_ms[index] = HAL_GetTick();

	auto &target = serial_interface_received_buffer[index];
	target.buffer[target.count++] = data;

	return 0;
}

void Sensor_Interface_Board::set_Interface_RX_Start(uint8_t index)
{
	if(index >= uart_channel_count) return;

	set_UART_Receive_Start((index == 0) ? 3 : 2);
}


void Sensor_Interface_Board::set_Voltage_Measuremnet_Test()
{
	for(uint8_t i = 0; i < vi_channel_count; i++)
	{
		float value = get_Voltage_RAW(i);
		printf("Voltage IN RAW %d: %.3fV ", (i + 1), value);
	}
	printf("\n");

	for(uint8_t i = 0; i < vi_channel_count; i++)
	{
		int16_t value = get_Voltage_mV(i);
		printf("Voltage IN %d: %dmV ", (i + 1), value);
	}
	printf("\n");
}

void Sensor_Interface_Board::set_Current_Measuremnet_Test()
{
	for(uint8_t i = 0; i < vi_channel_count; i++)
	{
		float value = get_Current_RAW(i);
		printf("Current IN %d: %.3fV ", (i + 1), value);
	}
	printf("\n");

	for(uint8_t i = 0; i < vi_channel_count; i++)
	{
		uint16_t value = get_Current_uA(i);
		printf("Current IN %d: %duA ", (i + 1), value);
	}
	printf("\n");
}

bool Sensor_Interface_Board::get_Interface_CTS(uint8_t index)
{
	if((index >= uart_channel_count) || (serial_interface[index] != INTERFACE_MODE::RS232)) return false;

	uint8_t offset = (index == 0) ? 1 : 3;
	auto &target = db9_rs232_rtscts_enable_info[offset];

	return (target.gpio->IDR & target.init.Pin);
}

UART_HandleTypeDef* Sensor_Interface_Board::get_Interface_Handle(uint8_t index)
{
	if(index >= uart_channel_count) return nullptr;

	return get_UART_Handle((index == 0) ? 3 : 2);
}

float Sensor_Interface_Board::get_Voltage_RAW(uint8_t index)
{
	if(index >= vi_channel_count) return 0;

	uint8_t target_index = (index * 2);

	return adc.get_Voltage(target_index);
}

int16_t Sensor_Interface_Board::get_Voltage_mV(uint8_t index)
{
	if(index >= vi_channel_count) return 0;

	uint16_t voltage_mv = get_Voltage_RAW(index) * 1000;
	return map(voltage_mv, 1500, 3500, -10000, 10000);
}

float Sensor_Interface_Board::get_Current_RAW(uint8_t index)
{
	if(index >= vi_channel_count) return 0;

	uint8_t target_index = (index * 2 + 1);
	return adc.get_Voltage(target_index);
}

uint16_t Sensor_Interface_Board::get_Current_uA(uint8_t index)
{
	if(index >= vi_channel_count) return 0;

	uint32_t voltage_uv = get_Current_RAW(index) * 1000 * 1000; //uA Unit
	return voltage_uv / 250;
}

uint8_t Sensor_Interface_Board::update_ADC_Value()
{
	return adc.update_Value();
}

void Sensor_Interface_Board::update_RX_LED_Status()
{
	uint32_t current_tick = HAL_GetTick();
	uint16_t last_led_status = led_value;
	for(uint8_t i = 0; i < uart_channel_count; i++)
	{
		if(current_tick - last_rx_tick_ms[i] < 30)
		{
			led_value |= rx_led_pos[i];
		}
		else
		{
			led_value &= ~rx_led_pos[i];
		}
	}
	if(last_led_status != led_value)
	{
		set_LED_Driver_Value(led_value);
	}
}

uint8_t Sensor_Interface_Board::Received_Packet_Handler(Packet_Manager* const manager, const Protocol::Packet_Ptr* const request, Protocol::Command_Response_Packet &response)
{
	switch(request->header.command)
	{
		case Protocol::COMMAND::ANALOG_READ:
		{
			if(request->header.data_length != 1) return 1;
			uint8_t number = request->data[0];
			if(!number || (number > vi_channel_count)) return 1;

			response.header.command = Protocol::COMMAND::ANALOG_RESPONSE;
			uint8_t buffer[vi_channel_count] = {};
			((int16_t*)buffer)[0] = get_Voltage_mV(number - 1);
			((uint16_t*)buffer)[1] = get_Current_uA(number - 1);

			response.init_Data(buffer, sizeof(buffer));
			break;
		}

		case Protocol::COMMAND::SERIAL_SETUP:
		{
			if(request->header.data_length != sizeof(Protocol::Interface::Setting_Buffer)) return 1;
			auto param = (Protocol::Interface::Setting_Buffer*)request->data;

			printf("%d, %d, %ld, %d, %d, %d\n", (uint8_t)param->type, (uint8_t)param->channel, param->baudrate, (uint8_t)param->parity, (uint8_t)param->stop_bit, (uint8_t)param->flow_control);
			if(set_Interface_Setting(*param)) return 1;
			response.header.command = Protocol::COMMAND::REQUEST_SUCCESS;
			break;
		}

		case Protocol::COMMAND::SERIAL_SETUP_READ:
		{
			if(request->header.data_length != 1) return 1;
			uint8_t number = request->data[0];
			if(!number || (number > uart_channel_count)) return 1;

			response.header.command = Protocol::COMMAND::SERIAL_SETUP_READ_RESPONSE;
			response.init_Data((uint8_t*)&uart_setting[number - 1], sizeof(Protocol::Interface::Setting_Buffer));
			break;
		}

		case Protocol::COMMAND::SERIAL_WIRTE:
		{
			if(request->header.data_length < sizeof(Protocol::Interface::Data_Buffer)) return 1;
			auto buffer = (Protocol::Interface::Data_Buffer*)request->data;

			uint8_t target_index;
			switch(buffer->channel)
			{
				case Protocol::Interface::CHANNEL::CH1:
					target_index = 0;
					break;

				case Protocol::Interface::CHANNEL::CH2:
					target_index = 1;
					break;

				default:
					return 1;
			}
			auto target = get_Interface_Handle(target_index);
			auto tx_data_length = request->header.data_length - sizeof(Protocol::Interface::Data_Buffer);
			if(serial_interface[target_index] == INTERFACE_MODE::DDI) { set_DB9_12V_Output(true); }
			if(tx_data_length)
			{
				set_Interface_TX(target_index, true);
				HAL_UART_Transmit(target, buffer->data, tx_data_length, -1);
				set_Interface_TX(target_index, false);
			}

			auto &target_buffer = serial_interface_received_buffer[target_index];
			target_buffer.count = 0;
			target_buffer.enable = true;
			uint32_t receive_start_tick = HAL_GetTick();
			while((HAL_GetTick() - receive_start_tick) < buffer->receive_timeout_ms)
			{
				set_Watchdog_Timer_Reset();
				update_RX_LED_Status();
			}
			target_buffer.enable = false;
			if(serial_interface[target_index] == INTERFACE_MODE::DDI) { set_DB9_12V_Output(false); }
			led_value &= ~rx_led_pos[target_index];
			set_LED_Driver_Value(led_value);

			response.header.command = Protocol::COMMAND::SERIAL_WRITE_RESPONSE;
			response.init_Data(target_buffer.buffer, target_buffer.count);
			break;
		}

		case Protocol::COMMAND::ANALOG_READ_ALL:
		{
			if(request->header.data_length) return 1;

			response.header.command = Protocol::COMMAND::ANALOG_ALL_RESPONSE;
			uint16_t buffer[vi_channel_count * 2] = {}; //current, voltage
			for(uint8_t i = 0; i < vi_channel_count; i++)
			{
				((int16_t*)buffer)[i] = get_Voltage_mV(i);
				((uint16_t*)buffer)[vi_channel_count + i] = get_Current_uA(i);
			}

			response.init_Data((uint8_t*)buffer, sizeof(buffer));
			break;
		}

		default:
			return 2;
	}
	return 0;
}