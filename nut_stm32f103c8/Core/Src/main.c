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

/**
 * Copyright CrackNuts
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "can.h"
#include "i2c.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"

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
#define BUF_SIZE  1024

#define CMD_LEN		(2+2+4)
#define STA_LEN		(2+4)

#define LED_ON HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_RESET)
#define LED_OFF HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_SET)

#define USER_IO_SET HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_SET)
#define USER_IO_CLR HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_RESET)

#define NUT_IO1_SET HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET)
#define NUT_IO1_CLR HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET)

#define NUT_IO2_SET HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET)
#define NUT_IO2_CLR HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET)

#define NUT_IO3_SET HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_SET)
#define NUT_IO4_CLR HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_RESET)

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

// todo 优化变量
unsigned char Key[] = { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab,
		0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c };

unsigned char Plaintext[] = { 0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96,
		0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a };

unsigned char deskey[8] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77 };
unsigned char input[8] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77 };
unsigned char output[8];
unsigned char backward[8];

/* Computed data rx_buffer */
unsigned char Computed_Ciphertext[sizeof(Plaintext)];
unsigned char Computed_Plaintext[sizeof(Plaintext)];

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint8_t flag_uart;
uint8_t flag_spi;
uint8_t flag_i2c;
uint8_t flag_can;
uint8_t flag_cmd_payload; // 0: cmd; 1: payload;

uint32_t payload_len;

uint8_t can_rx_buffer[8];
uint8_t rx_buffer[BUF_SIZE];
uint8_t tx_buffer[BUF_SIZE];

uint16_t status;

mbedtls_aes_context aes_ctx;
mbedtls_des_context des_ctx;
int key_length = 128; // 密钥长度，AES支持128�?????????????????????????192�?????????????????????????256位密�?????????????????????????

//uint8_t CANSendBuff[8]={0,1,2,3,4,5,6,7};
uint32_t CAN_ID=0x400;
uint32_t CAN_TARGET_ID=0x400;
static CAN_RxHeaderTypeDef RxMessage;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void process_cmd(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart->Instance == USART1) {
		if (flag_cmd_payload == 0) { // command phase
			payload_len = (rx_buffer[6] << 8) + rx_buffer[7]; // compute payload length
			flag_cmd_payload = 1; // wait for payload
			HAL_UART_Receive_IT(&huart1, rx_buffer + CMD_LEN, payload_len); // receive payload
//			HAL_UART_Receive_IT(&huart1, rx_buffer + CMD_LEN, CMD_LEN); // receive payload
		} else { // payload phase
			flag_uart = 1;
		}
	}
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi){
	if (hspi->Instance == SPI1) {
		HAL_SPI_Receive_IT(&hspi1, rx_buffer, CMD_LEN);
	}
}

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi) {
	if (hspi->Instance == SPI1) {
		if (flag_cmd_payload == 0) { // command phase
			payload_len = (rx_buffer[6] << 8) + rx_buffer[7]; // compute payload length
			flag_cmd_payload = 1; // wait for payload
			HAL_SPI_Receive_IT(&hspi1, rx_buffer + CMD_LEN, payload_len);
//			HAL_SPI_Receive(&hspi1, rx_buffer + CMD_LEN, CMD_LEN, HAL_MAX_DELAY);
//			flag_spi = 1;
//			HAL_SPI_Receive_IT(&hspi1, rx_buffer + CMD_LEN, payload_len); // receive payload
		} else { // payload phase
			flag_spi = 1;

		}
	}
}

void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi){
	if (hspi->Instance == SPI1) {
		;
	}

}

//void HAL_I2C_AddrCallback(I2C_HandleTypeDef *hi2c, uint8_t TransferDirection, uint16_t AddrMatchCode)
//{
//    if (TransferDirection == I2C_DIRECTION_RECEIVE)
//    {
//        // 主设备发送数据到Slave
//        HAL_I2C_Slave_Receive_IT(&hi2c1, rxBuffer, RX_BUFFER_SIZE);
//    }
//    else if (TransferDirection == I2C_DIRECTION_TRANSMIT)
//    {
//        // 主设备从Slave读取数据
//        HAL_I2C_Slave_Transmit_IT(&hi2c1, txBuffer, TX_BUFFER_SIZE);
//    }
//}

void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    // 处理接收到的数据
//    for (uint8_t i = 0; i < RX_BUFFER_SIZE; i++)
//    {
//        // 处理接收到的数据
//        // 例如：将数据存储到某个变量或执行某些操作
//    }
}

void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    // 处理发�?�完�?????
}

