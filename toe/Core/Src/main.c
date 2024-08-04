/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
//#include "cmox_crypto.h"
#include <string.h>
#include "mbedtls/aes.h"
#include "mbedtls/des.h"
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define UART2_TRIGGER	0
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

unsigned char Key[] = { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab,
		0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c };

unsigned char Plaintext[] = { 0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96,
		0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a };

unsigned char deskey[8] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77 };
unsigned char input[8] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77 };
unsigned char output[8];
unsigned char backward[8];

/* Computed data buffer */
unsigned char Computed_Ciphertext[sizeof(Plaintext)];
unsigned char Computed_Plaintext[sizeof(Plaintext)];

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart1_rx;
DMA_HandleTypeDef hdma_usart2_rx;

/* USER CODE BEGIN PV */
uint8_t rx1_buffer[BUF_SIZE];
uint8_t rx2_buffer[BUF_SIZE];
uint16_t rx1_cnt;
uint16_t rx2_cnt;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#ifdef __GNUC__

#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)

#else

#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)

#endif /* __GNUC__ */
PUTCHAR_PROTOTYPE {
	HAL_UART_Transmit(&huart2, (uint8_t*) &ch, 1, 0xFFFF);
	return ch;
}
int fputc(int ch, FILE*) {
	HAL_UART_Transmit(&huart2, (uint8_t*) &ch, 1, 0xFFFF);
	return 1;
}

