/******************************************************************************
 * @file    vcom.c
 * @author  MCD Application Team
 * @version V1.1.4
 * @date    10-July-2018
 * @brief   manages virtual com port
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics International N.V.
 * All rights reserved.</center></h2>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted, provided that the following conditions are met:
 *
 * 1. Redistribution of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of STMicroelectronics nor the names of other
 *    contributors to this software may be used to endorse or promote products
 *    derived from this software without specific written permission.
 * 4. This software, including modifications and/or derivative works of this
 *    software, must execute solely and exclusively on microcontroller or
 *    microprocessor devices manufactured by or for STMicroelectronics.
 * 5. Redistribution and use of this software other than as permitted under
 *    this license is void and will automatically terminate your rights under
 *    this license.
 *
 * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
 * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT
 * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************
 */

#include "hw.h"
#include "vcom.h"
#include "timeServer.h"
#include "delay.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Uart Handle */
UART_HandleTypeDef UartHandle;
UART_HandleTypeDef UartHandle1;

bool flags_command = 0;
uint8_t charRx;
uint8_t num = 0;
uint8_t num_check = 0;
uint8_t num_check2 = 0;
uint8_t aRxBuffer[1];
uint8_t response[50] = { 0x00 };
uint8_t responsetemp[1] = { 0x00 };
uint8_t responsecheck[1] = { 0x00 };
//uint8_t uartprintf_flag=0;
extern bool flags_command_check;
extern uint8_t rxdatacheck[7];

static void (*TxCpltCallback)(void);

static void (*RxCpltCallback)(uint8_t *rxChar);
/* Private function prototypes -----------------------------------------------*/
/* Functions Definition ------------------------------------------------------*/
void vcom_Init(void (*TxCb)(void)) {

	/*Record Tx complete for DMA*/
	TxCpltCallback = TxCb;
	/*## Configure the UART peripheral ######################################*/
	/* Put the USART peripheral in the Asynchronous mode (UART Mode) */
	/* UART1 configured as follow:
	 - Word Length = 8 Bits
	 - Stop Bit = One Stop bit
	 - Parity = ODD parity
	 - BaudRate = 921600 baud
	 - Hardware flow control disabled (RTS and CTS signals) */
	UartHandle.Instance = USARTX;

	UartHandle.Init.BaudRate = 9600;
	UartHandle.Init.WordLength = UART_WORDLENGTH_8B;
	UartHandle.Init.StopBits = UART_STOPBITS_1;
	UartHandle.Init.Parity = UART_PARITY_NONE;
	UartHandle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	UartHandle.Init.Mode = UART_MODE_TX_RX;

	if (HAL_UART_Init(&UartHandle) != HAL_OK) {
		/* Initialization Error */
		Error_Handler();
	}
}

void vcom_Trace(uint8_t *p_data, uint16_t size) {
	HAL_UART_Transmit_DMA(&UartHandle, p_data, size);
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *UartHandle) {
	/* buffer transmission complete*/
	if (NULL != TxCpltCallback) {
		TxCpltCallback();
	}
}

void vcom_ReceiveInit(void (*RxCb)(uint8_t *rxChar)) {
	UART_WakeUpTypeDef WakeUpSelection;

	/*record call back*/
	RxCpltCallback = RxCb;

	/*Set wakeUp event on start bit*/
	WakeUpSelection.WakeUpEvent = UART_WAKEUP_ON_STARTBIT;
//  
	HAL_UARTEx_StopModeWakeUpSourceConfig(&UartHandle, WakeUpSelection);

	/*Enable wakeup from stop mode*/
	HAL_UARTEx_EnableStopMode(&UartHandle);

	/*Start LPUART receive on IT*/
	HAL_UART_Receive_IT(&UartHandle, &charRx, 1);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle) {
	if (UartHandle->Instance == LPUART1) {
		if ((NULL != RxCpltCallback)
				&& (HAL_UART_ERROR_NONE == UartHandle->ErrorCode)) {
			RxCpltCallback(&charRx);
		}
		HAL_UART_Receive_IT(UartHandle, &charRx, 1);
	} else if (UartHandle->Instance == USART1) {
		while (HAL_UART_Receive_IT(&UartHandle1, (uint8_t*) aRxBuffer, 1)
				!= HAL_OK) {
			UartHandle1.RxState = HAL_UART_STATE_READY;
			__HAL_UNLOCK(&UartHandle1);
		}
		responsetemp[0] = aRxBuffer[0];
		if (flags_command == 1) {
			response[num++] = responsetemp[0];
		} else if (flags_command_check == 1) {
			rxdatacheck[num_check++] = responsetemp[0];
			if (num_check == 8) {
				flags_command_check = 0;
			}
		}
	}
}

