
/*
 * File:   DispCom.c
 * Author: MD. Faridul Islam
 * faridmdislam@gmail.com
 * LL Driver -> Display->DispCom Library
 * Created on December 12, 2024, 9:44 PM
 */


//add includes according to hardware

#include "stm32g030xx.h"
//must be included
#include "dispcom.h"


#define  DISPCOM_DOUBLE_SPEED
#define  DISPCOM_ENABLE_TX    
#define  DISPCOM_ENABLE_RX    
#define  DISPCOM_ENABLE_RX_INT

#define  DISPCOM_BUFFER_SIZE            64U
#define  DISPCOM_RX_PCKT_CMPLT_DELAY    25U



#define  DISPCOM_CRC_ENABLE     //Uncomment if packet validation by CRC is needed
#define  DISPCOM_CRC_XMODEM     //Uncomment if CRC is by X-MODEM Protocol
//#define  DISPCOM_CRC_ALTERNATE     //Uncomment if CRC is by Supplier Protocol




//Define Software Error Codes
#define  DISPCOM_RX_ERR_NO_ERR          0x00U
#define  DISPCOM_RX_ERR_FRAMING         0x01U
#define  DISPCOM_RX_ERR_OVERRUN         0x02U
#define  DISPCOM_RX_ERR_READ_INCOMPLETE 0x10U


typedef struct dispcom_timer_t{
  volatile uint8_t   Enabled;
  volatile uint8_t   ResetVal;
}dispcom_timer_t;

typedef struct dispcom_rx_packet_t{
  volatile uint16_t  CalculatedCRC;
  volatile uint16_t  ReceivedCRC;
  volatile uint8_t   CRCStatus;
  volatile uint8_t   DataAvailable;
  volatile uint8_t   DataReadComplete;
	volatile uint8_t   Reserved;
}dispcom_rx_packet_t;

typedef struct dispcom_t{
  volatile uint8_t   Error;
  uint8_t            Digits[8];
  uint8_t            InputNumDigits;

  volatile uint8_t   LastRxByte;
  volatile uint8_t   Buf[DISPCOM_BUFFER_SIZE];
	volatile uint8_t   Reserved;
  volatile uint16_t  BufSize;
  volatile uint16_t  BufIndex;
  
  dispcom_timer_t       Timer;
  
  dispcom_rx_packet_t   RxPacket;
}dispcom_t;


enum{
  DISPCOM_FALSE = 0,
  DISPCOM_TRUE  = 1,
  DISPCOM_NULL  = 0
};


static dispcom_t DispCom;






/*******************DispCom Structure Functions Start****************/

void DispCom_Struct_Init(void){
  DispCom.Error = 0;
  for(uint8_t i = 0; i < 8; i++){
    DispCom.Digits[i] = DISPCOM_NULL;
  }
  DispCom.InputNumDigits = DISPCOM_NULL;
  DispCom.LastRxByte = DISPCOM_NULL;
  DispCom.BufSize = DISPCOM_BUFFER_SIZE;
  DispCom.BufIndex = 0;
  for(uint8_t i = 0; i < DispCom.BufSize; i++){
    DispCom.Buf[i] = DISPCOM_NULL;
  }
}

void DispCom_RX_Packet_Struct_Init(void){
  DispCom.RxPacket.CalculatedCRC    = DISPCOM_NULL;
  DispCom.RxPacket.ReceivedCRC      = DISPCOM_NULL;
  DispCom.RxPacket.CRCStatus        = DISPCOM_FALSE;
  DispCom.RxPacket.DataAvailable    = DISPCOM_FALSE;
  DispCom.RxPacket.DataReadComplete = DISPCOM_FALSE;
}

/********************UART Structure Functions End*****************/









/*********************DispCom Init Functions Start******************/

void DispCom_Config_GPIO(void){
	if((RCC->IOPENR & RCC_IOPENR_GPIOBEN) != RCC_IOPENR_GPIOBEN){
		RCC->IOPENR |= RCC_IOPENR_GPIOBEN;
	}
	//PB6 alternate function, USART1->TX
	GPIOB->MODER  &=~GPIO_MODER_MODE6_Msk;
	GPIOB->MODER  |= GPIO_MODER_MODE6_1;
	
	//PB7 alternate function, USART1->RX
	GPIOB->MODER  &=~GPIO_MODER_MODE7_Msk;
	GPIOB->MODER  |= GPIO_MODER_MODE7_1;
}

