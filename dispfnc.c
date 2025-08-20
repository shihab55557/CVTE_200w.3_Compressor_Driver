#include "stm32g030xx.h"
#include "captouch.h"
#include "timebase.h"
#include "dispcom.h"
#include "dispfnc.h"
#include "display.h"
#include "tm1650.h"
#include "debug.h"
#include "cdef.h"

typedef struct Comp_param{
	int8_t set_rps;
	uint8_t print_flag;
	uint8_t com_state;
}Comp_param;

Comp_param Comp__param;

typedef struct Disp_com{
	uint8_t rx_packet_count;
	uint8_t rx_error_flag;
}Disp_com;

Disp_com Disp__com;



void Dispfnc_Structure_Init(void){
	Comp__param.set_rps = 0;
	Comp__param.print_flag = 0;
	Comp__param.com_state = 0;
	Disp__com.rx_packet_count = 0;
	Disp__com.rx_error_flag = 0;
}

enum{
	cmdspd_received = 0,
	realspd_received = 1,
	not_connected = 2
};






void Dispfnc_Led_Ind_RPS_Inc(uint8_t val){
	TM1650_Display_Set_State_LED(LED_IND_INC_BUTTON, val);
}

void Dispfnc_Led_Ind_RPS_Dec(uint8_t val){
	TM1650_Display_Set_State_LED(LED_IND_DEC_BUTTON, val);
}

void Dispfnc_Left_Side_LEDs(uint8_t state){
	TM1650_Display_Set_State_LED(1, state);
	TM1650_Display_Set_State_LED(6, state);
	TM1650_Display_Set_State_LED(8, state);
}

void Dispfnc_Right_Side_LEDs(uint8_t state){
	TM1650_Display_Set_State_LED(4, state);
	TM1650_Display_Set_State_LED(5, state);
	TM1650_Display_Set_State_LED(7, state);
}


void Dispfnc_Led_Ind_Neg_Led_Set(uint8_t val){
	TM1650_Display_Set_State_LED(LED_IND_NEGATIVE_POS, val);
}

void Dispfnc_Print_Number(int8_t val){
	if(val < 0){
		Dispfnc_Led_Ind_Neg_Led_Set( ON );
		val = -val;
	}
	else{
		Dispfnc_Led_Ind_Neg_Led_Set( OFF );
	}
	TM1650_Display_Print_Num_Unsigned(val);
}

uint8_t Dispfnc_Get_Realspeed_RPS(void){
	uint16_t realspeed = 0;
	realspeed = Display_RxData_RealSpeed_Get()/60;
	return realspeed;
}

uint8_t Dispfnc_Get_Set_RPS(void){
	return Comp__param.set_rps;
}

void Dispfnc_Ind_Led_Time_Handler(void) {
    if( (Dispfnc_Get_Realspeed_RPS() >= (Dispfnc_Get_Set_RPS() * 97.0 / 100.0)) &&
        (Dispfnc_Get_Realspeed_RPS() <= (Dispfnc_Get_Set_RPS() * 102.0 / 100.0)) &&
        (Dispfnc_Get_Set_RPS() != 0) )
    {
        Comp__param.com_state = realspd_received;
    }
    else if( (Display_RxData_CmdSpeed_Get() >= (Dispfnc_Get_Set_RPS() * 97.0 / 100.0)) &&
             (Display_RxData_CmdSpeed_Get() <= (Dispfnc_Get_Set_RPS() * 102.0 / 100.0)) &&
             (Dispfnc_Get_Set_RPS() != 0) )
    {
        Comp__param.com_state = cmdspd_received;
    }
    else {
        Comp__param.com_state = not_connected;
    }
}

void Dispfnc_Ind_LEDS_On(void){
	Comp__param.print_flag = ON;
	if(Comp__param.com_state == not_connected){
		Dispfnc_Left_Side_LEDs(OFF);
		Dispfnc_Right_Side_LEDs(OFF);
	}
	else if(Comp__param.com_state == cmdspd_received){
		Dispfnc_Left_Side_LEDs(ON);
		Dispfnc_Right_Side_LEDs(OFF);
	}
	else if(Comp__param.com_state == realspd_received){
		Dispfnc_Left_Side_LEDs(ON);
		Dispfnc_Right_Side_LEDs(ON);
	}
}

void Dispfnc_Ind_LEDS_Off(void){
	Comp__param.print_flag =  OFF;
	Dispfnc_Left_Side_LEDs(OFF);
	Dispfnc_Right_Side_LEDs(OFF);
}

uint8_t Dispfnc_Print_Number_Get_State(void){
	return Comp__param.print_flag;
}

void Dispfnc_Ind_LEDS_Toggle(void){
	if(Dispfnc_Print_Number_Get_State() != ON){
		Dispfnc_Ind_LEDS_On();
	}
	else{
		Dispfnc_Ind_LEDS_Off();
	}
}

