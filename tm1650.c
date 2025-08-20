#include "stm32g030xx.h"
//#include "Downcounter.h"
#include "Timebase.h"
#include "cdef.h"


#define  TM1650_SCL_BP       0   
#define  TM1650_SDA_BP       1    

//System Command 01 setting 
#define  TM1650_SYS_CMD_MSK  0xFF

#define  TM1650_SYSTEM_CMD   0x48
#define  TM1650_READ_CMD_KEY 0x49   
//Command 02 Parameter setting 
#define  TM1650_CMD_MODE_MSK 0x0F

#define  TM1650_7_SEG_OUTPUT 0x08
#define  TM1650_MODE_NORM    0x00
#define  TM1650_MODE_STD     0x04
#define  TM1650_DSP_OFF      0x00
#define  TM1650_DSP_ON       0x01
//Address setting commands
#define  TM1650_ADDR_MSK     0xFF

#define  TM1650_ADDR_DISP_01 0x68
#define  TM1650_ADDR_DISP_02 0x6A
#define  TM1650_ADDR_DISP_03 0x6C
#define  TM1650_ADDR_DISP_04 0x6E

//Display control parameter commands 02
#define  TM1650_BRT_MSK      0xF0
#define  TM1650_BRT1         0x10
#define  TM1650_BRT2         0x20
#define  TM1650_BRT3         0x30
#define  TM1650_BRT4         0x40
#define  TM1650_BRT5         0x50
#define  TM1650_BRT6         0x60
#define  TM1650_BRT7         0x70 
#define  TM1650_BRT0         0x00

#define  TM1650_KEY_01_PRESSED_DIG_04  0x47
#define  TM1650_KEY_02_PRESSED_DIG_04  0x4F
#define  TM1650_KEY_03_PRESSED_DIG_04  0x57
#define  TM1650_KEY_04_PRESSED_DIG_04  0x5F

#define  TM1650_KEY_01_RELEASED_DIG_04 0x07
#define  TM1650_KEY_02_RELEASED_DIG_04 0x0F
#define  TM1650_KEY_03_RELEASED_DIG_04 0x17
#define  TM1650_KEY_04_RELEASED_DIG_04 0x1F


///////////////////////////////////////////////////////////////////
#define  TM1650_DELAY_CYCLES           100
#define  TM1650_COMM_STOP_IF_ERROR

typedef struct tm1650_t{
	uint8_t  Error;
	uint8_t  ErrorStep;
	uint8_t  ErrorStateHandled;
	uint16_t ErrorsTotal;
	uint8_t  LastError;
	uint8_t  LastErrorStep;
	uint8_t  SCLState;
	uint8_t  SDAState;
}tm1650_t;

uint8_t TM1650_Numeric_Values[23]={
  0b00111111, //0    0
  0b00000110, //1    1
  0b01011011, //2    2
  0b01001111, //3    3
  0b01100110, //4    4
  0b01101101, //5    5
  0b01111101, //6    6
  0b00000111, //7    7
  0b01111111, //8    8
  0b01101111, //9    9
  0b00000000, //Null 10
  0b01000000, //'-'  11
  0b01110111, //'A'  12
  0b01111100, //'b'  13
  0b00111001, //'C'  14
  0b01011110, //'d'  15
  0b01111001, //'E'  16
  0b01110001, //'F'  17
  0b00111000, //'L'  18
	0b00111110, //'U'  19
	0b01110011, //'P'  20
	0b00000000, //Customized for Dig3  21
	0b00000000, //Customized for Dig4  22
};


tm1650_t TM1650;

void TM1650_Struct_Init(void){
  TM1650.Error = 0;
	TM1650.ErrorStep = 0;
	TM1650.ErrorStateHandled = FALSE;
	TM1650.ErrorsTotal = 0;
	TM1650.LastError = 0;
	TM1650.LastErrorStep = 0;
	TM1650.SCLState = INPUT_STATE;
	TM1650.SDAState = INPUT_STATE;
}