void vcom_DMA_TX_IRQHandler(void) {
	HAL_DMA_IRQHandler(UartHandle.hdmatx);
}

void vcom_IRQHandler(void) {
	HAL_UART_IRQHandler(&UartHandle);
}

void tfmini_uart_IRQHandler(void) {
	HAL_UART_IRQHandler(&UartHandle1);
}

void vcom_DeInit(void) {
	HAL_UART_DeInit(&UartHandle);
}

void HAL_UART_MspInit(UART_HandleTypeDef *huart) {
	if (huart->Instance == LPUART1) {
		static DMA_HandleTypeDef hdma_tx;

		/*##-1- Enable peripherals and GPIO Clocks #################################*/
		/* Enable GPIO TX/RX clock */
		USARTX_TX_GPIO_CLK_ENABLE();
		USARTX_RX_GPIO_CLK_ENABLE();

		/* Enable USARTX clock */
		USARTX_CLK_ENABLE();
		/* select USARTX clock source*/
		RCC_PeriphCLKInitTypeDef PeriphClkInit = { 0 };
		PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_LPUART1;
		PeriphClkInit.Lpuart1ClockSelection = RCC_LPUART1CLKSOURCE_HSI;
		HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);

		/* Enable DMA clock */
		DMAX_CLK_ENABLE();

		/*##-2- Configure peripheral GPIO ##########################################*/
		/* UART  pin configuration  */
		vcom_IoInit();

		/*##-3- Configure the DMA ##################################################*/
		/* Configure the DMA handler for Transmission process */
		hdma_tx.Instance = USARTX_TX_DMA_CHANNEL;
		hdma_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
		hdma_tx.Init.PeriphInc = DMA_PINC_DISABLE;
		hdma_tx.Init.MemInc = DMA_MINC_ENABLE;
		hdma_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
		hdma_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
		hdma_tx.Init.Mode = DMA_NORMAL;
		hdma_tx.Init.Priority = DMA_PRIORITY_LOW;
#ifndef STM32L152xE
		hdma_tx.Init.Request = USARTX_TX_DMA_REQUEST;
#endif
		HAL_DMA_Init(&hdma_tx);

		/* Associate the initialized DMA handle to the UART handle */
		__HAL_LINKDMA(huart, hdmatx, hdma_tx);

		/*##-4- Configure the NVIC for DMA #########################################*/
		/* NVIC configuration for DMA transfer complete interrupt*/
		HAL_NVIC_SetPriority(USARTX_DMA_TX_IRQn, USARTX_Priority, 1);
		HAL_NVIC_EnableIRQ(USARTX_DMA_TX_IRQn);

		/* NVIC for USART, to catch the TX complete */
		HAL_NVIC_SetPriority(USARTX_IRQn, USARTX_DMA_Priority, 1);
		HAL_NVIC_EnableIRQ(USARTX_IRQn);
	} else if (huart->Instance == USART1) {
		__HAL_RCC_USART1_CLK_ENABLE();
		uart1_IoDeInit();
		HAL_NVIC_SetPriority(USART1_IRQn, 2, 0);
		HAL_NVIC_EnableIRQ(USART1_IRQn);
	}
}

void HAL_UART_MspDeInit(UART_HandleTypeDef *huart) {
	vcom_IoDeInit();
	/*##-1- Reset peripherals ##################################################*/
	USARTX_FORCE_RESET();
	USARTX_RELEASE_RESET();

	/*##-3- Disable the DMA #####################################################*/
	/* De-Initialize the DMA channel associated to reception process */
	if (huart->hdmarx != 0) {
		HAL_DMA_DeInit(huart->hdmarx);
	}
	/* De-Initialize the DMA channel associated to transmission process */
	if (huart->hdmatx != 0) {
		HAL_DMA_DeInit(huart->hdmatx);
	}

	/*##-4- Disable the NVIC for DMA ###########################################*/
	HAL_NVIC_DisableIRQ(USARTX_DMA_TX_IRQn);
}

