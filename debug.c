



#include "stm32g030xx.h"
#include "debug.h"


/*******************Structure & Enumeration Start*****************/

typedef struct uart_t{
  volatile uint8_t   Error;
  volatile uint8_t   Buf[DEBUG_RX_BUF_SIZE];
  volatile uint8_t   BufIndex;
  uint8_t            Digits[8];
  uint8_t            InputNumDigits;
	volatile uint8_t   TimerEnabled;
	volatile uint8_t   DataAvailable;
}uart_t;

uart_t Debug;

/********************Structure & Enumeration End******************/









/***********************Init Functions Start*********************/

void Debug_Struct_Init(void){
  Debug.Error=0;
  for(uint8_t i=0;i<DEBUG_RX_BUF_SIZE;i++){
    Debug.Buf[i]=0;
  }
  Debug.BufIndex=0;
  for(uint8_t i=0;i<8;i++){
    Debug.Digits[i] = 0;
  }
  Debug.InputNumDigits = 0;
	Debug.TimerEnabled = 0;
	Debug.DataAvailable = 0;
}

void Debug_GPIO_Init(void){
	//PA2->TX, PA3->RX
	if( (RCC->IOPENR & RCC_IOPENR_GPIOAEN)!= RCC_IOPENR_GPIOAEN ){
		RCC->IOPENR |= RCC_IOPENR_GPIOAEN;
	}
	
	#ifdef DEBUG_TX_ENABLE
	//Config PA2 as alternate Func
	GPIOA->MODER &=~ GPIO_MODER_MODE2_Msk;
	GPIOA->MODER |=  GPIO_MODER_MODE2_1;
	
	//Enable Pull-up in PA2
	GPIOA->PUPDR &=~ GPIO_PUPDR_PUPD2_Msk;
	GPIOA->PUPDR |=  GPIO_PUPDR_PUPD2_0;
	
	//Select TX in AFR
	GPIOA->AFR[0]&=~ GPIO_AFRL_AFSEL2_Msk;
	GPIOA->AFR[0]|=  (1<<GPIO_AFRL_AFSEL2_Pos);
	#endif
	
	#ifdef DEBUG_RX_ENABLE
	//Config PA3 as alternate Func
	GPIOA->MODER &=~ GPIO_MODER_MODE3_Msk;
	GPIOA->MODER |=  GPIO_MODER_MODE3_1;
	
	//Enable Pull-up in PA3
	GPIOA->PUPDR &=~ GPIO_PUPDR_PUPD3_Msk;
	GPIOA->PUPDR |=  GPIO_PUPDR_PUPD3_0;
	
	//Select RX in AFR
	GPIOA->AFR[0]&=~ GPIO_AFRL_AFSEL3_Msk;
	GPIOA->AFR[0]|=  (1<<GPIO_AFRL_AFSEL3_Pos);
	#endif
}

void Debug_Reg_Init(uint32_t baud_rate){
	//Debug -> UART2
	RCC->APBENR1 |= RCC_APBENR1_USART2EN;
	USART2->BRR   = (uint16_t)(16000000/baud_rate);
	
	#ifdef DEBUG_TX_ENABLE
	USART2->CR1  |= USART_CR1_TE;
	#endif
	
	#ifdef DEBUG_RX_ENABLE
	USART2->CR1  |= USART_CR1_RE;
	//FIFO disable
	USART2->CR1 &=~USART_CR1_FIFOEN;
	USART2->CR1 |= USART_CR1_RXNEIE_RXFNEIE;
	//add interrupt
	NVIC_EnableIRQ(USART2_IRQn);
	NVIC_SetPriority(USART2_IRQn, 2);
	#endif
	
	USART2->CR1  |= USART_CR1_UE;
}


//Dedicated Timer for Data Packet End Detection
void Debug_Timer_Init(void){
	if( (RCC->APBENR2 & RCC_APBENR2_TIM14EN) != RCC_APBENR2_TIM14EN){
		RCC->APBENR2 |= RCC_APBENR2_TIM14EN;
	}
	TIM14->CR1 |= TIM_CR1_ARPE;
	//Prescaler 16000, to get 1ms
	TIM14->PSC  = 15999;
	//TIM16->ARR  = 0xFFFF;
	TIM14->ARR  = DEBUG_PCKT_COMP_DELAY;
	TIM14->DIER|= TIM_DIER_UIE;
	NVIC_EnableIRQ(TIM14_IRQn);
	NVIC_SetPriority(TIM14_IRQn, 2);
	
}

void Debug_Timer_Enable(void){
	TIM14->CR1 |= TIM_CR1_CEN;
	Debug.TimerEnabled = 1;
}

void Debug_Timer_Disable(void){
	TIM14->CR1 &=~ TIM_CR1_CEN;
	Debug.TimerEnabled = 0;
}

uint8_t Debug_Get_Timer_Status(void){
	return Debug.TimerEnabled;
}

uint16_t Debug_Get_Counter_Val(void){
	return TIM14->CNT;
}


void Debug_Counter_Reset(void){
	TIM14->CNT = 0;
}


