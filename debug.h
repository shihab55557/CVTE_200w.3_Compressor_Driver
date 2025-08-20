



#include "stm32g030xx.h"


#define  DEBUG_TX_ENABLE
#define  DEBUG_RX_ENABLE
#define  DEBUG_PCKT_COMP_DELAY 20
#define  DEBUG_RX_BUF_SIZE     128



void     Debug_Struct_Init(void);
void     Debug_GPIO_Init(void);
void     Debug_Reg_Init(uint32_t baud_rate);

void     Debug_Timer_Init(void);
void     Debug_Timer_Enable(void);
void     Debug_Timer_Disable(void);
uint8_t  Debug_Get_Timer_Status(void);
uint16_t Debug_Get_Counter_Val(void);
void     Debug_Counter_Reset(void);

void     Debug_Tx_Byte(uint8_t val);
uint8_t  Debug_Rx_Byte(void);

void     Debug_Tx_Buf(uint8_t *data, uint8_t len);

void     Debug_Tx_NL(void);
void     Debug_Tx_SP(void);
void     Debug_Tx_CM(void);

void     Debug_Tx_Text(char *str);
void     Debug_Tx_Text_NL(char *str);
void     Debug_Tx_Text_SP(char *str);
void     Debug_Tx_Text_CM(char *str);

void     Debug_Determine_Digit_Numbers(uint32_t num);
void     Debug_Tx_Number_Digits(void);
void     Debug_Tx_Number(int32_t num);
void     Debug_Tx_Number_Hex(uint32_t val);
void     Debug_Tx_Number_Bin(uint32_t val);

void     Debug_Tx_Number_NL(int32_t num);
void     Debug_Tx_Number_SP(int32_t num);
void     Debug_Tx_Number_CM(int32_t num);

void     Debug_Tx_Number_Hex_NL(int32_t num);
void     Debug_Tx_Number_Hex_SP(int32_t num);
void     Debug_Tx_Number_Hex_CM(int32_t num);

void     Debug_Tx_Number_Bin_NL(int32_t num);
void     Debug_Tx_Number_Bin_SP(int32_t num);
void     Debug_Tx_Number_Bin_CM(int32_t num);


void     Debug_Tx_Parameter_NL(char *name, int32_t num);
void     Debug_Tx_Parameter_SP(char *name, int32_t num);
void     Debug_Tx_Parameter_CM(char *name, int32_t num);

void     Debug_Tx_Parameter_Hex_NL(char *name, int32_t num);
void     Debug_Tx_Parameter_Hex_SP(char *name, int32_t num);
void     Debug_Tx_Parameter_Hex_CM(char *name, int32_t num);

void     Debug_Tx_Parameter_Bin_NL(char *name, int32_t num);
void     Debug_Tx_Parameter_Bin_SP(char *name, int32_t num);
void     Debug_Tx_Parameter_Bin_CM(char *name, int32_t num);


//Receiver Functions
void     Debug_Flush_Buf(void);
uint8_t  Debug_Get_Buf(uint16_t index);
uint8_t  Debug_Get_Buf_Index(void);
uint8_t  Debug_Get_Data_Available_Flag(void);


//Data Availablity chek using timer
uint8_t  Debug_Data_Available(void);
void     Debug_Clear_Data_Available_Flag(void);
uint8_t  Debug_Get_Data_Len(void);
void     Debug_Data_Copy_Buf(uint8_t *buf);
void     Debug_Print_Buf_Data(void);



void     Debug_Init(uint32_t baud_rate);