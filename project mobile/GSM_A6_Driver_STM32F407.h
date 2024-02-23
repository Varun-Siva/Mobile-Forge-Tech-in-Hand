/**
  ***********************************************************************************************************
	*@file    : GSM_A6_Driver_STM32F407.h
	*@Author  : S Varun
	*@Brief   : Header file for Driving GSM-A6 Module using STM32F407 Discovery Kit
  ***********************************************************************************************************
*/
#ifndef GSM_A6_Driver_STM32F407_H
#define GSM_A6_Driver_STM32F407_H

#include "stm32f4xx_hal.h"

#define RX_BUFFER_SIZE             128

void UART_Config(void);

void DMA_Config(void);

int GSM_Compare_GSMData_With(const char* string);

void GSM_Send_AT_Command(char* AT_CMD);

void GSM_Make_Call(char* phone_number);

void GSM_Receive_Call(void);

void GSM_HangUp_Call(void);

void GSM_Send_SMS(char* Message, char* phone_number);

void GSM_Receive_SMS(void);

void GSM_Init(void);

void GSM_Clear_RX_buffer(void);

#endif