void TIM14_IRQHandler(void){
	if(Debug.TimerEnabled == 1){
	  Debug_Timer_Disable();
	}
	if(Debug_Get_Buf_Index() > 0){
		Debug.DataAvailable = 1;
	}
	else{
		Debug.DataAvailable = 0;
	}
	TIM14->SR &=~ TIM_SR_UIF;
}

/***********************Init Functions Start*********************/









/**********************Basic Functions Start*********************/

void Debug_Tx_Byte(uint8_t val){
	USART2->TDR = val;
	while((USART2->ISR & USART_ISR_TC) != USART_ISR_TC){
		//Add Timeout functions
	}
	USART2->ICR |= USART_ICR_TCCF;
}



uint8_t Debug_Rx_Byte(void){
  volatile uint32_t temp = USART2->RDR;
	return (uint8_t)temp;
}

void USART2_IRQHandler(void){
	if(USART2->ISR & USART_ISR_RXNE_RXFNE){
	  Debug.Buf[Debug.BufIndex] = Debug_Rx_Byte();
		Debug.BufIndex++;
		if(Debug.BufIndex >= DEBUG_RX_BUF_SIZE){
			Debug.BufIndex = 0;
		}
		
		
		if(Debug.TimerEnabled == 0){
			Debug_Counter_Reset();
			Debug_Timer_Enable();
		}
		Debug_Counter_Reset();
	}
}

/***********************Basic Functions End**********************/









/********************Buffer Tx Functions Start*******************/

void Debug_Tx_Buf(uint8_t *data, uint8_t len){
	for(uint16_t i=0; i<len; i++){
		Debug_Tx_Byte( data[i] );
	}
}

/*********************Buffer Tx Functions End********************/









/*******************End Char Functions Start******************/

void Debug_Tx_NL(void){
  Debug_Tx_Byte('\r');
  Debug_Tx_Byte('\n');
}

void Debug_Tx_SP(void){
  Debug_Tx_Byte(' ');
}

void Debug_Tx_CM(void){
  Debug_Tx_Byte(',');
}

/*******************End Char Functions End*******************/









/*********************Text Functions Start*******************/

void Debug_Tx_Text(char *str){
    uint8_t i=0;
    while(str[i]!='\0'){
        Debug_Tx_Byte(str[i]);
        i++;
    }
}

void Debug_Tx_Text_NL(char *str){
  Debug_Tx_Text(str);
  Debug_Tx_NL();
}

void Debug_Tx_Text_SP(char *str){
  Debug_Tx_Text(str);
  Debug_Tx_SP();
}

void Debug_Tx_Text_CM(char *str){
  Debug_Tx_Text(str);
  Debug_Tx_CM();
}

/*********************Text Functions End********************/









/*********************Number Functions Start********************/

void Debug_Determine_Digit_Numbers(uint32_t num){
  uint8_t i=0;
  if(num==0){
    Debug.Digits[0]=0;
    Debug.InputNumDigits=1;
  }else{
    while(num!=0){
      Debug.Digits[i]=num%10;
      num/=10;
      i++;
    }
	Debug.InputNumDigits=i;
  }
}

void Debug_Tx_Number_Digits(void){
  for(uint8_t i=Debug.InputNumDigits; i>0; i--){
    uint8_t temp=i;
    temp-=1;
    temp=Debug.Digits[temp];
    temp+=48;
    Debug_Tx_Byte(temp);
  }
}

void Debug_Tx_Number(int32_t num){
  if(num<0){
    Debug_Tx_Byte('-');
	  num=-num;
  }
  Debug_Determine_Digit_Numbers((uint32_t)num);
  Debug_Tx_Number_Digits();
}

void Debug_Tx_Number_Hex(uint32_t val){
  uint16_t hex_digit, index=0, loop_counter=0;
  if(val <= 0xFF){
    index=8;
    loop_counter=2;
  }else if(val <= 0xFFFF){
    index=16;
    loop_counter=4;     
  }else{
    index=32;
    loop_counter=8;
  }
  Debug_Tx_Byte('0');
  Debug_Tx_Byte('x');
	for(uint8_t i=0;i<loop_counter;i++){
	  index-=4;
	  hex_digit=(uint8_t)((val>>index) & 0x0F);
	  if(hex_digit>9){
	    hex_digit+=55;
	  }else {
	    hex_digit+=48;
	  }
	  Debug_Tx_Byte((uint8_t)hex_digit);
	}
}

void Debug_Tx_Number_Bin(uint32_t val){
  uint8_t loop_counter=0;
  if(val <= 0xFF){
    loop_counter=7;
  }else if(val <= 0xFFFF){
    loop_counter=15;     
  }else{
    loop_counter=31;
  }
  
  Debug_Tx_Byte('0');
  Debug_Tx_Byte('b');
  for(int i=loop_counter; i>=0; i--){
    if( (val>>i) & 1){
      Debug_Tx_Byte( 49 );   
    }else{
      Debug_Tx_Byte( 48 );         
    }
  }
}

/*********************Number Functions End*********************/









/************Number with End Char Functions Start**************/

void Debug_Tx_Number_NL(int32_t num){
  Debug_Tx_Number(num);
  Debug_Tx_NL();
}

