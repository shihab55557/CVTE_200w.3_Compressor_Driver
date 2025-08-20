 #include "stm32g030xx.h"

 void     I2C2_GPIO_Init(void);
 void     I2C2_Clock_Init(void);
 void     I2C2_Config(void);
 void     I2C2_Init(void);
 uint8_t  I2C2_TX_ADDR(uint8_t addr);
 uint8_t  I2C2_TX_ADDR_READ(uint8_t addr);
 void     I2C2_Write_Register(uint8_t addr, uint8_t data);
 void     CapTouch_Struct_Init(void);
 void     I2C2_Read_Captouch_Sensors(void);
 void     CapTouch_Response(uint8_t current_channel);
 void     CapTouch_Check_Trigger(uint8_t current_channel);
 void     CapTouch_Check_Touch_Duration(uint8_t current_channel, uint32_t refernce_time);
 void     CapTouch_Scan_Sensors(uint32_t refernce_time);
 uint8_t  CapTouch_Short_Pressed(uint8_t current_channel);
 uint8_t  CapTouch_Long_Pressed(uint8_t current_channel);
 uint8_t  CapTouch_Constant_Pressed(uint8_t current_channel);
 uint32_t CapTouch_Get_Touch_Duration(uint8_t current_channel);
 uint8_t  CapTouch_Get_Touch_Response(uint8_t current_channel);
 uint8_t  CapTouch_Get_Reference_Timer_Use_Flag(void);
 uint8_t  CapTouch_Get_ErrorCode(uint8_t current_channel);
 void     CapTouch_Get_Debug_Data(uint8_t current_channel, int *data_out);
 void     CapTouch_Init(void);

uint8_t   Captouch_Get_Reference_Timer_Use_Flag(void);