//int m_printf(const char *fmt, ...)
//{
//	char dat[128];
//	int printed;
//	va_list args;
//	va_start(args, fmt);
//	printed = sprintf((char *)dat, fmt, args);
//	HAL_UART_Transmit_IT(&huart2, dat, printed);
//	va_end(args);
//	return printed;
//}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

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
  MX_DMA_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
	HAL_UARTEx_ReceiveToIdle_DMA(&huart1, rx1_buffer, BUF_SIZE);
	HAL_UARTEx_ReceiveToIdle_DMA(&huart2, rx2_buffer, BUF_SIZE);
	rx1_cnt = rx2_cnt = 0;

	mbedtls_aes_context aes_ctx;
	mbedtls_des_context ctx;
	int key_length = 128; // 密钥长度，AES支持128�??192�??256位密�??
	/*//AES加密示例//
	 mbedtls_aes_context aes_ctx;
	 mbedtls_aes_init(&aes_ctx);



	 mbedtls_aes_setkey_enc(&aes_ctx, Key, key_length); //设置密钥
	 mbedtls_aes_crypt_ecb(&aes_ctx, MBEDTLS_AES_ENCRYPT, Plaintext, Computed_Ciphertext); //加密
	 mbedtls_aes_free(&aes_ctx);
	 //AES加密示例结束//

	 //AES解密示例//
	 mbedtls_aes_init(&aes_ctx);
	 mbedtls_aes_setkey_dec(&aes_ctx, Key, key_length); //设置密钥
	 mbedtls_aes_crypt_ecb(&aes_ctx, MBEDTLS_AES_DECRYPT, Computed_Ciphertext, Computed_Plaintext); //加密
	 mbedtls_aes_free(&aes_ctx);
	 //AES解密示例结束//

	 //DES示例//
	 mbedtls_des_context ctx;
	 mbedtls_des_init(&ctx);
	 mbedtls_des_setkey_enc(&ctx, deskey);
	 mbedtls_des_crypt_ecb(&ctx, input, output);
	 mbedtls_des_setkey_dec(&ctx, deskey);
	 mbedtls_des_crypt_ecb(&ctx, output, backward);
	 mbedtls_des_free(&ctx);*/
	//DES示例结束//
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

	mbedtls_aes_init(&aes_ctx);
	mbedtls_des_init(&ctx);
	while (1) {

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		if (rx1_cnt) {

			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_RESET);
//			HAL_Delay(300);
			if (rx1_buffer[0] == 0x01 && rx1_buffer[1] == 0x00) { // Set AES encrypt Key
				memcpy(Key, rx1_buffer + 8, 16);
				mbedtls_aes_setkey_enc(&aes_ctx, Key, key_length);
				memset(rx1_buffer, 0x00, 6);
				HAL_UART_Transmit(&huart1, rx1_buffer, 6, 0xffff);
			} else if (rx1_buffer[0] == 0x01 && rx1_buffer[1] == 0x01) { // Set AES decrypt Key
				memcpy(Key, rx1_buffer + 8, 16);
				mbedtls_aes_setkey_dec(&aes_ctx, Key, key_length);
				memset(rx1_buffer, 0x00, 6);
				HAL_UART_Transmit(&huart1, rx1_buffer, 6, 0xffff);
			} else if (rx1_buffer[0] == 0x01 && rx1_buffer[1] == 0x02) { // AES Encryption
#if UART2_TRIGGER
				HAL_UART_Transmit(&huart2, rx2_buffer, 1, 0xffff);
#endif
				mbedtls_aes_crypt_ecb(&aes_ctx, MBEDTLS_AES_ENCRYPT,
						rx1_buffer + 8, rx1_buffer + 6);
#if UART2_TRIGGER
				HAL_UART_Transmit(&huart2, rx2_buffer, 1, 0xffff);
#endif
				memset(rx1_buffer, 0x00, 6);
				rx1_buffer[5] = 0x10;
				HAL_UART_Transmit(&huart1, rx1_buffer, 6 + 16, 0xffff);
//				memcpy(Plaintext, rx1_buffer + 8, 16);
//				HAL_UART_Transmit(&huart2, rx2_buffer, 1, 0xffff);
//				mbedtls_aes_crypt_ecb(&aes_ctx, MBEDTLS_AES_ENCRYPT,
//						rx1_buffer + 8, Computed_Ciphertext);
//				HAL_UART_Transmit(&huart2, rx2_buffer, 1, 0xffff);
//				memcpy(rx1_buffer + 6, Computed_Ciphertext, 16);
//				memset(rx1_buffer, 0x00, 6);
//				rx1_buffer[5] = 0x10;
//				HAL_UART_Transmit(&huart1, rx1_buffer, 6 + 16, 0xffff);
			} else if (rx1_buffer[0] == 0x01 && rx1_buffer[1] == 0x03) {  // AES Decryption
//				memcpy(Plaintext, rx1_buffer + 8, 16);
#if UART2_TRIGGER
				HAL_UART_Transmit(&huart2, rx2_buffer, 1, 0xffff);
#endif
				mbedtls_aes_crypt_ecb(&aes_ctx, MBEDTLS_AES_DECRYPT, rx1_buffer + 8,
						rx1_buffer + 6);
#if UART2_TRIGGER
				HAL_UART_Transmit(&huart2, rx2_buffer, 1, 0xffff);
#endif
//				memcpy(rx1_buffer + 6, Computed_Ciphertext, 16);
				memset(rx1_buffer, 0x00, 6);
				rx1_buffer[5] = 0x10;
				HAL_UART_Transmit(&huart1, rx1_buffer, 6 + 16, 0xffff);
			} else if (rx1_buffer[0] == 0x02 && rx1_buffer[1] == 0x00) {  // Set DES encryption key
				memcpy(deskey, rx1_buffer + 8, 8);
				mbedtls_des_setkey_enc(&ctx, deskey);
				memset(rx1_buffer, 0x00, 6);
				HAL_UART_Transmit(&huart1, rx1_buffer, 6, 0xffff);
			} else if (rx1_buffer[0] == 0x02 && rx1_buffer[1] == 0x01) {  // Set DES decryption key
				memcpy(deskey, rx1_buffer + 8, 8);
				mbedtls_des_setkey_dec(&ctx, deskey);
				memset(rx1_buffer, 0x00, 6);
				HAL_UART_Transmit(&huart1, rx1_buffer, 6, 0xffff);
			} else if (rx1_buffer[0] == 0x02 && rx1_buffer[1] == 0x02) {  // DES Encryption
//				memcpy(input, rx1_buffer + 8, 8);
#if UART2_TRIGGER
				HAL_UART_Transmit(&huart2, rx2_buffer, 1, 0xffff);
#endif
				mbedtls_des_crypt_ecb(&ctx, rx1_buffer + 8, rx1_buffer + 6);
#if UART2_TRIGGER
				HAL_UART_Transmit(&huart2, rx2_buffer, 1, 0xffff);
#endif
//				memcpy(rx1_buffer + 6, out?put, 8);
				memset(rx1_buffer, 0x00, 6);
				rx1_buffer[5] = 0x8;
				HAL_UART_Transmit(&huart1, rx1_buffer, 6 + 8, 0xffff);
			} else if (rx1_buffer[0] == 0x02 && rx1_buffer[1] == 0x03) {
//				memcpy(input, rx1_buffer + 8, 8);
#if UART2_TRIGGER
				HAL_UART_Transmit(&huart2, rx2_buffer, 1, 0xffff);
#endif
				mbedtls_des_crypt_ecb(&ctx, rx1_buffer + 8, rx1_buffer + 6);
#if UART2_TRIGGER
				HAL_UART_Transmit(&huart2, rx2_buffer, 1, 0xffff);
#endif
//				memcpy(rx1_buffer + 6, output, 8);
				memset(rx1_buffer, 0x00, 6);
				rx1_buffer[5] = 0x8;
				HAL_UART_Transmit(&huart1, rx1_buffer, 6 + 8, 0xffff);
			}

			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_SET);
			//UART1收到了rx1_cnt个字节， 在rx1_buffer[]�?

			memset(rx1_buffer, 0, rx1_cnt);
			rx1_cnt = 0;
		}

		if (rx2_cnt) {
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_RESET);
			HAL_Delay(300);
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_SET);
			//UART2收到了rx1_cnt个字节， 在rx2_buffer[]�?
			HAL_UART_Transmit(&huart2, rx2_buffer, rx2_cnt, 0xffff);
			memset(rx2_buffer, 0, rx2_cnt);
			rx2_cnt = 0;
		}

	}
	mbedtls_aes_free(&aes_ctx);
	mbedtls_des_free(&ctx);
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
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
  huart1.Init.BaudRate = 115200;
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
  huart2.Init.BaudRate = 115200;
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
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel5_IRQn);
  /* DMA1_Channel6_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel6_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel6_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_RESET);

  /*Configure GPIO pin : PB9 */
  GPIO_InitStruct.Pin = GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
	}
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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