void DispCom_Config_Clock(void){
  RCC->APBENR2  |= RCC_APBENR2_USART1EN; 
}

void DispCom_Config_BAUD_Rate(uint32_t baud_rate){
	if(USART1->CR1 & USART_CR1_UE){
    USART1->CR1 &=~USART_CR1_UE;
	}
  USART1->BRR=(uint16_t)(16000000/baud_rate);
}


void DispCom_Config_Tx(void){
	USART1->CR1   |= USART_CR1_TE;
	if((USART1->CR1 & USART_CR1_UE) != USART_CR1_UE){
	  USART1->CR1 |= USART_CR1_UE;
	}
}


void DispCom_Config_Rx(void){
  USART1->CR1   |= USART_CR1_RE;
	if((USART1->CR1 & USART_CR1_UE) != USART_CR1_UE){
	  USART1->CR1 |= USART_CR1_UE;
	}
}

void DispCom_Config_Rx_Interrupt(void){
  USART1->CR1  |= USART_CR1_RXNEIE_RXFNEIE;
	NVIC_EnableIRQ(USART1_IRQn);
	NVIC_SetPriority(USART1_IRQn, 0);
}

void DispCom_Clear_Interrupt_Flag(void){
  //Not necessary, auto cleared when RXDR read
}

void DispCom_Tx_Byte(uint8_t val){
  USART1->TDR=val;
	while((USART1->ISR & USART_ISR_TC)!=USART_ISR_TC);
	USART1->ICR|=USART_ICR_TCCF;	                                                                                                 
}

uint8_t DispCom_Rx_Byte(void){
  volatile uint8_t val = 0;
  val = (uint8_t)USART1->RDR;
  return val;
}

//add DispCom int handler vector
//call DispCom_ISR_Handler() inside ISR

void USART1_IRQHandler(void){
	DispCom_ISR_Handler();
}

/**********************DispCom Init Functions End*******************/









/********************DispCom Timer Functions Start*****************/

void DispCom_Timer_Struct_Init(void){
  DispCom.Timer.Enabled = DISPCOM_FALSE;
  DispCom.Timer.ResetVal = DISPCOM_NULL;
}

void DispCom_Timer_Init(void){
  //config DispCom timer for auto packet validation
	//calculate DispCom.Timer.ResetVal if overflow int is used
	//timer resolution should be 1ms
	//if other than 1ms is used, adjust DISPCOM_RX_PCKT_CMPLT_DELAY
	//TIM14 for Timebase
	RCC->APBENR2 |= RCC_APBENR2_TIM16EN;
	TIM16->PSC    = 15999;
	TIM16->ARR    = DISPCOM_RX_PCKT_CMPLT_DELAY;
	TIM16->DIER  |= TIM_DIER_UIE;
	NVIC_EnableIRQ(TIM16_IRQn);
	NVIC_SetPriority(TIM16_IRQn, 2);
  #if DISPCOM_RX_PCKT_CMPLT_DELAY<20U
    #warning DISPCOM_RX_PCKT_CMPLT_DELAY value < 20
  #endif
}

void DispCom_Timer_Enable(void){
  TIM16->CR1   |= TIM_CR1_CEN;
}

void DispCom_Timer_Disable(void){
  TIM16->CR1   &=~TIM_CR1_CEN;
}

uint8_t DispCom_Timer_Get_Status(void){
  return DispCom.Timer.Enabled;
}

uint16_t DispCom_Timer_Get_Val(void){
  //return current timer val
	return TIM16->CNT;
}


void DispCom_Timer_Value_Reset(void){
	//reset timer val if compare mode selected
	TIM16->CNT = 0;
	//set timer val to DispCom.Timer.ResetVal if Overflow int is used
}

void DispCom_Timer_Clear_Interrupt_Flag(void){
  //Clear flag if 
	TIM16->SR &=~ TIM_SR_CC1IF;
	TIM16->SR &=~ TIM_SR_UIF;
}