void TM1650_GPIO_Init(void){
	//Enable clock to GPIOA
	if((RCC->IOPENR & RCC_IOPENR_GPIOAEN)!=RCC_IOPENR_GPIOAEN){
		RCC->IOPENR|=RCC_IOPENR_GPIOAEN;
	}

	GPIOA->OTYPER|=GPIO_OTYPER_OT0_Msk;
	GPIOA->OTYPER|=GPIO_OTYPER_OT1_Msk;
	
	GPIOA->MODER&=~GPIO_MODER_MODE0_Msk;
	GPIOA->MODER|= GPIO_MODER_MODE0_0;
	
	GPIOA->MODER&=~GPIO_MODER_MODE1_Msk;
	GPIOA->MODER|= GPIO_MODER_MODE1_0;
	
	GPIOA->ODR  |= (1<<TM1650_SCL_BP);
	GPIOA->ODR  |= (1<<TM1650_SDA_BP);
	
	for(uint32_t i=0;i<160000;i++){__NOP();}
}

void TM1650_Bit_Delay(void){
	for(uint32_t i=0;i<TM1650_DELAY_CYCLES;i++){
	  __NOP();
	}
}

void TM1650_SCL_Input(void){
	if(TM1650.SCLState != INPUT_STATE){
		GPIOA->MODER &=~ GPIO_MODER_MODE0_0;
	  TM1650.SCLState = INPUT_STATE;
		//need to add delay if necessary
		TM1650_Bit_Delay();
  }
}


void TM1650_SCL_Output(void){
	if(TM1650.SCLState != OUTPUT_STATE){
	  GPIOA->MODER |=  GPIO_MODER_MODE0_0;
	  TM1650.SCLState = OUTPUT_STATE;
		//need to add delay if necessary
		TM1650_Bit_Delay();
	}
}


void TM1650_SCL_Set(uint8_t state){
	if(state == LOGIC_HIGH){
		GPIOA->ODR |= (1<<TM1650_SCL_BP);
	}else if(state == LOGIC_LOW){
		GPIOA->ODR &=~(1<<TM1650_SCL_BP);
	}
	TM1650_SCL_Output();
}

uint8_t TM1650_SCL_Get(void){
	TM1650_SCL_Input();
	if(GPIOA->IDR & (1<<TM1650_SCL_BP)){
		return LOGIC_HIGH;
	}else{
		return LOGIC_LOW;
  }
}


void TM1650_SDA_Input(void){
	if(TM1650.SDAState != INPUT_STATE){
	  GPIOA->MODER &=~ GPIO_MODER_MODE1_0;
	  TM1650.SDAState = INPUT_STATE;
		//need to add delay if necessary
		TM1650_Bit_Delay();
	}
}

void TM1650_SDA_Output(void){
	if(TM1650.SDAState != OUTPUT_STATE){
	  GPIOA->MODER|= GPIO_MODER_MODE1_0;
	  TM1650.SDAState = OUTPUT_STATE;
		//need to add delay if necessary
		TM1650_Bit_Delay();
	}
}


void TM1650_SDA_Set(uint8_t state){
	if(state == LOGIC_HIGH){
		GPIOA->ODR  |= (1<<TM1650_SDA_BP);
	}else if(state == LOGIC_LOW){
		GPIOA->ODR  &=~(1<<TM1650_SDA_BP);
	}
	TM1650_SDA_Output();
}

uint8_t TM1650_SDA_Get(void){
	TM1650_SDA_Input();
	if(GPIOA->IDR & (1<<TM1650_SDA_BP)){
		return LOGIC_HIGH;
	}else{
		return LOGIC_LOW;
  }
}


 
uint8_t TM1650_Ack_Get(void){
	uint8_t  ack_sts = FALSE;
	uint32_t ticks = 0;
	#ifdef TM1650_COMM_STOP_IF_ERROR
	if(TM1650.Error == 0x00){
	#endif
	  TM1650_Bit_Delay();
	  TM1650_SCL_Set(LOGIC_HIGH);
	  TM1650_Bit_Delay();
	  if(TM1650_SDA_Get() == LOGIC_LOW){
		  ack_sts = TRUE;
  	}else{
			TM1650.Error = 0x01;
		}
	  TM1650_Bit_Delay();
	  TM1650_SCL_Set(LOGIC_LOW);
	  while(1){
		  if(TM1650_SDA_Get() == LOGIC_HIGH ){ //Idle state
			  TM1650.ErrorStep = 0x00;
			  break;
		  }
		  else {
			  __NOP();
			  ticks++;
			  if(ticks>TM1650_DELAY_CYCLES){
				  //TM1650.Error = 0x02;
				  break;
			  }
		  }
	  }
	#ifdef TM1650_COMM_STOP_IF_ERROR
	}
	#endif
	return ack_sts;
}


