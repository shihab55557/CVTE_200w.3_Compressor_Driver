

/* 
 * File:   DispCom.h
 * Author: MD. Faridul Islam
 * faridmdislam@gmail.com
 * LL Driver -> Display->DispCom Library
 * Created on October 30, 2022, 19:00
 */
 



#ifndef  _DISPCOM_H_
#define  _DISPCOM_H_

#include "stm32g030xx.h"

void     DispCom_Struct_Init(void);
void     DispCom_RX_Packet_Struct_Init(void);

void     DispCom_Config_GPIO(void);
void     DispCom_Config_Clock(void);
void     DispCom_Config_BAUD_Rate(uint32_t baud_rate);
void     DispCom_Config_Tx(void);
void     DispCom_Config_Rx(void);
void     DispCom_Config_Rx_Interrupt(void);
void     DispCom_Clear_Interrupt_Flag(void);
void     DispCom_Tx_Byte(uint8_t val);
uint8_t  DispCom_Rx_Byte(void);


void     DispCom_Timer_Struct_Init(void);
void     DispCom_Timer_Init(void);
void     DispCom_Timer_Enable(void);
void     DispCom_Timer_Disable(void);
uint8_t  DispCom_Timer_Get_Status(void);
uint16_t DispCom_Timer_Get_Val(void);
void     DispCom_Timer_Value_Reset(void);
void     DispCom_Timer_Clear_Interrupt_Flag(void);

void     DispCom_Tx_Buf(uint8_t *data, uint8_t len);


void     DispCom_Tx_NL(void);
void     DispCom_Tx_SP(void);
void     DispCom_Tx_CM(void);


void     DispCom_Tx_Text(char *str);
void     DispCom_Tx_Text_NL(char *str);
void     DispCom_Tx_Text_SP(char *str);
void     DispCom_Tx_Text_CM(char *str);


void     DispCom_Determine_Digit_Numbers(uint32_t num);
void     DispCom_Tx_Number_Digits(void);
void     DispCom_Tx_Number(int32_t num);
void     DispCom_Tx_Number_Hex(uint32_t val);
void     DispCom_Tx_Number_Bin(uint32_t val);


void     DispCom_Tx_Number_NL(int32_t num);
void     DispCom_Tx_Number_SP(int32_t num);
void     DispCom_Tx_Number_CM(int32_t num);


void     DispCom_Tx_Number_Hex_NL(uint32_t num);
void     DispCom_Tx_Number_Hex_SP(uint32_t num);
void     DispCom_Tx_Number_Hex_CM(uint32_t num);


void     DispCom_Tx_Number_Bin_NL(uint32_t num);
void     DispCom_Tx_Number_Bin_SP(uint32_t num);
void     DispCom_Tx_Number_Bin_CM(uint32_t num);


void     DispCom_Tx_Parameter_NL(char *name, int32_t num);
void     DispCom_Tx_Parameter_SP(char *name, int32_t num);
void     DispCom_Tx_Parameter_CM(char *name, int32_t num);


void     DispCom_Tx_Parameter_Hex_NL(char *name, uint32_t num);
void     DispCom_Tx_Parameter_Hex_SP(char *name, uint32_t num);
void     DispCom_Tx_Parameter_Hex_CM(char *name, uint32_t num);


void     DispCom_Tx_Parameter_Bin_NL(char *name, uint32_t num);
void     DispCom_Tx_Parameter_Bin_SP(char *name, uint32_t num);
void     DispCom_Tx_Parameter_Bin_CM(char *name, uint32_t num);

//Receiver Functions
void     DispCom_Buf_Flush(void);
uint8_t  DispCom_Buf_Get(uint16_t index);
uint16_t DispCom_Buf_Get_Index(void);


//DispCom Data Functions
uint8_t  DispCom_Data_Available(void);
uint16_t DispCom_Data_Len_Get(void);

uint16_t DispCom_Data_Calculated_CRC_Get(void);
uint16_t DispCom_Data_Received_CRC_Get(void);
uint8_t  DispCom_Data_CRC_Status_Get(void);
uint8_t  DispCom_Data_Read_Complete_Status(void);

void     DispCom_Data_Clear_Available_Flag(void);
void     DispCom_Data_Clear_Read_Complete_Flag(void);

void     DispCom_Data_Copy_Buf(uint8_t *buf);
void     DispCom_Data_Print_Buf(void);

uint8_t  DispCom_Error_Code_Get(void);
void     DispCom_Error_Code_Clear(void);

void     DispCom_ISR_Handler(void);
void     DispCom_Timer_ISR_Handler(void);


uint16_t DispCom_CRC_Calculate_Byte(uint16_t crc, uint8_t data);
uint16_t DispCom_CRC_Calculate_Block(uint8_t *buf, uint8_t len);

void     DispCom_RX_Packet_CRC_Check(void);
void     DispCom_RX_Packet_Read_Complete(void);

void     DispCom_Init(uint32_t baud);

#endif