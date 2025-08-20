



#include "stm32g030xx.h"
#include "dispcom.h"
#include "display.h"

typedef struct display_data_t{
	uint8_t   Header;
	uint8_t   Length;
	uint8_t   DoorSwitch;
	uint8_t   CabinetLight;
	uint8_t   NTC_ADC_H;
	uint8_t   NTC_ADC_L;
	uint16_t  NTC_ADC;
	uint8_t   CmdSpeed;
	uint8_t   Reserved0;
	uint16_t  RealSpeed;
	uint32_t  AppCRC32;
	uint32_t  InvAppCRC32;
	uint32_t  InvErrorMode;
	uint8_t   CRC_H;
	uint8_t   CRC_L;
	uint16_t  CRC16;
	uint8_t   ByteCount;
	uint8_t   Buf[39];
}display_data_t;

typedef struct display_t{
	display_data_t RxData;
	display_data_t TxData;
}display_t;

static display_t Display;


void Display_Data_Struct_Init(void){
	Display.RxData.Header = 0x00;
	Display.RxData.Length = 0x00;
  Display.RxData.DoorSwitch = 0x00;
  Display.RxData.CabinetLight = 0x00;
  Display.RxData.NTC_ADC_H = 0x00;
	Display.RxData.NTC_ADC_L = 0x00;
	Display.RxData.NTC_ADC = 0x00;
	Display.RxData.CmdSpeed = 0x00;
	Display.RxData.RealSpeed = 0x00;
	
	Display.RxData.AppCRC32    = 0x00;
	Display.RxData.InvAppCRC32 = 0x00;
	Display.RxData.InvErrorMode = 0x00;
	
	Display.RxData.CRC_H = 0x00;
	Display.RxData.CRC_L = 0x00;
  Display.RxData.CRC16 = 0x00;
	
	Display.TxData.Header = 0xA5;
	Display.TxData.Length = 0x0E;
  Display.TxData.DoorSwitch = 0xF1;
  Display.TxData.CabinetLight = 0xFE;
  Display.TxData.NTC_ADC_H = 0x00;
	Display.TxData.NTC_ADC_L = 0x00;
	Display.TxData.NTC_ADC = 0x00;
	Display.TxData.CmdSpeed = 0x00;
	Display.TxData.RealSpeed = 0x00;
	Display.TxData.CRC_H = 0x00;
	Display.TxData.CRC_L = 0x00;
  Display.TxData.CRC16 = 0x00;
	
	Display.TxData.ByteCount = 0x00;
}

void Display_TxData_Update_Buf(void){
	Display.TxData.Buf[0]  = Display.TxData.Header;
	Display.TxData.Buf[1]  = Display.TxData.Length;
	
	Display.TxData.Buf[2]  = Display.TxData.DoorSwitch;
	Display.TxData.Buf[3]  = Display.TxData.CabinetLight;
	
	Display.TxData.Buf[4]  = Display.TxData.NTC_ADC_H;
	Display.TxData.Buf[5]  = Display.TxData.NTC_ADC_L;
	
	Display.TxData.Buf[6]  = Display.TxData.CmdSpeed;
	Display.TxData.Buf[7]  = (Display.TxData.RealSpeed & 0xFF);
	
	Display.TxData.Buf[8]  = 0x00;
	Display.TxData.Buf[9]  = 0x00;
	Display.TxData.Buf[10] = 0x00;
	Display.TxData.Buf[11] = 0x00;
	
	Display.TxData.CRC16   = DispCom_CRC_Calculate_Block(Display.TxData.Buf, 12);
	Display.TxData.CRC_H   = (uint8_t)(Display.TxData.CRC16 >> 8);
	Display.TxData.CRC_L   = (uint8_t)(Display.TxData.CRC16 & 0xFF);
	Display.TxData.Buf[12] = Display.TxData.CRC_H;
	Display.TxData.Buf[13] = Display.TxData.CRC_L;
}


void Display_TxData_Header_Set(uint8_t val){
	Display.TxData.Header = val;
}

void Display_TxData_Length_Set(uint8_t val){
	Display.TxData.Length = val;
}

void Display_TxData_DoorSwitch_Set(uint8_t val){
	Display.TxData.DoorSwitch = val;
}

void Display_TxData_CabinetLight_Set(uint8_t val){
	Display.TxData.CabinetLight = val;
}

void Display_TxData_CmdSpeed_Set(uint8_t val){
	Display.TxData.CmdSpeed = val;
}

void Display_TxData_DoorSwitch_DoorSwInv_Set(uint8_t val){
	uint8_t temp = Display.TxData.DoorSwitch;
	if(val == 0){
		temp &=~ (1<<2);
	}
	else if(val == 1){
		temp |=  (1<<2);
	}
  Display_TxData_DoorSwitch_Set(temp);
}

void Display_TxData_DoorSwitch_DoorSwSelfCtrl_Set(uint8_t val){
	uint8_t temp = Display.TxData.DoorSwitch;
	if(val == 0){
		temp &=~ (1<<3);
	}
	else if(val == 1){
		temp |=  (1<<3);
	}
  Display_TxData_DoorSwitch_Set(temp);
}


void Display_TxData_CabinetLight_CabinetLightEnable_Set(uint8_t val){
	uint8_t temp = Display.TxData.CabinetLight;
	if(val == 0){
		temp &=~ (1<<0);
	}
	else if(val == 1){
		temp |=  (1<<0);
	}
  Display_TxData_CabinetLight_Set(temp);
}