uint8_t TM1650_Ack_Get_With_Timeout(void){
	uint8_t  ack_sts = FALSE;
	uint32_t ticks = 0;
	#ifdef TM1650_COMM_STOP_IF_ERROR
	if(TM1650.Error == 0x00){
	#endif
	  TM1650_Bit_Delay();
	  TM1650_SCL_Set(LOGIC_HIGH);
	  TM1650_Bit_Delay();
	  if(TM1650_SDA_Get() == LOGIC_LOW){
		  ack_sts = TRUE;
  	}else{
			TM1650.Error = 0x01;
		}
	  TM1650_Bit_Delay();
	  TM1650_SCL_Set(LOGIC_LOW);
	  while(1){
		  if(TM1650_SDA_Get() == LOGIC_HIGH ){ //Idle state
			  TM1650.ErrorStep = 0x00;
			  break;
		  }
		  else {
			  __NOP();
			  ticks++;
			  if(ticks>TM1650_DELAY_CYCLES){
				  TM1650.Error = 0x02;
				  break;
			  }
		  }
	  }
	#ifdef TM1650_COMM_STOP_IF_ERROR
	}
	#endif
	return ack_sts;
}

void TM1650_Ack_Send(void){
	uint32_t ticks = 0;
	#ifdef TM1650_COMM_STOP_IF_ERROR
	if(TM1650.Error == 0x00){
	#endif
		TM1650_SDA_Set(LOGIC_LOW); 
	  TM1650_Bit_Delay();
	  TM1650_SCL_Set(LOGIC_HIGH);
	  TM1650_Bit_Delay();
	  TM1650_SCL_Set(LOGIC_LOW);
		TM1650_SDA_Set(LOGIC_HIGH); 
	  while(1){
		  if(TM1650_SDA_Get() == LOGIC_HIGH ){ //Idle state
			  TM1650.ErrorStep = 0x00;
			  break;
		  }
		  else {
			  __NOP();
			  ticks++;
			  if(ticks>TM1650_DELAY_CYCLES){
				  //TM1650.Error = 0x02;
				  break;
			  }
		  }
	  }
	#ifdef TM1650_COMM_STOP_IF_ERROR
	}
	#endif
}


void TM1650_Nack_Send(void){
	uint32_t ticks = 0;
	#ifdef TM1650_COMM_STOP_IF_ERROR
	if(TM1650.Error == 0x00){
	#endif
		TM1650_SDA_Set(LOGIC_HIGH);
	  TM1650_Bit_Delay();
	  TM1650_SCL_Set(LOGIC_HIGH);
	  TM1650_Bit_Delay();
	  TM1650_SCL_Set(LOGIC_LOW);
	  while(1){
		  if(TM1650_SDA_Get() == LOGIC_HIGH ){ //Idle state
			  TM1650.ErrorStep = 0x00;
			  break;
		  }
		  else {
			  __NOP();
			  ticks++;
			  if(ticks>TM1650_DELAY_CYCLES){
				  //TM1650.Error = 0x02;
				  break;
			  }
		  }
	  }
	#ifdef TM1650_COMM_STOP_IF_ERROR
	}
	#endif
}

void TM1650_Start(void){
	#ifdef TM1650_COMM_STOP_IF_ERROR
	if(TM1650.Error == 0x00){
	#endif
	  TM1650_SCL_Set(LOGIC_HIGH);
		TM1650_Bit_Delay();
	  TM1650_SDA_Set(LOGIC_HIGH);
	  TM1650_Bit_Delay();
	  TM1650_SDA_Set(LOGIC_LOW);
	  TM1650_Bit_Delay();
	  TM1650_SCL_Set(LOGIC_LOW);
		TM1650_Bit_Delay();
	#ifdef TM1650_COMM_STOP_IF_ERROR
  }
	#endif
}

void TM1650_Stop(void){
	#ifdef TM1650_COMM_STOP_IF_ERROR
	if(TM1650.Error == 0x00){
	#endif
	  TM1650_SDA_Set(LOGIC_LOW);
	  TM1650_Bit_Delay();
	  TM1650_SCL_Set(LOGIC_LOW);
	  TM1650_Bit_Delay();
	  TM1650_SCL_Set(LOGIC_HIGH);
	  TM1650_Bit_Delay();
	  TM1650_SDA_Set(LOGIC_HIGH);
	  TM1650_Bit_Delay();
	#ifdef TM1650_COMM_STOP_IF_ERROR
	}
	#endif
}


