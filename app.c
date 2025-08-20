#include "stm32g030xx.h"
#include "timebase.h"
#include "dispcom.h"
#include "dispfnc.h"
#include "display.h"
#include "captouch.h"
#include "tm1650.h"
#include "diag.h"
#include "debug.h"


void App_Setup(void){
	Timebase_Init(1000);
	CapTouch_Init();
	TM1650_Init();
	Debug_Init(38400);
	DispCom_Init(9600);
	Display_Init();
	Diag_Init();
	Dispfnc_Init();
}

void App_Main_Loop(void){
	CapTouch_Scan_Sensors(Timebase_Timer_Get_SubSecondsUpTime_Securely());
	Diag_Handler();
	Timebase_Main_Loop_Executables();
	Dispfnc_Handler();
}