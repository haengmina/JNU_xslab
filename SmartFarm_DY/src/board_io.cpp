/**
 * @file board_io.cpp
 * @author Kwon Jinwoo (jinwoo@xslab.co.kr)
 * @brief SmartFarmST32 Board IO
 * @date 2025-08-18
 */

#include <board_io.h>

#define _BV(x) (1 << x)

static DAC_HandleTypeDef dac1 = {};
static I2C_HandleTypeDef i2c[2] = {};
static UART_HandleTypeDef uart[4] = {};
static IWDG_HandleTypeDef iwdg = {};
static MXC6655 accel(get_I2C_Handle(0), 0x2A);

extern "C" void Error_Handler(void)
{
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
}

void init_Clock()
{
	RCC_OscInitTypeDef RCC_OscInitStruct = {};
	RCC_ClkInitTypeDef RCC_ClkInitStruct = {};

	/** Configure the main internal regulator output voltage
	 */
	HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

	/** Configure LSE Drive Capability
	 */
	HAL_PWR_EnableBkUpAccess();
	__HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE
								|RCC_OSCILLATORTYPE_LSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
	RCC_OscInitStruct.LSEState = RCC_LSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV2;
	RCC_OscInitStruct.PLL.PLLN = 16;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
	RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV3;
	if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
								|RCC_CLOCKTYPE_PCLK1;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
	{
		Error_Handler();
	}
	HAL_RCC_MCOConfig(RCC_MCO2, RCC_MCO2SOURCE_HSE, RCC_MCO2DIV_16);

	//ALL GPIO Clock Enable
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOE_CLK_ENABLE();
	__HAL_RCC_GPIOF_CLK_ENABLE();
}

void init_GPIO()
{
	GPIO_Init_Info target_gpio[] = {
		{ GPIOA, {.Pin = _BV(15), GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, .Alternate = GPIO_AF3_MCO2 }}, //PA15_TFDRIVER_CLK (MCO2 Output)
		{ GPIOA, {.Pin = (_BV(0) | _BV(1) | _BV(2)), GPIO_MODE_INPUT, GPIO_NOPULL }}, //Interface Board Version
		{ GPIOF, {.Pin = _BV(3), GPIO_MODE_OUTPUT_PP, GPIO_PULLUP }}, //PF3_BUTTON1
		{ GPIOC, {.Pin = _BV(0), GPIO_MODE_OUTPUT_PP, GPIO_NOPULL }}, //PC0_RS485_POWER_EN
		{ GPIOF, {.Pin = _BV(5), GPIO_MODE_OUTPUT_PP, GPIO_NOPULL }}, //PF5_RS485_DE_RE#
		{ GPIOE, {.Pin = _BV(3), GPIO_MODE_OUTPUT_PP, GPIO_NOPULL }}, //PE3_LED_INPUT_POWER (Active Low)
		{ GPIOE, {.Pin = _BV(4), GPIO_MODE_OUTPUT_PP, GPIO_NOPULL }}, //PE4_LED_+24V (Active Low)
		{ GPIOE, {.Pin = _BV(5), GPIO_MODE_OUTPUT_PP, GPIO_NOPULL }}, //PE5_LED_+3.3V (Active Low)
		{ GPIOE, {.Pin = _BV(6), GPIO_MODE_OUTPUT_PP, GPIO_NOPULL }}, //PE6_LED_GRN (Active Low), For Node Selected Status
		{ GPIOC, {.Pin = _BV(12), GPIO_MODE_OUTPUT_PP, GPIO_NOPULL }}, //PC12_LED_BLE (Active Low)
		{ GPIOC, {.Pin = _BV(13), GPIO_MODE_OUTPUT_PP, GPIO_NOPULL }}, //PC13_LED_YEL (Active Low)
		{ GPIOC, {.Pin = _BV(1), GPIO_MODE_OUTPUT_PP, GPIO_NOPULL }}, //PC1_RS485_TX/RX_LED_ON
		{ GPIOC, {.Pin = _BV(2), GPIO_MODE_OUTPUT_PP, GPIO_NOPULL }}, //PC2_FAULT_SIGNAL_OFF
		{ GPIOC, {.Pin = _BV(11), GPIO_MODE_INPUT, GPIO_NOPULL }}, //PC11_PPS_GPS
		{ GPIOD, {.Pin = _BV(3), GPIO_MODE_OUTPUT_PP, GPIO_NOPULL }}, //PD3_RESET#_GPS
		{ GPIOF, {.Pin = _BV(4), GPIO_MODE_OUTPUT_PP, GPIO_NOPULL }}, //PF4_REST#_LED_DRIVER
		{ GPIOD, {.Pin = _BV(4), GPIO_MODE_INPUT, GPIO_NOPULL }}, //PD4_+24V_PG
		{ GPIOC, {.Pin = _BV(3), GPIO_MODE_OUTPUT_PP, GPIO_NOPULL }}, //PC3_TERMIANL_SHDN, Active High
		{ GPIOA, {.Pin = _BV(3), GPIO_MODE_OUTPUT_PP, GPIO_NOPULL }}, //PA3_RJ45_INPUT_SHDN, Active High
		{ GPIOB, {.Pin = _BV(15), GPIO_MODE_OUTPUT_PP, GPIO_NOPULL }}, //PB15_RJ45_OUTPUT_SHDN, Active High
		{ GPIOB, {.Pin = _BV(12), GPIO_MODE_INPUT, GPIO_NOPULL }}, //PB12_ADC11_TERMIANL_IMON
		{ GPIOB, {.Pin = _BV(11), GPIO_MODE_INPUT, GPIO_NOPULL }}, //PB11_ADC15_RJ45_INPUT_IMON
		{ GPIOB, {.Pin = _BV(10), GPIO_MODE_INPUT, GPIO_NOPULL }}, //PB10_ADC16_RJ45_OUTPUT_IMON
	};

	for(auto &target : target_gpio)
	{
		HAL_GPIO_Init(target.gpio, &target.init);
	}

	GPIOF->ODR |= _BV(4); //LED Driver Reset OFF
	GPIOC->ODR |= _BV(0); //RS485 Power Enable
	set_Status_LED(STATUS_LED_TYPE::GREEN, false);
	set_Status_LED(STATUS_LED_TYPE::BLUE, false);
	set_Status_LED(STATUS_LED_TYPE::YELLOW, false);
	set_Buzzer(false);
}