void TM1650_Tx_Byte(uint8_t data){
	uint8_t i;
	#ifdef TM1650_COMM_STOP_IF_ERROR
	if(TM1650.Error == 0x00){
	#endif
	  for(i=0;i<8;i++){
	    TM1650_SDA_Set( (data & 0x80)? LOGIC_HIGH : LOGIC_LOW);
		  TM1650_Bit_Delay();
		  TM1650_SCL_Set(LOGIC_HIGH);
		  TM1650_Bit_Delay();
		  TM1650_SCL_Set(LOGIC_LOW);
		  TM1650_Bit_Delay();
		  data = data<<1;
	  }
	#ifdef TM1650_COMM_STOP_IF_ERROR
  }
	#endif
}

uint8_t TM1650_Rx_Byte(void){
	uint8_t i, data = 0;
	#ifdef TM1650_COMM_STOP_IF_ERROR
	if(TM1650.Error == 0x00){
	#endif
	  for(i=0;i<8;i++){
	    TM1650_SCL_Set(LOGIC_HIGH);
		  TM1650_Bit_Delay();
			data <<= 1;
			data |= TM1650_SDA_Get();
		  TM1650_Bit_Delay();
		  TM1650_SCL_Set(LOGIC_LOW);
		  TM1650_Bit_Delay();
		}
	#ifdef TM1650_COMM_STOP_IF_ERROR
	}
	#endif
	return data;
}


void TM1650_Clear_Error(void){
	TM1650.Error = 0x00;
	TM1650.ErrorStep = 0x00;
	TM1650.ErrorStateHandled = FALSE;
}

void TM1650_Error_Handler(void){
	if(TM1650.ErrorStateHandled == FALSE){
		//Force Stop CMD
		TM1650_SDA_Set(LOGIC_LOW);
	  TM1650_Bit_Delay();
	  TM1650_SCL_Set(LOGIC_LOW);
	  TM1650_Bit_Delay();
	  TM1650_SCL_Set(LOGIC_HIGH);
	  TM1650_Bit_Delay();
	  TM1650_SDA_Set(LOGIC_HIGH);
	  TM1650_Bit_Delay();
		TM1650.ErrorStateHandled = TRUE;
	}
}

void TM1650_Error_Handled_Clear_Errors(void){
	if(TM1650.ErrorStateHandled == TRUE){
		if(TM1650.Error != 0x00){
			TM1650.LastError = TM1650.Error;
			TM1650.LastErrorStep = TM1650.ErrorStep;
			TM1650.ErrorsTotal++;
		}
    TM1650_Clear_Error();
	}
}

void TM1650_Write(uint8_t data){
	TM1650_Tx_Byte(data);
	TM1650_Ack_Get_With_Timeout();
}
 
uint8_t TM1650_Read(void){
	uint8_t data = 0;
	data = TM1650_Rx_Byte();
  TM1650_Ack_Send();
	return data;
}

void TM1650_Write_Cmd_Sevenseg(uint8_t cmd, uint8_t cmd2, uint8_t brt){
	TM1650_Start();
	TM1650_Write((cmd & TM1650_SYS_CMD_MSK));
	TM1650_Write((cmd2 & TM1650_CMD_MODE_MSK)|(brt & TM1650_BRT_MSK));
	TM1650_Stop();
 }

 void TM1650_Write_System_Cmd(uint8_t cmd){
	TM1650_Start();
	TM1650_Write((cmd & TM1650_SYS_CMD_MSK));
	TM1650_Stop();
 }
 
void TM1650_Write_Data(uint8_t address, uint8_t data){
	TM1650_Start();
	TM1650_Write(address & TM1650_ADDR_MSK);
	TM1650_Write(data);
	TM1650_Stop();
 }

uint8_t TM1650_Read_Key_Cmd(void){
	uint8_t key;
	TM1650_Start();
	TM1650_Tx_Byte(0x4F);
  TM1650_Ack_Get();
	key=TM1650_Rx_Byte();
	TM1650_Ack_Send();
	TM1650_Stop();
	return key;
}

