#include "stm32g030xx.h"

#define  LED_IND_INC_BUTTON                                9U
#define  LED_IND_DEC_BUTTON                               10U
#define  LED_IND_NEGATIVE_POS                              2U
#define  CAPTOUCH_INC_BUTTON                                1
#define  CAPTOUCH_DEC_BUTTON                                0

#define  TIMEBASE_HEARTBEAT_WINDOW_S                        0
#define  TIMEBASE_HEARTBEAT_TIME_S                          1

#define  TIMEBASE_RX_PACKET_WINDOW_S                        1
#define  TIMEBASE_RX_PACKET_TIME_S                          4

#define  TIMEBASE_DISPLAY_PRINT_WINDOW_SS                   0
#define  TIMEBASE_DISPLAY_PRINT_TIME_S                    700

#define  DISPLAY_DRIVER_INIT_WINDOW_SS                      1
//#define  DISPLAY_DRIVER_INIT_TIME_SS                    100


void Dispfnc_Structure_Init(void);
void Dispfnc_Led_Ind_RPS_Inc(uint8_t val);
void Dispfnc_Led_Ind_RPS_Dec(uint8_t val);
void Dispfnc_Led_Ind_Neg_Led_Set(uint8_t val);
void Dispfnc_Print_Number(int8_t val);
uint8_t Dispfnc_Get_Realspeed_RPS(void);
void Dispfnc_Left_Side_LEDs(uint8_t state);
void Dispfnc_Right_Side_LEDs(uint8_t state);
void Dispfnc_Ind_Led_Time_Handler(void);
void Dispfnc_Ind_LEDS_On(void);
void Dispfnc_Ind_LEDS_Off(void);
uint8_t Dispfnc_Print_Number_Get_State(void);
void Dispfnc_Ind_LEDS_Toggle(void);
void Dispfnc_RPS_Print_And_LED_Handler(void);
void Dispfnc_Display_Normal_Mode_Print_Funtions(void);
uint8_t Dispfnc_Get_Set_RPS(void);
void Dispfnc_Set_Rx_Packet_Timer(void);
void Dispfnc_Get_Rx_Packet_Error(void);
uint8_t Dispfnc_Get_Rx_Packet_Error_Flag(void);
void Dispfnc_Tx_Rx_Handler(void);
void Dispfnc_Init(void);
void Dispfnc_Handler(void);