void Debug_Tx_Number_SP(int32_t num){
  Debug_Tx_Number(num);
  Debug_Tx_SP();
}

void Debug_Tx_Number_CM(int32_t num){
  Debug_Tx_Number(num);
  Debug_Tx_CM();
}

/*************Number with End Char Functions End***************/









/**********Hex Number with End Char Functions Start************/

void Debug_Tx_Number_Hex_NL(int32_t num){
  Debug_Tx_Number_Hex(num);
  Debug_Tx_NL();
}

void Debug_Tx_Number_Hex_SP(int32_t num){
  Debug_Tx_Number_Hex(num);
  Debug_Tx_SP();
}

void Debug_Tx_Number_Hex_CM(int32_t num){
  Debug_Tx_Number_Hex(num);
  Debug_Tx_CM();
}

/***********Hex Number with End Char Functions End*************/









/**********Bin Number with End Char Functions Start************/

void Debug_Tx_Number_Bin_NL(int32_t num){
  Debug_Tx_Number_Bin(num);
  Debug_Tx_NL();
}

void Debug_Tx_Number_Bin_SP(int32_t num){
  Debug_Tx_Number_Bin(num);
  Debug_Tx_SP();
}

void Debug_Tx_Number_Bin_CM(int32_t num){
  Debug_Tx_Number_Bin(num);
  Debug_Tx_CM();
}

/***********Bin Number with End Char Functions End*************/







/************Number with Parameter Functions Start*************/

void Debug_Tx_Parameter_NL(char *name, int32_t num){
  Debug_Tx_Text(name);
  Debug_Tx_SP();
  Debug_Tx_Number_NL(num);
}

void Debug_Tx_Parameter_SP(char *name, int32_t num){
  Debug_Tx_Text(name);
  Debug_Tx_SP();
  Debug_Tx_Number_SP(num);
}

void Debug_Tx_Parameter_CM(char *name, int32_t num){
  Debug_Tx_Text(name);
  Debug_Tx_SP();
  Debug_Tx_Number_CM(num);
}

/*************Number with Parameter Functions End**************/









/**********Hex Number with Parameter Functions Start***********/

void Debug_Tx_Parameter_Hex_NL(char *name, int32_t num){
  Debug_Tx_Text(name);
  Debug_Tx_SP();
  Debug_Tx_Number_Hex_NL(num);
}

void Debug_Tx_Parameter_Hex_SP(char *name, int32_t num){
  Debug_Tx_Text(name);
  Debug_Tx_SP();
  Debug_Tx_Number_Hex_SP(num);
}

void Debug_Tx_Parameter_Hex_CM(char *name, int32_t num){
  Debug_Tx_Text(name);
  Debug_Tx_SP();
  Debug_Tx_Number_Hex_CM(num);
}

/***********Hex Number with Parameter Functions End************/









/**********Bin Number with Parameter Functions Start***********/

void Debug_Tx_Parameter_Bin_NL(char *name, int32_t num){
  Debug_Tx_Text(name);
  Debug_Tx_SP();
  Debug_Tx_Number_Bin_NL(num);
}

void Debug_Tx_Parameter_Bin_SP(char *name, int32_t num){
  Debug_Tx_Text(name);
  Debug_Tx_SP();
  Debug_Tx_Number_Bin_SP(num);
}

void Debug_Tx_Parameter_Bin_CM(char *name, int32_t num){
  Debug_Tx_Text(name);
  Debug_Tx_SP();
  Debug_Tx_Number_Bin_CM(num);
}

/***********Bin Number with Parameter Functions End************/








//Receiver Functions

void Debug_Flush_Buf(void){
	for(uint8_t i=0;i<DEBUG_RX_BUF_SIZE;i++){
	  Debug.Buf[i] = 0;
	}
	Debug.BufIndex = 0;
}

uint8_t Debug_Get_Buf(uint16_t index){
	return Debug.Buf[index];
}

uint8_t Debug_Get_Buf_Index(void){
	return Debug.BufIndex;
}

uint8_t Debug_Get_Data_Available_Flag(void){
	return Debug.DataAvailable;
}




//Data Availablity chek using timer
uint8_t Debug_Data_Available(void){
	return Debug.DataAvailable;
}

void Debug_Clear_Data_Available_Flag(void){
	Debug.DataAvailable = 0;
}


uint8_t Debug_Get_Data_Len(void){
	return Debug_Get_Buf_Index();
}



void Debug_Data_Copy_Buf(uint8_t *buf){
	for(uint16_t i=0;i<Debug_Get_Data_Len();i++){
		buf[i] = Debug_Get_Buf(i);
	}
}


void Debug_Print_Buf_Data(void){
	if(Debug_Data_Available()){
	  for(uint16_t i=0; i<Debug_Get_Data_Len(); i++){
		  Debug_Tx_Number_Hex( Debug_Get_Buf(i) );
	  }
  }
	Debug_Tx_NL();
}




void Debug_Init(uint32_t baud_rate){
	Debug_Struct_Init();
	Debug_GPIO_Init();
	Debug_Reg_Init(baud_rate);
	Debug_Timer_Init();
}