//add DispCom timer int handler vector
//call DispCom_Timer_ISR_Handler() inside ISR

void TIM16_IRQHandler(void){
	DispCom_Timer_ISR_Handler();	
}


/*********************DispCom Timer Functions End******************/









/********************Buffer Tx Functions Start*******************/

void DispCom_Tx_Buf(uint8_t *data, uint8_t len){
  for(uint16_t i = 0; i < len; i++){
	DispCom_Tx_Byte( data[i] );
  }
}

/*********************Buffer Tx Functions End********************/









/*******************End Char Functions Start******************/

void DispCom_Tx_NL(void){
  DispCom_Tx_Byte('\r');
  DispCom_Tx_Byte('\n');
}

void DispCom_Tx_SP(void){
  DispCom_Tx_Byte(' ');
}

void DispCom_Tx_CM(void){
  DispCom_Tx_Byte(',');
}

/*******************End Char Functions End*******************/









/*********************Text Functions Start*******************/

void DispCom_Tx_Text(char *str){
  uint8_t i = 0;
  while(str[i] != '\0'){
    DispCom_Tx_Byte(str[i]);
    i++;
  }
}

void DispCom_Tx_Text_NL(char *str){
  DispCom_Tx_Text(str);
  DispCom_Tx_NL();
}

void DispCom_Tx_Text_SP(char *str){
  DispCom_Tx_Text(str);
  DispCom_Tx_SP();
}

void DispCom_Tx_Text_CM(char *str){
  DispCom_Tx_Text(str);
  DispCom_Tx_CM();
}

/*********************Text Functions End********************/









/*********************Number Functions Start********************/

void DispCom_Determine_Digit_Numbers(uint32_t num){
  uint8_t i = 0;
  if(num == 0){
    DispCom.Digits[0] = 0;
    DispCom.InputNumDigits = 1;
  }else{
    while(num != 0){
      DispCom.Digits[i] = num%10;
      num /= 10;
      i++;
    }
	DispCom.InputNumDigits = i;
  }
}

void DispCom_Tx_Number_Digits(void){
  for(uint8_t i = DispCom.InputNumDigits; i > 0; i--){
    uint8_t temp = i;
    temp -= 1;
    temp  = DispCom.Digits[temp];
    temp += 48;
    DispCom_Tx_Byte(temp);
  }
}

void DispCom_Tx_Number(int32_t num){
  if(num < 0){
    DispCom_Tx_Byte('-');
	  num = -num;
  }
  DispCom_Determine_Digit_Numbers((uint32_t)num);
  DispCom_Tx_Number_Digits();
}

void DispCom_Tx_Number_Hex(uint32_t val){
  uint16_t hex_digit, index = 0, loop_counter = 0;
  if(val <= 0xFF){
    index = 8;
    loop_counter = 2;
  }else if(val <= 0xFFFF){
    index = 16;
    loop_counter = 4;     
  }else{
    index = 32;
    loop_counter = 8;
  }
  DispCom_Tx_Byte('0');
  DispCom_Tx_Byte('x');
  for(uint8_t i = 0; i < loop_counter; i++){
	index -= 4;
	hex_digit = (uint8_t)((val>>index) & 0x0F);
	if(hex_digit > 9){
	  hex_digit += 55;
	}
	else{
	  hex_digit += 48;
	}
	DispCom_Tx_Byte((uint8_t)hex_digit);
  }
}

void DispCom_Tx_Number_Bin(uint32_t val){
  uint8_t loop_counter = 0;
  if(val <= 0xFF){
    loop_counter = 7;
  }else if(val <= 0xFFFF){
    loop_counter = 15;     
  }else{
    loop_counter = 31;
  }
  
  DispCom_Tx_Byte('0');
  DispCom_Tx_Byte('b');
  for(int i = loop_counter; i >= 0; i--){
    if( (val>>i) & 1){
      DispCom_Tx_Byte( 49 );   
    }else{
      DispCom_Tx_Byte( 48 );         
    }
  }
}

/*********************Number Functions End*********************/









/************Number with End Char Functions Start**************/

void DispCom_Tx_Number_NL(int32_t num){
  DispCom_Tx_Number(num);
  DispCom_Tx_NL();
}