void init_DAC()
{
	DAC_ChannelConfTypeDef sConfig = {};

	dac1.Instance = DAC1;
	if(HAL_DAC_Init(&dac1) != HAL_OK)
	{
		Error_Handler();
	}

	sConfig.DAC_SampleAndHold = DAC_SAMPLEANDHOLD_DISABLE;
	sConfig.DAC_Trigger = DAC_TRIGGER_NONE;
	sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
	sConfig.DAC_ConnectOnChipPeripheral = DAC_CHIPCONNECT_DISABLE;
	sConfig.DAC_UserTrimming = DAC_TRIMMING_FACTORY;
	if(HAL_DAC_ConfigChannel(&dac1, &sConfig, DAC_CHANNEL_1) != HAL_OK)
	{
		Error_Handler();
	}

	HAL_DAC_Start(&dac1, DAC_CHANNEL_1);
}

void init_I2C()
{
	i2c[0].Instance = I2C1;
	i2c[1].Instance = I2C2;
	for(auto &target : i2c)
	{
		target.Init.Timing = 0x2010091A;
		target.Init.OwnAddress1 = 0;
		target.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
		target.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
		target.Init.OwnAddress2 = 0;
		target.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
		target.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
		target.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
		if (HAL_I2C_Init(&target) != HAL_OK)
		{
			Error_Handler();
		}

		/** Configure Analogue filter
		 */
		if(HAL_I2CEx_ConfigAnalogFilter(&target, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
		{
			Error_Handler();
		}

		/** Configure Digital filter
		 */
		if(HAL_I2CEx_ConfigDigitalFilter(&target, 0) != HAL_OK)
		{
			Error_Handler();
		}
	}
}

void init_UART()
{
	uart[0].Instance = USART1;
	uart[0].Init.BaudRate = 19200;
	uart[0].Init.HwFlowCtl = UART_HWCONTROL_NONE;
	uart[0].AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;

	uart[1].Instance = USART2;
	uart[1].Init.BaudRate = 9600;
	uart[1].Init.HwFlowCtl = UART_HWCONTROL_NONE;
	uart[1].AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;

	uart[2].Instance = USART3;
	uart[2].Init.BaudRate = 9600;
	uart[2].Init.HwFlowCtl = UART_HWCONTROL_NONE;
	// uart[2].AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_TXINVERT_INIT | UART_ADVFEATURE_RXINVERT_INIT;
	uart[2].AdvancedInit.RxPinLevelInvert = UART_ADVFEATURE_RXINV_ENABLE;
	uart[2].AdvancedInit.TxPinLevelInvert = UART_ADVFEATURE_TXINV_ENABLE;
	// uart[2].Init.HwFlowCtl = UART_HWCONTROL_RTS_CTS;

	uart[3].Instance = USART5;
	uart[3].Init.BaudRate = 9600;
	uart[3].Init.HwFlowCtl = UART_HWCONTROL_NONE;
	// uart[3].AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_TXINVERT_INIT | UART_ADVFEATURE_RXINVERT_INIT;
	uart[3].AdvancedInit.RxPinLevelInvert = UART_ADVFEATURE_RXINV_ENABLE;
	uart[3].AdvancedInit.TxPinLevelInvert = UART_ADVFEATURE_TXINV_ENABLE;
	// uart[3].Init.HwFlowCtl = UART_HWCONTROL_RTS_CTS;

	for(auto &target : uart)
	{
		target.Init.WordLength = UART_WORDLENGTH_8B;
		target.Init.StopBits = UART_STOPBITS_1;
		target.Init.Parity = UART_PARITY_NONE;
		target.Init.Mode = UART_MODE_TX_RX;
		target.Init.OverSampling = UART_OVERSAMPLING_16;
		target.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
		target.Init.ClockPrescaler = UART_PRESCALER_DIV1;
		if(HAL_UART_Init(&target) != HAL_OK)
		{
			Error_Handler();
		}
		if(HAL_UARTEx_SetTxFifoThreshold(&target, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
		{
			Error_Handler();
		}
		if(HAL_UARTEx_SetRxFifoThreshold(&target, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
		{
			Error_Handler();
		}
		if(HAL_UARTEx_DisableFifoMode(&target) != HAL_OK)
		{
			Error_Handler();
		}
	}

    HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
    HAL_NVIC_SetPriority(USART2_LPUART2_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART2_LPUART2_IRQn);
    HAL_NVIC_SetPriority(USART3_4_5_6_LPUART1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART3_4_5_6_LPUART1_IRQn);
}

void init_Watchdog()
{
	iwdg.Instance = IWDG;
	iwdg.Init.Prescaler = IWDG_PRESCALER_32;
	iwdg.Init.Window = 4095;
	iwdg.Init.Reload = 4095;
	if(HAL_IWDG_Init(&iwdg) != HAL_OK)
	{
		Error_Handler();
	}
}

static uint8_t uart_rx_data[4] = {};
void set_UART_Receive_Start(uint8_t index)
{
	if(index >= sizeof(uart_rx_data)) return;

	volatile auto ret = HAL_UART_Receive_IT(get_UART_Handle(index), &uart_rx_data[index], 1);
	(void)ret;
}

void set_Watchdog_Timer_Reset()
{
	if(!iwdg.Instance) return;

	HAL_IWDG_Refresh(&iwdg);
}

I2C_HandleTypeDef* get_I2C_Handle(uint8_t i2c_index)
{
	if(i2c_index > 1) return nullptr;

	return &i2c[i2c_index];
}

UART_HandleTypeDef* get_UART_Handle(uint8_t uart_index)
{
	if(uart_index > 3) return nullptr;

	return &uart[uart_index];
}

MXC6655* get_Accel_Object()
{
	return &accel;
}

INTERFACE_BOARD_TYPE get_Interface_Board_Type()
{
	if(GPIOA->IDR & _BV(2))
	{
		return INTERFACE_BOARD_TYPE::CONTROL;
	}
	else
	{
		return INTERFACE_BOARD_TYPE::SENSOR;
	}
}

uint8_t get_Interface_Board_Version()
{
	uint8_t version = 0x00;

	if(GPIOA->IDR & _BV(0))
	{
		version |= _BV(0);
	}

	if(GPIOA->IDR & _BV(1))
	{
		version |= _BV(1);
	}

	return version;
}

bool get_P24V_PG_Status()
{
	return (GPIOD->IDR & _BV(4));
}

uint16_t get_LED_Driver_Value()
{
	uint16_t value = 0x0000;
	auto i2c_handle = get_I2C_Handle(0);
	uint8_t i2c_buffer[4][2] = {{ 0x14, 0x00 }, { 0x15, 0x00 }, { 0x16, 0x00 }, { 0x17, 0x00 }};

	for(uint8_t index = 0; index < 4; index++)
	{
		auto &target = i2c_buffer[index];

		HAL_I2C_Master_Transmit(i2c_handle, 0xC0, target, 1, 100);
		HAL_I2C_Master_Receive(i2c_handle, 0xC0, &target[1], 1, 100);
		for(uint8_t i = 0; i < 4; i++)
		{
			if(target[1] & (0x01 << (i * 2)))
			{
				value |= _BV((i + index * 4));
			}
		}
	}

	return value;
}

uint8_t get_UART_Received_Byte(uint8_t index)
{
	if(index >= sizeof(uart_rx_data)) return 0x00;

	return uart_rx_data[index];
}

void set_Status_LED(STATUS_LED_TYPE type, bool enable)
{
	GPIO_TypeDef *target_gpio = nullptr;
	uint32_t target_pin = 0x00;
	
	switch(type)
	{
		case STATUS_LED_TYPE::INPUT_POWER:
			target_gpio = GPIOE;
			target_pin = _BV(3);
			break;

		case STATUS_LED_TYPE::V24_STATUS:
			target_gpio = GPIOE;
			target_pin = _BV(4);
			break;

		case STATUS_LED_TYPE::V3P3_STATUS:
			target_gpio = GPIOE;
			target_pin = _BV(5);
			break;

		case STATUS_LED_TYPE::GREEN:
			target_gpio = GPIOE;
			target_pin = _BV(6);
			break;

		case STATUS_LED_TYPE::BLUE:
			target_gpio = GPIOC;
			target_pin = _BV(12);
			break;

		case STATUS_LED_TYPE::YELLOW:
			target_gpio = GPIOC;
			target_pin = _BV(13);
			break;

		case STATUS_LED_TYPE::RS485_ACT:
			target_gpio = GPIOC;
			target_pin = _BV(1);
			enable = !enable; //Active High
			break;

		default:
			break;
	}

	if(!target_gpio) return;

	if(enable)
	{
		target_gpio->ODR &= ~target_pin;
	}
	else
	{
		target_gpio->ODR |= target_pin;
	}
}

void set_LED_Driver_Enable(bool enable)
{
	uint8_t i2c_buffer[] =  { 0x00, (uint8_t)((enable) ? 0x00 : 0x10) }; //MODE1, OSC Bit Set
	HAL_I2C_Master_Transmit(get_I2C_Handle(0), 0xC0, i2c_buffer, sizeof(i2c_buffer), 100);
}

void set_LED_Driver_Value(uint16_t value)
{
	auto i2c_handle = get_I2C_Handle(0);
	uint8_t i2c_buffer[4][2] = {{ 0x14, 0x00 }, { 0x15, 0x00 }, { 0x16, 0x00 }, { 0x17, 0x00 }};

	for(uint8_t index = 0; index < 4; index++)
	{
		auto &target = i2c_buffer[index];

		for(uint8_t i = 0; i < 4; i++)
		{
			if(!(value & _BV((i + index * 4)))) { continue; }

			target[1] |= (0x01 << (i * 2));
		}
		HAL_I2C_Master_Transmit(i2c_handle, 0xC0, target, sizeof(target), 100);
	}
}

void set_Buzzer(bool enable)
{
	if(enable)
	{
		GPIOC->ODR &= ~_BV(2);
	}
	else
	{
		GPIOC->ODR |= _BV(2);
	}
	set_Status_LED(STATUS_LED_TYPE::YELLOW, enable);
}

void set_DAC_Value(uint16_t value)
{
	if(value > 4095) return;

	HAL_DAC_SetValue(&dac1, DAC_CHANNEL_1, DAC_ALIGN_12B_R, value);
}

void set_DAC_Voltage(float value)
{
	if(value > 3.3) return;

	uint16_t reg_value = (value / 3.3) * 4095;

	HAL_DAC_SetValue(&dac1, DAC_CHANNEL_1, DAC_ALIGN_12B_R, reg_value);
}

void set_GPS_Reset(bool enable)
{
	if(enable)
	{
		GPIOD->ODR &= ~_BV(3);
	}
	else
	{
		GPIOD->ODR |= _BV(3);
	}
}

void set_Isolated_RS485_TX_Enable(bool enable)
{
	if(enable)
	{
		GPIOF->ODR |= _BV(5);
	}
	else
	{
		GPIOF->ODR &= ~_BV(5);
	}
}

void set_Accel_Value_Test()
{
	MXC6655::Value value = {};
	if(accel.get_Value(&value)) return;

	for(uint8_t i = 0; i < 3; i++)
	{
		switch(i)
		{
			case 0:
				printf("XOUT: %.3f", value.x_g);
				break;
			case 1:
				printf("YOUT: %.3f", value.y_g);
				break;
			case 2:
				printf("ZOUT: %.3f", value.z_g);
				break;
		}
	}
	printf("\n");
}

void update_Status_LED()
{
	static bool led_status = false;
	static uint32_t last_tick = HAL_GetTick();
	uint32_t current_tick = HAL_GetTick();

	set_Status_LED(STATUS_LED_TYPE::V24_STATUS, get_P24V_PG_Status());
	bool flag = ((current_tick - last_tick) < 200);
	if(!flag)
	{
		led_status ^= true;
		last_tick = current_tick;
		set_Status_LED(STATUS_LED_TYPE::GREEN, led_status);
	}
}

// IRQ Handler //
extern "C" void USART1_IRQHandler(void)
{
	HAL_UART_IRQHandler(&uart[0]);
}

extern "C" void USART2_LPUART2_IRQHandler(void)
{
	HAL_UART_IRQHandler(&uart[1]);
}

extern "C" void USART3_4_5_6_LPUART1_IRQHandler()
{
	HAL_UART_IRQHandler(&uart[2]);
	HAL_UART_IRQHandler(&uart[3]);
}