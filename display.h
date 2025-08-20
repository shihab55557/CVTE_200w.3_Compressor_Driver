


#ifndef  _DISPLAY_H_
#define  _DISPLAY_H_

#include "stm32g030xx.h"



void     Display_Data_Struct_Init(void);
void     Display_TxData_Update_Buf(void);
void     Display_TxData_Header_Set(uint8_t val);
void     Display_TxData_Length_Set(uint8_t val);
void     Display_TxData_DoorSwitch_Set(uint8_t val);
void     Display_TxData_CabinetLight_Set(uint8_t val);
void     Display_TxData_CmdSpeed_Set(uint8_t val);
void     Display_TxData_DoorSwitch_DoorSwInv_Set(uint8_t val);
void     Display_TxData_DoorSwitch_DoorSwSelfCtrl_Set(uint8_t val);
void     Display_TxData_CabinetLight_CabinetLightEnable_Set(uint8_t val);
void     Display_TxData_Handler(void);


uint8_t  Display_RxData_Header_Get(void);
uint8_t  Display_RxData_Length_Get(void);

uint8_t  Display_RxData_DoorSwitch_Get(void);
uint8_t  Display_RxData_DoorSwitch_DoorSwSts_Get(void);
uint8_t  Display_RxData_DoorSwitch_DoorSwInv_Get(void);
uint8_t  Display_RxData_DoorSwitch_DoorSwSelfCtrl_Get(void);

uint8_t  Display_RxData_CabinetLight_Get(void);
uint8_t  Display_RxData_CabinetLight_CabinetLightEnable_Get(void);

uint16_t Display_RxData_NTC_ADC_Get(void);
uint8_t  Display_RxData_CmdSpeed_Get(void);
uint16_t Display_RxData_RealSpeed_Get(void);

uint32_t Display_RxData_AppCRC32_Get(void);
uint32_t Display_RxData_InvAppCRC32_Get(void);
uint32_t Display_RxData_InvErrorMode_Get(void);

uint16_t Display_RxData_CRC16_Get(void);



void     Display_Init(void);

void     Display_RxData_Update_Buf(void);







#endif