void DispCom_Tx_Number_SP(int32_t num){
  DispCom_Tx_Number(num);
  DispCom_Tx_SP();
}

void DispCom_Tx_Number_CM(int32_t num){
  DispCom_Tx_Number(num);
  DispCom_Tx_CM();
}

/*************Number with End Char Functions End***************/









/**********Hex Number with End Char Functions Start************/

void DispCom_Tx_Number_Hex_NL(uint32_t num){
  DispCom_Tx_Number_Hex(num);
  DispCom_Tx_NL();
}

void DispCom_Tx_Number_Hex_SP(uint32_t num){
  DispCom_Tx_Number_Hex(num);
  DispCom_Tx_SP();
}

void DispCom_Tx_Number_Hex_CM(uint32_t num){
  DispCom_Tx_Number_Hex(num);
  DispCom_Tx_CM();
}

/***********Hex Number with End Char Functions End*************/









/**********Bin Number with End Char Functions Start************/

void DispCom_Tx_Number_Bin_NL(uint32_t num){
  DispCom_Tx_Number_Bin(num);
  DispCom_Tx_NL();
}

void DispCom_Tx_Number_Bin_SP(uint32_t num){
  DispCom_Tx_Number_Bin(num);
  DispCom_Tx_SP();
}

void DispCom_Tx_Number_Bin_CM(uint32_t num){
  DispCom_Tx_Number_Bin(num);
  DispCom_Tx_CM();
}

/***********Bin Number with End Char Functions End*************/







/************Number with Parameter Functions Start*************/

void DispCom_Tx_Parameter_NL(char *name, int32_t num){
  DispCom_Tx_Text(name);
  DispCom_Tx_SP();
  DispCom_Tx_Number_NL(num);
}

void DispCom_Tx_Parameter_SP(char *name, int32_t num){
  DispCom_Tx_Text(name);
  DispCom_Tx_SP();
  DispCom_Tx_Number_SP(num);
}

void DispCom_Tx_Parameter_CM(char *name, int32_t num){
  DispCom_Tx_Text(name);
  DispCom_Tx_SP();
  DispCom_Tx_Number_CM(num);
}

/*************Number with Parameter Functions End**************/









/**********Hex Number with Parameter Functions Start***********/

void DispCom_Tx_Parameter_Hex_NL(char *name, uint32_t num){
  DispCom_Tx_Text(name);
  DispCom_Tx_SP();
  DispCom_Tx_Number_Hex_NL(num);
}

void DispCom_Tx_Parameter_Hex_SP(char *name, uint32_t num){
  DispCom_Tx_Text(name);
  DispCom_Tx_SP();
  DispCom_Tx_Number_Hex_SP(num);
}

void DispCom_Tx_Parameter_Hex_CM(char *name, uint32_t num){
  DispCom_Tx_Text(name);
  DispCom_Tx_SP();
  DispCom_Tx_Number_Hex_CM(num);
}

/***********Hex Number with Parameter Functions End************/









/**********Bin Number with Parameter Functions Start***********/

void DispCom_Tx_Parameter_Bin_NL(char *name, uint32_t num){
  DispCom_Tx_Text(name);
  DispCom_Tx_SP();
  DispCom_Tx_Number_Bin_NL(num);
}

void DispCom_Tx_Parameter_Bin_SP(char *name, uint32_t num){
  DispCom_Tx_Text(name);
  DispCom_Tx_SP();
  DispCom_Tx_Number_Bin_SP(num);
}

void DispCom_Tx_Parameter_Bin_CM(char *name, uint32_t num){
  DispCom_Tx_Text(name);
  DispCom_Tx_SP();
  DispCom_Tx_Number_Bin_CM(num);
}

/***********Bin Number with Parameter Functions End************/









/*******************DispCom Buffer Functions Start***************/

void DispCom_Buf_Flush(void){
  for(uint8_t i = 0; i < DISPCOM_BUFFER_SIZE; i++){
	DispCom.Buf[i] = 0;
  }
  DispCom.BufIndex = 0;
}

uint8_t DispCom_Buf_Get(uint16_t index){
  return DispCom.Buf[index];
}

