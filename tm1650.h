#include "stm32g030xx.h"


void     TM1650_Struct_Init(void);
void     TM1650_GPIO_Init(void);
void     TM1650_Bit_Delay(void);

void     TM1650_SCL_Input(void);
void     TM1650_SCL_Output(void);
void     TM1650_SCL_Set(uint8_t state);
uint8_t  TM1650_SCL_Get(void);

void     TM1650_SDA_Input(void);
void     TM1650_SDA_Output(void);
void     TM1650_SDA_Set(uint8_t state);
uint8_t  TM1650_SDA_Get(void);

uint8_t  TM1650_Ack_Get(void);
uint8_t  TM1650_Ack_Get_With_Timeout(void);
void     TM1650_Ack_Send(void);
void     TM1650_Nack_Send(void);

void     TM1650_Start(void);
void     TM1650_Stop(void);
void     TM1650_Tx_Byte(uint8_t data);
uint8_t  TM1650_Rx_Byte(void);
void     TM1650_Clear_Error(void);
void     TM1650_Error_Handler(void);

void     TM1650_Write(uint8_t data);
uint8_t  TM1650_Read(void);
void     TM1650_Write_Cmd_Sevenseg(uint8_t cmd, uint8_t cmd2, uint8_t brt);
void     TM1650_Write_Data(uint8_t address, uint8_t data);
uint8_t  TM1650_Read_Key_Cmd(void);
void     TM1650_Clear_Display(void);

uint8_t  TM1650_Get_ErrorCode(void);
uint8_t  TM1650_Get_LastErrorCode(void);
uint8_t  TM1650_Get_ErrorStep(void);
uint8_t  TM1650_Get_LastErrorStep(void);
uint8_t  TM1650_Get_ErrorStepHandle(void);
void     TM1650_Error_Handled_Clear_Errors(void);
uint16_t TM1650_Get_ErrorsTotal(void);

void 		 TM1650_Init(void);
void 		 TM1650_Display_All_Led_ON(void);
void 		 TM1650_Display_All_Led_OFF(void);
void 		 TM1650_Display_Deem_Smoothly(void);
void 		 TM1650_Display_Fully_Bright(void);
void 		 TM1650_Display_Off_CMD(void);
void 		 TM1650_Display_On_CMD(void);
void 	 	 TM1650_Display_Write_Dig1(uint8_t val);
void 		 TM1650_Display_Write_Dig2(uint8_t val);
void 		 TM1650_Display_Write_Dig3(uint8_t val);
void 		 TM1650_Display_Write_Dig4(uint8_t val);

void     TM1650_Display_Set_State_LED1_LED8(uint8_t pos, uint8_t val);
uint8_t  TM1650_Display_Get_State_LED1_LED8(uint8_t pos);
void     TM1650_Display_Toggle_LED1_LED8(uint8_t pos);
void     TM1650_Display_Set_State_LED9_LED10(uint8_t pos, uint8_t val);
uint8_t  TM1650_Display_Get_State_LED9_LED10(uint8_t pos);
void     TM1650_Display_Toggle_LED9_LED10(uint8_t pos);

void     TM1650_Display_Set_State_LED(uint8_t pos, uint8_t val);
uint8_t  TM1650_Display_Get_State_LED(uint8_t pos);
void     TM1650_Display_Toggle_LED(uint8_t pos);

void		 TM1650_Display_Print_Num_Unsigned(int8_t val);
void 		 TM1650_Display_Print_CL(void);
void 		 TM1650_Display_Print_UL(void);
void 		 TM1650_Display_Print_CE(void);
void 	 	 TM1650_Write_System_Cmd(uint8_t cmd);
	

	
	
	
	