/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "string.h"
#include <IR_Remote.h>
#include <stdlib.h>
//#include "spi.h"
//#include "gpio.h"
#include "max7219_digits.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
HAL_TIM_StateTypeDef pwm;
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi2;

TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
#define STR_LEN 300
#define SHORT_STR_LEN 50
uint16_t * rawData = NULL;
unsigned int rawLen = 0;
uint32_t protocolCode = 0;
int protocolBits = 0;
int protocol= UNUSED;
uint8_t recBuffer[STR_LEN];

char trans_str[STR_LEN];
remote_mode mode = NONE;
int should_receive = 1;
char delim[] = " ";
int len = 0;
int textIndex = -1;
int textDirection = 1;
int configMode = 0;
int bt_connected = 0;
int hasCalled = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM4_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_SPI2_Init(void);
static void MX_TIM3_Init(void);
/* USER CODE BEGIN PFP */
void reset();
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	uint8_t count = 0;
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_TIM4_Init();
  MX_USART1_UART_Init();
  MX_SPI2_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start_IT(&htim3);
  HAL_GPIO_WritePin(GPIOA, MAX7219_CS_Pin|TEST_Pin, GPIO_PIN_RESET);
  MAX7219_Init(&hspi2);
  HAL_Delay(1000);
  //MAX7219_ClearAll();
  //MAX7219_PutString(0, "YALOLI");