void vcom_IoInit(void) {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	/* Enable GPIO TX/RX clock */
	USARTX_TX_GPIO_CLK_ENABLE();
	USARTX_RX_GPIO_CLK_ENABLE();
	/* UART TX GPIO pin configuration  */
	GPIO_InitStruct.Pin = USARTX_TX_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	GPIO_InitStruct.Alternate = USARTX_TX_AF;

	HAL_GPIO_Init(USARTX_TX_GPIO_PORT, &GPIO_InitStruct);

	/* UART RX GPIO pin configuration  */
	GPIO_InitStruct.Pin = USARTX_RX_PIN;
	GPIO_InitStruct.Alternate = USARTX_RX_AF;

	HAL_GPIO_Init(USARTX_RX_GPIO_PORT, &GPIO_InitStruct);
}

void uart1_init_uart1(void) {
	UartHandle1.Instance = USART1;

	UartHandle1.Init.BaudRate = 115200;
	UartHandle1.Init.WordLength = UART_WORDLENGTH_8B;
	UartHandle1.Init.StopBits = UART_STOPBITS_1;
	UartHandle1.Init.Parity = UART_PARITY_NONE;
	UartHandle1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	UartHandle1.Init.Mode = UART_MODE_TX_RX;

	if (HAL_UART_Init(&UartHandle1) != HAL_OK) {
		Error_Handler();
	}
}

void uart1_IoInit(void) {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	/* Enable GPIO TX/RX clock */
	__GPIOA_CLK_ENABLE()
	;
	/* UART TX GPIO pin configuration  */
	GPIO_InitStruct.Pin = GPIO_PIN_9;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF4_USART1;

	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/* UART RX GPIO pin configuration  */
	GPIO_InitStruct.Pin = GPIO_PIN_10;
	GPIO_InitStruct.Alternate = GPIO_AF4_USART1;

	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void vcom_IoDeInit(void) {
	GPIO_InitTypeDef GPIO_InitStructure = { 0 };

	USARTX_TX_GPIO_CLK_ENABLE();

	GPIO_InitStructure.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStructure.Pull = GPIO_NOPULL;

	GPIO_InitStructure.Pin = USARTX_TX_PIN;
	HAL_GPIO_Init( USARTX_TX_GPIO_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.Pin = USARTX_RX_PIN;
	HAL_GPIO_Init( USARTX_RX_GPIO_PORT, &GPIO_InitStructure);
}

void uart1_IoDeInit(void) {
	__HAL_RCC_GPIOB_CLK_ENABLE();

	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Pin = GPIO_PIN_9;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	GPIO_InitTypeDef GPIO_InitStruct2;
	GPIO_InitStruct2.Pin = GPIO_PIN_10;
	GPIO_InitStruct2.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct2.Pull = GPIO_NOPULL;
	GPIO_InitStruct2.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct2);
}

void at_tfmini_data_receive(uint8_t rxdatatemp[], uint16_t delayvalue) {
	uint8_t responsetemp[1];
	uint8_t begin = 0, datanumber = 0;
	uint8_t txenoutput[5] = { 0x5A, 0x05, 0x07, 0x01, 0x67 };

	HAL_UART_Transmit(&UartHandle1, txenoutput, 5, 0xFFFF);
	flags_command = 1;
	HAL_UART_Receive_IT(&UartHandle1, (uint8_t*) aRxBuffer, 1);
	DelayMs(delayvalue);
	flags_command = 0;
	num = 0;

	for (uint8_t number = 0; number < sizeof(response); number++) {
		if (begin == 1) {
			rxdatatemp[datanumber++] = response[number];
			if (datanumber == 7) {
				begin = 2;
			}
		}
		if ((responsetemp[0] == 0x59) && (begin == 0)) {
			if (response[number] == 0x59) {
				begin = 1;
			} else {
				responsetemp[0] = 0x00;
			}
		} else if (response[number] == 0x59 && (begin == 0)) {
			responsetemp[0] = 0x59;
		}
		response[number] = 0x00;
	}
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