void Display_TxData_Handler(void){
	Display_TxData_Update_Buf();
	DispCom_Tx_Buf(Display.TxData.Buf, 14);
}







void Display_RxData_Update_Buf(void){
	Display.RxData.Header       = DispCom_Buf_Get(0);
	Display.RxData.Length       = DispCom_Buf_Get(1);
	Display.RxData.DoorSwitch   = DispCom_Buf_Get(2);
	Display.RxData.CabinetLight = DispCom_Buf_Get(3);
	Display.RxData.NTC_ADC_H    = DispCom_Buf_Get(4);
	Display.RxData.NTC_ADC_L    = DispCom_Buf_Get(5);
	Display.RxData.NTC_ADC      = Display.RxData.NTC_ADC_H;
	Display.RxData.NTC_ADC    <<= 8;
	Display.RxData.NTC_ADC     |= Display.RxData.NTC_ADC_L;
	Display.RxData.CmdSpeed     = DispCom_Buf_Get(6);
	
	//Fetch RealSpeed,  2 bytes
	Display.RxData.RealSpeed    = DispCom_Buf_Get(7);
	Display.RxData.RealSpeed  <<= 8;
	Display.RxData.RealSpeed   |= DispCom_Buf_Get(8);
	
	//Fetch AppCRC32,  4 bytes
	Display.RxData.AppCRC32     = DispCom_Buf_Get(9);
	Display.RxData.AppCRC32   <<= 8;
	Display.RxData.AppCRC32    |= DispCom_Buf_Get(10);
	Display.RxData.AppCRC32   <<= 8;
	Display.RxData.AppCRC32    |= DispCom_Buf_Get(11);
	Display.RxData.AppCRC32   <<= 8;
	Display.RxData.AppCRC32    |= DispCom_Buf_Get(12);
	
	//Fetch InvAppCRC32,  4 bytes
	Display.RxData.InvAppCRC32  = DispCom_Buf_Get(13);
	Display.RxData.InvAppCRC32<<= 8;
	Display.RxData.InvAppCRC32 |= DispCom_Buf_Get(14);
	Display.RxData.InvAppCRC32<<= 8;
	Display.RxData.InvAppCRC32 |= DispCom_Buf_Get(15);
	Display.RxData.InvAppCRC32<<= 8;
	Display.RxData.InvAppCRC32 |= DispCom_Buf_Get(16);
	
	//Fetch InvErrorMode,  4 bytes
	Display.RxData.InvErrorMode   = DispCom_Buf_Get(17);
	Display.RxData.InvErrorMode <<= 8;
	Display.RxData.InvErrorMode  |= DispCom_Buf_Get(18);
	Display.RxData.InvErrorMode <<= 8;
	Display.RxData.InvErrorMode  |= DispCom_Buf_Get(19);
	Display.RxData.InvErrorMode <<= 8;
	Display.RxData.InvErrorMode  |= DispCom_Buf_Get(20);
	
	
	Display.RxData.CRC_H        = DispCom_Buf_Get(21);
	Display.RxData.CRC_L        = DispCom_Buf_Get(22);
	Display.RxData.CRC16        = Display.RxData.CRC_H;
	Display.RxData.CRC16      <<= 8;
	Display.RxData.CRC16       |= Display.RxData.CRC_L;
}

uint8_t Display_RxData_Header_Get(void){
	return Display.RxData.Header;
}

uint8_t Display_RxData_Length_Get(void){
	return Display.RxData.Length;
}

uint8_t Display_RxData_DoorSwitch_Get(void){
	return Display.RxData.DoorSwitch;
}

uint8_t Display_RxData_DoorSwitch_DoorSwSts_Get(void){
	if(Display.RxData.DoorSwitch & (1<<1)){
		return 1;
	}
	else{
		return 0;
	}
}

uint8_t Display_RxData_DoorSwitch_DoorSwInv_Get(void){
	if(Display.RxData.DoorSwitch & (1<<2)){
		return 1;
	}
	else{
		return 0;
	}
}

uint8_t Display_RxData_DoorSwitch_DoorSwSelfCtrl_Get(void){
	if(Display.RxData.DoorSwitch & (1<<3)){
		return 1;
	}
	else{
		return 0;
	}
}


uint8_t Display_RxData_CabinetLight_Get(void){
	return Display.RxData.CabinetLight;
}


uint8_t Display_RxData_CabinetLight_CabinetLightEnable_Get(void){
	if(Display.RxData.CabinetLight & (1<<0)){
		return 1;
	}
	else{
		return 0;
	}
}



uint16_t Display_RxData_NTC_ADC_Get(void){
	return Display.RxData.NTC_ADC;
}

uint8_t Display_RxData_CmdSpeed_Get(void){
	return Display.RxData.CmdSpeed;
}

uint16_t Display_RxData_RealSpeed_Get(void){
	return Display.RxData.RealSpeed;
}

uint32_t Display_RxData_AppCRC32_Get(void){
	return Display.RxData.AppCRC32;
}

uint32_t Display_RxData_InvAppCRC32_Get(void){
	return Display.RxData.InvAppCRC32;
}

uint32_t Display_RxData_InvErrorMode_Get(void){
	return Display.RxData.InvErrorMode;
}

uint16_t Display_RxData_CRC16_Get(void){
	return Display.RxData.CRC16;
}






void Display_Init(void){
	Display_Data_Struct_Init();
	Display_TxData_Update_Buf();
	
}