//  HAL_Delay(500);
//  HAL_UART_Transmit(&huart1, "AT\n", 3, 100);
//  HAL_UART_Receive(&huart1, recBuffer, SHORT_STR_LEN, 200);
//  HAL_UART_Transmit(&huart1, "AT+NAMERC-Clone \n", 17, 100);
//  HAL_UART_Receive(&huart1, recBuffer, SHORT_STR_LEN, 200);
//  HAL_Delay(500);
//  HAL_UART_Transmit(&huart1, "AT+PIN1214 \n", 12, 100);
//  HAL_UART_Receive(&huart1, recBuffer, SHORT_STR_LEN, 200);
  HAL_UART_Transmit(&huart1, "OK\n", 3, 10);
  HAL_UART_Receive_IT(&huart1, recBuffer, 4);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1)
	{
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		if (HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == GPIO_PIN_RESET) {
			HAL_Delay(500);
			configMode = !configMode;
		}

		if (configMode) {
			MAX7219_ClearAll();
			MAX7219_PutString(0, "BL-CONF");
			__HAL_UART_ENABLE(&huart2);
			HAL_UART_AbortReceive_IT(&huart1);
			HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
			HAL_UART_Receive(&huart2, recBuffer, SHORT_STR_LEN, 100);
			if (recBuffer[0] == 'A' && recBuffer[1] == 'T') {
				len = strlen(recBuffer);
				HAL_UART_Transmit(&huart1, recBuffer, len, 100);
				HAL_Delay(500);
				HAL_UART_Receive(&huart1, recBuffer, SHORT_STR_LEN, 200);
				len = strlen(recBuffer);
				HAL_UART_Transmit(&huart2, recBuffer, len, 100);
			}

			memset(recBuffer, 0, STR_LEN);
			continue;
		}
		__HAL_UART_DISABLE(&huart2);
		HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
		switch(mode) {
		case TRAN:
			MAX7219_ClearAll();
			MAX7219_PutString(0, "sEnd");
			bt_connected = 0;
			if (rawLen > 0 || protocolBits > 0) {
				HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
				send(rawData, rawLen, protocolCode, protocolBits, protocol);
				HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
			}
			reset();
			HAL_UART_Receive_IT(&huart1, recBuffer, 4);
			break;
		case DECODE:
			MAX7219_ClearAll();
			MAX7219_PutString(0, "dECodE");
			bt_connected = 0;
			if (protocol == UNKNOWN) {
				if (rawData) {
					free(rawData);
				}
				rawData = (uint16_t *) calloc(rawLen, sizeof(uint16_t));
				char *ptr = strtok(trans_str, delim);
				int i = 0;
				while(ptr != NULL && i < rawLen)
				{
					rawData[i] = atoi(ptr);
					ptr = strtok(NULL, delim);
					i++;
				}
			}
			else {
				char *ptr = strtok(trans_str, delim);
				protocolBits = atoi(ptr);
				ptr = strtok(NULL, delim);
				protocolCode = strtoul(ptr, NULL, 10);
			}

			mode = TRAN;
			break;
		case RECV:
			bt_connected = 0;
			if (in_enabled == 0) {
				my_enableIRIn();
				MAX7219_ClearAll();
				MAX7219_PutString(0, "ClONE");
			}

			if(my_decode(&results))
			{
				HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);

				memset(trans_str, 0, STR_LEN);
				sprintf(trans_str, "%d %d", results.decode_type, results.rawlen);
				if (results.rawlen > 0) {
					for (int i = 0; i < results.rawlen; ++i) {
						sprintf(trans_str, "%s %d", trans_str, results.rawbuf[i]);
					}
				}
				sprintf(trans_str, "%s %d", trans_str, results.bits);
				if (results.bits > 0) {
					sprintf(trans_str, "%s %u\n", trans_str, results.value);
				}
				len = strlen(trans_str);
				HAL_UART_Transmit_IT(&huart1, trans_str, len); // CHANGE THIS TO IT Mode After DEBUG

				HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
				reset();
				HAL_UART_Receive_IT(&huart1, recBuffer, 4);
			}
			break;
		default:
			if (should_receive) {
				HAL_UART_Receive_IT(&huart1, recBuffer, 4);
			}

			if (bt_connected) continue;
			if (textIndex > 1) {
				textIndex = 1;
				textDirection = 0;
			}
			if(textIndex < -1) {
				textIndex = -1;
				textDirection = 1;
			}
			MAX7219_ClearAll();
			MAX7219_PutString(textIndex, "RC-ClONE");
			if (textDirection) {
				textIndex++;
			}
			else {
				textIndex--;
			}
			HAL_Delay(100);
		}

	}
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage 
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 72;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 5624;
  htim3.Init.CounterMode = TIM_COUNTERMODE_DOWN;
  htim3.Init.Period = 63999;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 71;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 9;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */
  HAL_TIM_MspPostInit(&htim4);

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 9600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */
  __HAL_UART_ENABLE_IT(&huart1, UART_IT_TC); // TX Flag
  __HAL_UART_ENABLE_IT(&huart1, UART_IT_RXNE); // RX Flag
  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 9600;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, MAX7219_CS_Pin_Pin|LD2_Pin|TEST_Pin_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : MAX7219_CS_Pin_Pin LD2_Pin TEST_Pin_Pin */
  GPIO_InitStruct.Pin = MAX7219_CS_Pin_Pin|LD2_Pin|TEST_Pin_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : IR_In_Pin */
  GPIO_InitStruct.Pin = IR_In_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(IR_In_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

void reset() {
	mode = NONE;
	protocol = UNUSED;
	protocolBits = 0;
	protocolCode = 0;
	if (rawData) {
		free(rawData);
	}
	rawData = NULL;
	rawLen = 0;
	should_receive = 1;
	my_disable();
	textIndex = -1;
	textDirection = 1;
}

// Only use recBuffer here. Otherwise it wouldnt be complete.
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == USART1){
		int is_ok = 0;
		char message[5] = "OK\n";
		hasCalled = 1;
		if (mode == NONE) {
			if (protocol != UNUSED) {
				memset(trans_str, 0, STR_LEN);
				len = strlen(recBuffer);
				memcpy(trans_str, recBuffer, sizeof(char) * len);
				mode = DECODE;
				memset(recBuffer, 0, STR_LEN);
				//HAL_UART_Transmit(&huart1, message, 3, 10);
				return;
			}
		}

		if (strcmp(recBuffer, "RUOK") == 0) {
			// Are you ok message.
			protocol = UNUSED;
			is_ok = 1;
		}
		else if (strcmp(recBuffer, "RSET") == 0) {
			// Reset message. Reset everything.
			reset();
			is_ok = 1;

		}
		else if (strcmp(recBuffer, "RECV") == 0) {
			mode = RECV;
			is_ok = 1;
		}
		else if (recBuffer[0] == 'R' && recBuffer[1] == 'W') {
			// Raw command being sent. Prepare to receive raw timings array
			int len = atoi(&recBuffer[2]);
			if (len > 0 && len < 100) {
				protocol = UNKNOWN;
				rawLen = len;
			}
			should_receive = 0;
			HAL_UART_Receive_IT(&huart1, recBuffer, STR_LEN);
			is_ok = 1;
		}
		else if (recBuffer[0] == 'R') {
			// Raw command being sent. Prepare to receive raw timings array
			int prot = atoi(&recBuffer[1]);
			if (prot > 0 && prot < 100) {
				protocol = prot;
			}
			should_receive = 0;
			HAL_UART_Receive_IT(&huart1, recBuffer, SHORT_STR_LEN);
			is_ok = 1;
		}
		memset(recBuffer, 0, STR_LEN);
		if (is_ok == 0) {
			MAX7219_ClearAll();
			MAX7219_PutString(0, "Err");
			HAL_NVIC_SystemReset();
		}
		HAL_UART_Transmit(&huart1, message, 3, 10);
		if (should_receive) {
			if (!bt_connected) {
				MAX7219_ClearAll();
				MAX7219_PutString(0, "RC-ClONE");
			}
			bt_connected = 1;
			HAL_UART_Receive_IT(&huart1, recBuffer, 4);
		}
	}
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
	/* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