void CAN_SendMsg(uint16_t msgID, uint8_t *Data,uint8_t data_loc,uint8_t data_len)
{
    CAN_TxHeaderTypeDef   TxHeader;
	TxHeader.StdId = msgID;			//stdID
	TxHeader.RTR = CAN_RTR_DATA;		//Êý¾ÝÖ¡,CAN_RTR_DATA
	TxHeader.IDE = CAN_ID_STD;		//±ê×¼¸ñÊ½
	TxHeader.DLC =8;   				//Êý¾Ý³¤¶È
	TxHeader.TransmitGlobalTime = DISABLE;
	uint8_t  TxData[8];		//×î¶à8¸ö×Ö½Ú
	//if(data_loc==0) TxData[0] = 0;
	//else TxData[0] = 0xff;
	TxData[0] = data_loc;
	TxData[1] = *(Data+data_loc*7+0);
	TxData[2] = *(Data+data_loc*7+1);
	TxData[3] = *(Data+data_loc*7+2);
	TxData[4] = *(Data+data_loc*7+3);
	TxData[5] = *(Data+data_loc*7+4);
	TxData[6] = *(Data+data_loc*7+5);
	TxData[7] = *(Data+data_loc*7+6);
	while(HAL_CAN_GetTxMailboxesFreeLevel(&hcan) < 1) {
	} //µÈ´ýÓÐ¿ÉÓÃµÄ·¢ËÍÓÊÏä
	uint32_t TxMailbox;		//ÁÙÊ±±äÁ¿, ÓÃÓÚ·µ»ØÊ¹ÓÃµÄÓÊÏä±àºÅ
	/*  ·¢ËÍµ½ÓÊÏä£¬ÓÉCANÄ£¿é¸ºÔð·¢ËÍµ½CAN×ÜÏß   */
	if(HAL_CAN_AddTxMessage(&hcan, &TxHeader, TxData, &TxMailbox) != HAL_OK)
    {
         printf("can_err\r\n");
    }
}

void CAN_Sendframe(uint16_t CAN_TARGET_ID, uint8_t *Data,uint8_t can_frame_num)
{
	uint8_t i;
	for( i =0 ;i<can_frame_num;i++)
	{
	  CAN_SendMsg(CAN_TARGET_ID,Data,i,8);
      //HAL_Delay(200);
	}
}