void TM1650_Clear_Display(void){
  for (uint8_t i=104;i<=110;i+=2){
		TM1650_Write_Data(i,0);
	}
}

uint8_t TM1650_Get_ErrorCode(void){
	return TM1650.Error;
}

uint8_t TM1650_Get_LastErrorCode(void){
	return TM1650.LastError;
}

uint8_t TM1650_Get_ErrorStep(void){
	return TM1650.ErrorStep;
}

uint8_t TM1650_Get_LastErrorStep(void){
	return TM1650.LastErrorStep;
}

uint8_t TM1650_Get_ErrorStepHandle(void){
	return TM1650.ErrorStateHandled;
}

uint16_t TM1650_Get_ErrorsTotal(void){
	return TM1650.ErrorsTotal;
}





void TM1650_Init(void){
	TM1650_GPIO_Init();
	TM1650_Write_Cmd_Sevenseg(TM1650_SYSTEM_CMD,TM1650_DSP_ON,TM1650_BRT7);
	TM1650_Clear_Display();
}

 
void TM1650_Display_All_Led_ON(void){
	TM1650_Write_Data(TM1650_ADDR_DISP_01, 0xFF);
  TM1650_Write_Data(TM1650_ADDR_DISP_02, 0xFF);
	TM1650_Write_Data(TM1650_ADDR_DISP_03, 0xFF);
	TM1650_Write_Data(TM1650_ADDR_DISP_04, 0xFF);
}

void TM1650_Display_All_Led_OFF(void){
	TM1650_Write_Data(TM1650_ADDR_DISP_01, 0x00);
	TM1650_Write_Data(TM1650_ADDR_DISP_02, 0x00);
	TM1650_Write_Data(TM1650_ADDR_DISP_03, 0x00);
  TM1650_Write_Data(TM1650_ADDR_DISP_04, 0x00);
}


void TM1650_Display_Deem_Smoothly(void){
	for(uint8_t i=112;i>=16;i--){		 
		TM1650_Write_Cmd_Sevenseg(TM1650_SYSTEM_CMD,TM1650_DSP_ON,i);
	}
}

void TM1650_Display_Fully_Bright(void){ 
	TM1650_Write_Cmd_Sevenseg(TM1650_SYSTEM_CMD,TM1650_DSP_ON,112);
}
 
void TM1650_Display_Off_CMD(void){
	TM1650_Write_Cmd_Sevenseg(TM1650_SYSTEM_CMD,TM1650_DSP_OFF,TM1650_BRT0);
}
 
void TM1650_Display_On_CMD(void){
	TM1650_Write_Cmd_Sevenseg(TM1650_SYSTEM_CMD,TM1650_DSP_ON,TM1650_BRT7);
}
 					
void TM1650_Display_Write_Dig1(uint8_t val){
	TM1650_Write_Data(TM1650_ADDR_DISP_01, val);
}

void TM1650_Display_Write_Dig2(uint8_t val){
	TM1650_Write_Data(TM1650_ADDR_DISP_02, val);
}

void TM1650_Display_Write_Dig3(uint8_t val){
	TM1650_Write_Data(TM1650_ADDR_DISP_03, val);
}
	
void TM1650_Display_Write_Dig4(uint8_t val){
	TM1650_Write_Data(TM1650_ADDR_DISP_04, val);
}



//LED Operations For LED1-LED8

void TM1650_Display_Set_State_LED1_LED8(uint8_t pos, uint8_t val){
	//Omit check if already
	if( (pos >=1) && (pos <= 8) ){
		if(val == ON){
			pos -= 1;
			TM1650_Numeric_Values[21] |= (1<< pos);
		}
		else if(val == OFF){
			pos -= 1;
			TM1650_Numeric_Values[21] &=~ (1<< pos);
		}
		else{
			//Report Error, Undefined state
		}
	}
	else{
		//Report Error
		//Set LED index number out of boundary
	}
	
	//Check For errors
	//push variable
	if(TM1650.Error == NULL){
	  TM1650_Display_Write_Dig3(TM1650_Numeric_Values[21]);
	}
}