uint16_t DispCom_Buf_Get_Index(void){
  return DispCom.BufIndex;
}

/********************DispCom Buffer Functions End****************/









/*******************DispCom Data Functions Start****************/

uint8_t DispCom_Data_Available(void){
  return DispCom.RxPacket.DataAvailable;
}

uint16_t DispCom_Data_Len_Get(void){
  return DispCom_Buf_Get_Index();
}

uint16_t DispCom_Data_Calculated_CRC_Get(void){
  return DispCom.RxPacket.CalculatedCRC;
}

uint16_t DispCom_Data_Received_CRC_Get(void){
  return DispCom.RxPacket.ReceivedCRC;
}

uint8_t DispCom_Data_CRC_Status_Get(void){
  return DispCom.RxPacket.CRCStatus;
}

uint8_t DispCom_Data_Read_Complete_Status(void){
  return DispCom.RxPacket.DataReadComplete;
}

void DispCom_Data_Clear_Available_Flag(void){
  DispCom.RxPacket.DataAvailable = DISPCOM_FALSE;
}

void DispCom_Data_Clear_Read_Complete_Flag(void){
  //DispCom_Buf_Flush();
  DispCom.RxPacket.DataReadComplete = DISPCOM_TRUE;
}


void DispCom_Data_Copy_Buf(uint8_t *buf){
  for(uint16_t i = 0; i < DispCom_Data_Len_Get(); i++){
	  buf[i] = DispCom_Buf_Get(i);
  }
}


void DispCom_Data_Print_Buf(void){
  for(uint16_t i = 0; i < DispCom_Data_Len_Get(); i++){
	  DispCom_Tx_Byte( DispCom_Buf_Get(i) );
  }
  DispCom_Tx_NL();
}

/********************DispCom Data Functions End*****************/









/******************Error Code Functions Start****************/

uint8_t DispCom_Error_Code_Get(void){
  return DispCom.Error;
}

void DispCom_Error_Code_Clear(void){
  DispCom.Error = 0;
}

/******************Error Code Functions End******************/









/***************DispCom ISR Handler Functions Start************/

void DispCom_ISR_Handler(void){
  DispCom_Clear_Interrupt_Flag();
  DispCom.LastRxByte = (uint8_t)DispCom_Rx_Byte();
  if(DispCom.Error == 0x00){
    DispCom.Buf[DispCom.BufIndex] = DispCom.LastRxByte;
    DispCom.BufIndex++;
    if(DispCom.BufIndex >= DispCom.BufSize){
      DispCom.BufIndex = 0;
    }
  }
  else{
    DispCom.LastRxByte = DISPCOM_NULL;
  }
  
  DispCom_Timer_Value_Reset();
  if(DispCom.Timer.Enabled == DISPCOM_FALSE){
	  DispCom_Timer_Enable();
	  DispCom.Timer.Enabled = DISPCOM_TRUE;
  }
  
}

void DispCom_Timer_ISR_Handler(void){
  DispCom_Timer_Clear_Interrupt_Flag();
  if(DispCom.Timer.Enabled == DISPCOM_TRUE){
    DispCom_Timer_Disable();
	  DispCom.Timer.Enabled = DISPCOM_FALSE;
  }
  
  if(DispCom_Buf_Get_Index() != DISPCOM_NULL){
    
	  if(DispCom.RxPacket.DataReadComplete == DISPCOM_FALSE){
	    DispCom.Error = DISPCOM_RX_ERR_READ_INCOMPLETE;
	  }
    DispCom_RX_Packet_CRC_Check();
    #ifdef DISPCOM_CRC_ENABLE
	  if(DispCom.RxPacket.CRCStatus == DISPCOM_TRUE){
	    DispCom.RxPacket.DataAvailable = DISPCOM_TRUE;
	  }
	  else{
	    DispCom_RX_Packet_Read_Complete();
	    DispCom.RxPacket.DataAvailable = DISPCOM_FALSE;
	  }
	  #else
	  DispCom.RxPacket.DataAvailable = DISPCOM_TRUE;
	  #endif
	
	  DispCom.RxPacket.DataReadComplete = DISPCOM_FALSE;
  }
}

/****************DispCom ISR Handler Functions End*************/