void Dispfnc_RPS_Print_And_LED_Handler(void){
	Dispfnc_Ind_Led_Time_Handler();
	Dispfnc_Print_Number(Comp__param.set_rps);
	if(Timebase_DownCounter_SS_Expired_Event(TIMEBASE_DISPLAY_PRINT_WINDOW_SS)){
		Dispfnc_Ind_LEDS_Toggle();
		Timebase_DownCounter_SS_Set_Securely(TIMEBASE_DISPLAY_PRINT_WINDOW_SS, TIMEBASE_DISPLAY_PRINT_TIME_S);
	}
}

void Dispfnc_Set_RPS_Handler(void){
	if(CapTouch_Short_Pressed(CAPTOUCH_INC_BUTTON)){
		Comp__param.set_rps += 5;
		if(Comp__param.set_rps > 75){
			Comp__param.set_rps = 0;
		}
	}
	if(CapTouch_Short_Pressed(CAPTOUCH_DEC_BUTTON)){
		Comp__param.set_rps--;
		if(Comp__param.set_rps < 0 ){
			Comp__param.set_rps = 0;
		}
	}
	if(CapTouch_Long_Pressed(CAPTOUCH_DEC_BUTTON)){
		Comp__param.set_rps = 0;
	}
}


void Dispfnc_Display_Normal_Mode_Print_Funtions(void){
	Dispfnc_Set_RPS_Handler();
	Dispfnc_Led_Ind_RPS_Inc(ON);
	Dispfnc_Led_Ind_RPS_Dec(ON);
	Dispfnc_RPS_Print_And_LED_Handler();
	Display_TxData_CmdSpeed_Set(Comp__param.set_rps);
	//Debug_Tx_Parameter_NL("Setspeed ", Display_RxData_CmdSpeed_Get());
}


void Dispfnc_Display_Error_Mode_Print_Funtions(void){
	TM1650_Display_Print_CE();
	Dispfnc_Left_Side_LEDs(OFF);
	Dispfnc_Right_Side_LEDs(OFF);
}

void Dispfnc_Display_Print_Funtions(void){
	if(Dispfnc_Get_Rx_Packet_Error_Flag() == FALSE){
		Dispfnc_Display_Normal_Mode_Print_Funtions();
	}
	else{
		Dispfnc_Display_Error_Mode_Print_Funtions();
	}
}




void Dispfnc_Set_Rx_Packet_Timer(void){
	Timebase_DownCounter_Set_Forcefully(TIMEBASE_RX_PACKET_WINDOW_S,TIMEBASE_RX_PACKET_TIME_S);
	Disp__com.rx_error_flag = FALSE;
}

void Dispfnc_Get_Rx_Packet_Error(void){
	if(Timebase_DownCounter_Expired_Event(TIMEBASE_RX_PACKET_WINDOW_S)){
		Disp__com.rx_error_flag = TRUE;
	}
}

uint8_t Dispfnc_Get_Rx_Packet_Error_Flag(void){
	return Disp__com.rx_error_flag;
}


void Dispfnc_Tx_Rx_Handler(void){
	if(Timebase_DownCounter_Continuous_Expired_Event(TIMEBASE_HEARTBEAT_WINDOW_S)){
	  Display_TxData_Handler();
	}
	if(DispCom_Data_Available()){
		Display_RxData_Update_Buf();
		DispCom_RX_Packet_Read_Complete();
		Dispfnc_Set_Rx_Packet_Timer();
	}
	Dispfnc_Get_Rx_Packet_Error();
}

void Dispfnc_Display_Repeated_Init_Timer_Init(void){
	Timebase_DownCounter_SS_Set_Forcefully(DISPLAY_DRIVER_INIT_WINDOW_SS, 100);
}

void Dispfnc_Display_Repeated_Init(void){
		if(Timebase_DownCounter_SS_Expired_Event(DISPLAY_DRIVER_INIT_WINDOW_SS)){
			TM1650_Write_System_Cmd(0x48); // Display Init Command
			Dispfnc_Display_Repeated_Init_Timer_Init();
			Debug_Tx_Text("Reset done\n\r");
		}
}



void Dispfnc_Init(void){
	Dispfnc_Structure_Init();
	Timebase_DownCounter_Set_Securely(TIMEBASE_HEARTBEAT_WINDOW_S, TIMEBASE_HEARTBEAT_TIME_S);
	Timebase_DownCounter_SS_Set_Securely(TIMEBASE_DISPLAY_PRINT_WINDOW_SS, TIMEBASE_DISPLAY_PRINT_TIME_S);
	Dispfnc_Display_Repeated_Init_Timer_Init();
}


void Dispfnc_Handler(void){
	Dispfnc_Display_Repeated_Init();
	Debug_Tx_Parameter_NL("packet error flag ", Dispfnc_Get_Rx_Packet_Error_Flag());
	Dispfnc_Display_Print_Funtions();
	Dispfnc_Tx_Rx_Handler();
}