uint8_t TM1650_Display_Get_State_LED1_LED8(uint8_t pos){
	//Omit check if already
	if( (pos >=1) && (pos <= 8) ){
		pos -= 1;
	  if( TM1650_Numeric_Values[21] & (1<<pos)){
			return ON;
		}
		else{
			return OFF;
		}
	}
	else{
		//Report Error
	}
	return 0xFF;
}

void TM1650_Display_Toggle_LED1_LED8(uint8_t pos){
	if( TM1650_Display_Get_State_LED1_LED8(pos) == ON){
		TM1650_Display_Set_State_LED1_LED8(pos, OFF);
	}
	else{
		TM1650_Display_Set_State_LED1_LED8(pos, ON);
	}
}




//LED Operations For LED9-LED10

void TM1650_Display_Set_State_LED9_LED10(uint8_t pos, uint8_t val){
	//Omit check if already
	if( (pos >=9) && (pos <= 10) ){
		if(val == ON){
			pos -= 9;
			TM1650_Numeric_Values[22] |= (1<< pos);
		}
		else if(val == OFF){
			pos -= 9;
			TM1650_Numeric_Values[22] &=~ (1<< pos);
		}
		else{
			//Report Error, Undefined state
		}
	}
	else{
		//Report Error
		//Set LED index number out of boundary
	}
	
	//Check For errors
	//push variable
	if(TM1650.Error == NULL){
	  TM1650_Display_Write_Dig4(TM1650_Numeric_Values[22]);
	}
}

uint8_t TM1650_Display_Get_State_LED9_LED10(uint8_t pos){
	//Omit check if already
	if( (pos >=9) && (pos <= 10) ){
		pos -= 9;
	  if( TM1650_Numeric_Values[22] & (1<<pos)){
			return ON;
		}
		else{
			return OFF;
		}
	}
	else{
		//Report Error
	}
	return 0xFF;
}


void TM1650_Display_Toggle_LED9_LED10(uint8_t pos){
	if( TM1650_Display_Get_State_LED9_LED10(pos) == ON){
		TM1650_Display_Set_State_LED9_LED10(pos, OFF);
	}
	else{
		TM1650_Display_Set_State_LED9_LED10(pos, ON);
	}
}





void TM1650_Display_Set_State_LED(uint8_t pos, uint8_t val){
	if( (pos>= 1) && (pos <= 8) ){
		TM1650_Display_Set_State_LED1_LED8(pos, val);
	}
	else if((pos>= 9) && (pos <= 10)){
		TM1650_Display_Set_State_LED9_LED10(pos, val);
	}
	else{
		//Report Error
	}
}

uint8_t TM1650_Display_Get_State_LED(uint8_t pos){
	if( (pos>= 1) && (pos <= 8) ){
		return TM1650_Display_Get_State_LED1_LED8(pos);
	}
	else if((pos>= 9) && (pos <= 10)){
		return TM1650_Display_Get_State_LED9_LED10(pos);
	}
	else{
		//Report Error
	}
	return 0xFF;
}

void TM1650_Display_Toggle_LED(uint8_t pos){
	if( TM1650_Display_Get_State_LED(pos) == ON){
		TM1650_Display_Set_State_LED(pos, OFF);
	}
	else{
		TM1650_Display_Set_State_LED(pos, ON);
	}
}


void TM1650_Display_Print_Num_Unsigned(int8_t val){
	uint8_t temp0, temp1;
	if( (val >= 0) && (val <= 99) ){
		temp0 = val / 10;
		temp1 = val % 10;
		TM1650_Display_Write_Dig1(TM1650_Numeric_Values[temp0]);
		TM1650_Display_Write_Dig2(TM1650_Numeric_Values[temp1]);
	}
	else{
		//Report error that number is beyond limits
	}
}

void TM1650_Display_Print_CL(void){
	TM1650_Display_Write_Dig1(TM1650_Numeric_Values[14]);
	TM1650_Display_Write_Dig2(TM1650_Numeric_Values[18]);
}

void TM1650_Display_Print_UL(void){
	TM1650_Display_Write_Dig1(TM1650_Numeric_Values[19]);
	TM1650_Display_Write_Dig2(TM1650_Numeric_Values[18]);
}

void TM1650_Display_Print_CE(void){
	TM1650_Display_Write_Dig1(TM1650_Numeric_Values[14]);
	TM1650_Display_Write_Dig2(TM1650_Numeric_Values[16]);
}

	
	
	
	
	
	
	
	
	