/******************DispCom CRC Functions Start****************/

#ifdef   DISPCOM_CRC_XMODEM

uint16_t DispCom_CRC_Calculate_Byte(uint16_t crc, uint8_t data){
	uint16_t temp = data;
	temp <<= 8;
  crc = crc ^ temp;
  for(uint8_t i = 0; i < 8; i++){
    if(crc & 0x8000){
			temp   = crc;
			temp <<= 0x01;
			temp  ^= 0x1021;
	    crc = temp;
	  }
    else{
	    crc <<= 1;
	  }
  }
  return crc;
}

uint16_t DispCom_CRC_Calculate_Block(uint8_t *buf, uint8_t len){
  uint16_t crc = 0;
  for(uint8_t i = 0; i < len; i++){
    crc = DispCom_CRC_Calculate_Byte(crc,buf[i]);
  }
  return crc;
}
#endif

#ifdef   DISPCOM_CRC_ALTERNATE

uint16_t CRCTalbe[16] = {
 0x0000, 0xCC01, 0xD801, 0x1400,
 0xF001, 0x3C00, 0x2800, 0xE401,
 0xA001, 0x6C00, 0x7800, 0xB401,
 0x5000, 0x9C01, 0x8801, 0x4400
};


uint16_t DispCom_CRC_Calculate_Block(uint8_t *buf, uint8_t len){
 uint16_t crc = 0xFFFF, i;
 uint8_t  Data;
 for (i = 0; i < len; i++) {
  Data = *buf++;
  crc = CRCTalbe[(Data ^ crc) & 0x0f] ^ (crc >> 4);
  crc = CRCTalbe[((Data >> 4) ^ crc) & 0x0f] ^ (crc >> 4);
 }
 crc = ((crc & 0xFF) << 8) | ((crc >> 8) & 0xFF);
 return crc;
}

#endif

/*******************DispCom CRC Functions End*****************/









/*************DispCom RX Packet Functions Start***************/

void DispCom_RX_Packet_CRC_Check(void){
	uint8_t  temp = 0 ;
  uint16_t crc_calc = 0, crc_recv = 0;
  if( DispCom_Data_Len_Get() >= 2){
		temp  = (uint8_t)DispCom_Data_Len_Get();
		temp -= 2;
    crc_calc   =  DispCom_CRC_Calculate_Block((uint8_t*)DispCom.Buf, temp);
    crc_recv   =  DispCom_Buf_Get(DispCom_Data_Len_Get() - 2);
    crc_recv <<= 8;
    crc_recv  |= DispCom_Buf_Get(DispCom_Data_Len_Get() - 1);
  }
  DispCom.RxPacket.CalculatedCRC = crc_calc;
  DispCom.RxPacket.ReceivedCRC = crc_recv;
  if( DispCom.RxPacket.CalculatedCRC == DispCom.RxPacket.ReceivedCRC ){
    DispCom.RxPacket.CRCStatus = DISPCOM_TRUE;
  }
  else{
    DispCom.RxPacket.CRCStatus = DISPCOM_FALSE;
  }
}

void DispCom_RX_Packet_Read_Complete(void){
  DispCom_Buf_Flush();
  DispCom_Data_Clear_Available_Flag();
  DispCom_Data_Clear_Read_Complete_Flag();
	DispCom_Error_Code_Clear();
}

/**************DispCom RX Packet Functions End****************/









/*****************DispCom Init Functions Start****************/

void DispCom_Init(uint32_t baud){
  DispCom_Struct_Init();
  DispCom_RX_Packet_Struct_Init();
  DispCom_Timer_Struct_Init();
  
  DispCom_Config_GPIO();
  DispCom_Config_Clock();
  DispCom_Config_BAUD_Rate(baud);
  
  #ifdef DISPCOM_ENABLE_TX  
  DispCom_Config_Tx();
  #endif
  
  #ifdef DISPCOM_ENABLE_RX
  DispCom_Config_Rx();
  #endif
  
  #ifdef DISPCOM_ENABLE_RX_INT
  DispCom_Config_Rx_Interrupt();
  #endif
  
  DispCom_Timer_Init();
  DispCom_Buf_Flush();
}

/******************DispCom Init Functions End*****************/