uint8_t bsp_can1_filter_config(void)
{
    CAN_FilterTypeDef filter = {0};
    filter.FilterActivation = ENABLE;
    filter.FilterMode = CAN_FILTERMODE_IDMASK;
    filter.FilterScale = CAN_FILTERSCALE_32BIT;
    filter.FilterBank = 0;
    filter.FilterFIFOAssignment = CAN_FILTER_FIFO0;
    filter.FilterIdLow = 0;
    filter.FilterIdHigh = 0;
    filter.FilterMaskIdLow = 0;
    filter.FilterMaskIdHigh = 0;
    HAL_CAN_ConfigFilter(&hcan, &filter);

}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{

		uint8_t data[8];
		HAL_StatusTypeDef status;

		status = HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxMessage, data);
		if (HAL_OK == status){
			if(CAN_ID=RxMessage.StdId)
			{memcpy(can_rx_buffer,data,8);

			flag_can=1;}
		}

}

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
  MX_USART1_UART_Init();
  MX_CAN_Init();
  MX_I2C1_Init();
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */
	flag_uart = 0;
	flag_spi = 0;
	flag_i2c = 0;
	flag_can = 0;
	flag_cmd_payload = 0;

	HAL_UART_Receive_IT(&huart1, rx_buffer, CMD_LEN);
	HAL_SPI_Receive_IT(&hspi1, rx_buffer, CMD_LEN);


    bsp_can1_filter_config();//ÂË²¨Æ÷ÉèÖÃ£¬½ÓÊÕËùÓÐÖ¡
    HAL_CAN_Start(&hcan);//Æô¶¯CAN
	HAL_CAN_ActivateNotification(&hcan,CAN_IT_RX_FIFO0_MSG_PENDING);//ÖÐ¶Ï½ÓÊÕÊ¹ÄÜ
	for(uint8_t i =0 ;i<16;i++)
		tx_buffer[i] = i;

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

	mbedtls_aes_init(&aes_ctx);
	mbedtls_des_init(&des_ctx);




	while (1) {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */




		if (flag_uart || flag_spi || flag_i2c || flag_can)
		{
			SysTick->CTRL &= ~SysTick_CTRL_TICKINT_Msk;

			LED_ON;
			USER_IO_SET;

			process_cmd();

			USER_IO_CLR;
			LED_OFF;
			SysTick->CTRL &= SysTick_CTRL_TICKINT_Msk;

			tx_buffer[0] = (status & 0xff00) >> 8;
			tx_buffer[1] = status & 0xff;
			tx_buffer[2] = 0;
			tx_buffer[3] = 0;
			tx_buffer[4] = (payload_len & 0xff00) >> 8;

			tx_buffer[5] = payload_len & 0xff;

			for(uint8_t i =6 ;i<payload_len+6;i++)//make fake txbuffer data for test
			{
				tx_buffer[i] = i;
			}

			if (flag_uart) {
				HAL_UART_Transmit(&huart1, tx_buffer, 6 + payload_len, 0xffff);
				flag_uart = 0;
				HAL_UART_Receive_IT(&huart1, rx_buffer, CMD_LEN);
			} else if (flag_spi) {
				HAL_SPI_Transmit_IT(&hspi1, tx_buffer, 6 + payload_len);
				flag_spi = 0;
			}else if (flag_can)
			{
				uint32_t rx_data_cnt;
				uint32_t rx_payload;
				uint8_t rx_DLC;
				rx_DLC =RxMessage.DLC;

				if(can_rx_buffer[0]==0)//frame_start
				{
					rx_payload = can_rx_buffer[5]<<8+can_rx_buffer[6];
					rx_data_cnt =0;
				}


				for(uint8_t i =0 ;i<7;i++)//make fake txbuffer data for test
				{
					rx_buffer[rx_data_cnt] = can_rx_buffer[i+1];
					rx_data_cnt = rx_data_cnt+1;
				}

				if(rx_data_cnt>= rx_payload +6-1) {  //frame end
				  //generate can_frame_num by counting every 8bytes;for a CAN_tx frame is 8bytes
				  uint8_t can_frame_num = (6 + payload_len)/7;
				  //pluse 1 frame_num,if there areis extra bytes exceed 8bytes
				  if((6 + payload_len)%7 !=0) can_frame_num =can_frame_num+1;
     				CAN_Sendframe(CAN_TARGET_ID, tx_buffer, can_frame_num);
			      }

				flag_can = 0;
			}


			flag_cmd_payload = 0;


		}  // if (flag_uart || flag_spi || flag_i2c || flag_can)
	}  // while(1)
	mbedtls_aes_free(&aes_ctx);
	mbedtls_des_free(&des_ctx);
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
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void process_cmd() {
	if (rx_buffer[0] == 0x01 && rx_buffer[1] == 0x00) { // Set AES encrypt Key
		mbedtls_aes_setkey_enc(&aes_ctx, rx_buffer + CMD_LEN, key_length);
		status = 0;
		payload_len = 0;
	} else if (rx_buffer[0] == 0x01 && rx_buffer[1] == 0x01) { // Set AES decrypt Key
		mbedtls_aes_setkey_dec(&aes_ctx, rx_buffer + CMD_LEN, key_length);
		status = 0;
		payload_len = 0;
	} else if (rx_buffer[0] == 0x01 && rx_buffer[1] == 0x02) { // AES Encryption
		NUT_IO1_SET;
		mbedtls_aes_crypt_ecb(&aes_ctx, MBEDTLS_AES_ENCRYPT,
				rx_buffer + CMD_LEN, tx_buffer + STA_LEN);
		NUT_IO1_CLR;
		status = 0;
		payload_len = 16;
	} else if (rx_buffer[0] == 0x01 && rx_buffer[1] == 0x03) { // AES Decryption
		NUT_IO1_SET;
		mbedtls_aes_crypt_ecb(&aes_ctx, MBEDTLS_AES_DECRYPT,
				rx_buffer + CMD_LEN, tx_buffer + STA_LEN);
		NUT_IO1_CLR;
		status = 0;
		payload_len = 16;
	} else if (rx_buffer[0] == 0x02 && rx_buffer[1] == 0x00) { // Set DES encryption key
		mbedtls_des_setkey_enc(&des_ctx, rx_buffer + CMD_LEN);
		status = 0;
		payload_len = 0;
	} else if (rx_buffer[0] == 0x02 && rx_buffer[1] == 0x01) { // Set DES decryption key
		mbedtls_des_setkey_dec(&des_ctx, rx_buffer + CMD_LEN);
		status = 0;
		payload_len = 0;
	} else if (rx_buffer[0] == 0x02 && rx_buffer[1] == 0x02) { // DES Encryption
		NUT_IO1_SET;
		mbedtls_des_crypt_ecb(&des_ctx, rx_buffer + CMD_LEN,
				tx_buffer + STA_LEN);
		NUT_IO1_CLR;
		status = 0;
		payload_len = 8;
	} else if (rx_buffer[0] == 0x02 && rx_buffer[1] == 0x03) {
		NUT_IO1_SET;
		mbedtls_des_crypt_ecb(&des_ctx, rx_buffer + CMD_LEN,
				tx_buffer + STA_LEN);
		NUT_IO1_CLR;
		status = 0;
		payload_len = 8;
	} else {
		status = 0x8000;
		payload_len = 0;
	}
	return;
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
