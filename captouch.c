#include "stm32g030xx.h"


 void I2C2_GPIO_Init(void){
	if((RCC->IOPENR & RCC_IOPENR_GPIOAEN)!=RCC_IOPENR_GPIOAEN){
		RCC->IOPENR|=RCC_IOPENR_GPIOAEN;
	}
	GPIOA->MODER&=~GPIO_MODER_MODE11_Msk;
	GPIOA->MODER|=GPIO_MODER_MODE11_1;
	GPIOA->MODER&=~GPIO_MODER_MODE12_Msk;
	GPIOA->MODER|=GPIO_MODER_MODE12_1;
	GPIOA->OTYPER|=GPIO_OTYPER_OT11;
	GPIOA->OTYPER|=GPIO_OTYPER_OT12;
	GPIOA->PUPDR&=~GPIO_PUPDR_PUPD11_Msk;
	GPIOA->PUPDR|=GPIO_PUPDR_PUPD11_0;
	GPIOA->PUPDR&=~GPIO_PUPDR_PUPD12_Msk;
	GPIOA->PUPDR|=GPIO_PUPDR_PUPD12_0;
	GPIOA->AFR[1]&=~GPIO_AFRH_AFSEL11_Msk;
	GPIOA->AFR[1]|=(0x06<<GPIO_AFRH_AFSEL11_Pos);
	GPIOA->AFR[1]&=~GPIO_AFRH_AFSEL12_Msk;
	GPIOA->AFR[1]|=(0x06<<GPIO_AFRH_AFSEL12_Pos);
}

 void I2C2_Clock_Init(void){
	RCC->APBENR1|=RCC_APBENR1_I2C2EN;
	RCC->CCIPR&=~RCC_CCIPR_I2S1SEL_Msk;
	RCC->CCIPR|=RCC_CCIPR_I2S1SEL_1;
}

 void I2C2_Config(void){
	I2C2->TIMINGR =(0x03<<I2C_TIMINGR_PRESC_Pos);
	I2C2->TIMINGR|=(0x04<<I2C_TIMINGR_SCLDEL_Pos);
	I2C2->TIMINGR|=(0x02<<I2C_TIMINGR_SDADEL_Pos);
	I2C2->TIMINGR|=(0xC3<<I2C_TIMINGR_SCLH_Pos);
	I2C2->TIMINGR|=(0xC7<<I2C_TIMINGR_SCLL_Pos);
	I2C2->CR1|=I2C_CR1_PE;
}

 void I2C2_Init(void){
  I2C2_GPIO_Init();
	I2C2_Clock_Init();
	I2C2_Config();
}

 uint8_t I2C2_TX_ADDR(uint8_t addr){
	uint8_t ack=1;
	I2C2->CR2=I2C_CR2_AUTOEND|(1<<16)|(addr<<1);
	I2C2->CR2|=I2C_CR2_START;
	while((I2C2->ISR & I2C_ISR_TXE)!=I2C_ISR_TXE);
	if(I2C2->ISR & I2C_ISR_NACKF){
		ack=0;
		I2C2->ICR|=I2C_ICR_NACKCF;
	}
	return ack;
}

 uint8_t I2C2_TX_ADDR_READ(uint8_t addr){
	uint8_t ack=1;
	I2C2->CR2=I2C_CR2_AUTOEND|(1<<16)|(addr<<1)|I2C_CR2_RD_WRN;
	I2C2->CR2|=I2C_CR2_START;
	while((I2C2->ISR & I2C_ISR_TXE)!=I2C_ISR_TXE);
	if(I2C2->ISR & I2C_ISR_NACKF){
		ack=0;
		I2C2->ICR|=I2C_ICR_NACKCF;
	}
	return ack;
}

 void I2C2_Write_Register(uint8_t addr, uint8_t data){
	I2C2->TXDR=addr;
  I2C2->CR2=I2C_CR2_AUTOEND|(2<<16)|(0x41<<1);
	I2C2->CR2|=I2C_CR2_START;
	while((I2C2->ISR & I2C_ISR_TXE)!=I2C_ISR_TXE);
	I2C2->TXDR=data;
	while((I2C2->ISR & I2C_ISR_TXE)!=I2C_ISR_TXE);
}

#define    CAPTOUCH_SHORT_PRESS_TIME_MIN  1   
#define    CAPTOUCH_SHORT_PRESS_TIME_MAX  1000  
#define    CAPTOUCH_LONG_PRESS_TIME       2500


#define    CAPTOUCH_TOTAL_TOUCH_KEY       2
#define    CAPTOUCH_TOUCH_KEY_1           3
#define    CAPTOUCH_TOUCH_KEY_2           4
//#define    CAPTOUCH_TOUCH_KEY_3           4

uint8_t    CapTouch_Pins[CAPTOUCH_TOTAL_TOUCH_KEY]={CAPTOUCH_TOUCH_KEY_1,CAPTOUCH_TOUCH_KEY_2};

typedef struct captouch_t{
  uint8_t  Channel;
	uint8_t  Raw;
  uint8_t  Result[CAPTOUCH_TOTAL_TOUCH_KEY];
  uint8_t  TriggerMethod[CAPTOUCH_TOTAL_TOUCH_KEY];
  uint8_t  LastTouchState[CAPTOUCH_TOTAL_TOUCH_KEY];
  uint32_t TouchDuration[CAPTOUCH_TOTAL_TOUCH_KEY];
  uint32_t TouchDurationTemp[CAPTOUCH_TOTAL_TOUCH_KEY];
  uint8_t  LongTouchFlag[CAPTOUCH_TOTAL_TOUCH_KEY];
  uint8_t  UsingReferenceTimer;
  uint8_t  ErrorCode[CAPTOUCH_TOTAL_TOUCH_KEY];
}captouch_t;
  
captouch_t CapTouch;
  
void CapTouch_Struct_Init(void){
  CapTouch.Channel=0;
  for(uint8_t i=0;i<CAPTOUCH_TOTAL_TOUCH_KEY;i++){
  CapTouch.Result[i]=0;
  CapTouch.TriggerMethod[i]=0;
  CapTouch.LastTouchState[i]=0;
  CapTouch.TouchDuration[i]=0;
  CapTouch.TouchDurationTemp[i]=0;
  CapTouch.LongTouchFlag[i]=0;
  CapTouch.UsingReferenceTimer=0;
  CapTouch.ErrorCode[i]=0;
  }
	CapTouch.Raw=0x00;
}

void I2C2_Read_Captouch_Sensors(void){
  I2C2->TXDR=0x08;
	I2C2_TX_ADDR(0x41);
	I2C2_TX_ADDR_READ(0x41);
	CapTouch.Raw=I2C2->RXDR;
}

void CapTouch_Response(uint8_t current_channel){
	if(CapTouch.Raw & (1<<CapTouch_Pins[current_channel])){
		CapTouch.Result[current_channel]=1;
	}else{
		CapTouch.Result[current_channel]=0;
	}
}

void CapTouch_Check_Trigger(uint8_t current_channel){
  uint8_t current_state=0;
  CapTouch_Response(current_channel);
  current_state=CapTouch.Result[current_channel];
    if((CapTouch.LastTouchState[current_channel]==0) && (current_state==1)){
        CapTouch.LongTouchFlag[current_channel]=0;     //Rising Edge
        CapTouch.TriggerMethod[current_channel]=0x01;
    }else if((CapTouch.LastTouchState[current_channel]==1) && (current_state==0)){
        CapTouch.TriggerMethod[current_channel]=0x04;  //Falling Edge
    }else if((CapTouch.LastTouchState[current_channel]==1) && (current_state==1)){
        CapTouch.TriggerMethod[current_channel]=0x02;  //Constant Touched
    }else{
        CapTouch.TriggerMethod[current_channel]=0x00;  //No Touch Event Occured
    }
    CapTouch.LastTouchState[current_channel]=current_state;
}


void CapTouch_Check_Touch_Duration(uint8_t current_channel, uint32_t refernce_time){
  uint32_t duration=0;
  CapTouch_Check_Trigger(current_channel);
  if(CapTouch.TriggerMethod[current_channel]==0x01){
    CapTouch.UsingReferenceTimer|=(1<<current_channel);
    CapTouch.TouchDurationTemp[current_channel]=refernce_time;
  }else if(CapTouch.TriggerMethod[current_channel]==0x04 || CapTouch.TriggerMethod[current_channel]==0x02){
    duration=refernce_time;
    duration-=CapTouch.TouchDurationTemp[current_channel];
    if(duration>0){
      CapTouch.TouchDuration[current_channel]=duration;
    }
    if(CapTouch.TriggerMethod[current_channel]==0x04){
      CapTouch.TouchDurationTemp[current_channel]=refernce_time;
      CapTouch.UsingReferenceTimer&=~(1<<current_channel);
    }
  }
  else{
    CapTouch.TouchDuration[current_channel]=0;
    CapTouch.TouchDurationTemp[current_channel]=0;
  }
}


void CapTouch_Scan_Sensors(uint32_t refernce_time){
	I2C2_Read_Captouch_Sensors();
  for(uint8_t channel=0;channel<CAPTOUCH_TOTAL_TOUCH_KEY;channel++){
    CapTouch_Check_Touch_Duration(channel, refernce_time);
  }
}

uint8_t CapTouch_Short_Pressed(uint8_t current_channel){
  uint8_t sts=0;
  if(CapTouch.TriggerMethod[current_channel]==0x04){
    if((CapTouch.TouchDuration[current_channel]>CAPTOUCH_SHORT_PRESS_TIME_MIN) && (CapTouch.TouchDuration[current_channel]<CAPTOUCH_SHORT_PRESS_TIME_MAX)){
      sts=1;
    }
  }
  return sts;
}

uint8_t CapTouch_Long_Pressed(uint8_t current_channel){
  uint8_t sts=0;
  if((CapTouch.TouchDuration[current_channel]>CAPTOUCH_LONG_PRESS_TIME) && (CapTouch.LongTouchFlag[current_channel]==0)){
    CapTouch.LongTouchFlag[current_channel]=1;
    sts=1;
  }
  return sts;
}

uint8_t CapTouch_Constant_Pressed(uint8_t current_channel){
  uint8_t sts=0;
  if((CapTouch.TriggerMethod[current_channel]==0x02) && (CapTouch.TouchDuration[current_channel]>CAPTOUCH_SHORT_PRESS_TIME_MIN)){
    sts=1;
  }
  return sts;
}


uint32_t CapTouch_Get_Touch_Duration(uint8_t current_channel){
  return CapTouch.TouchDuration[current_channel];
}

uint8_t CapTouch_Get_Touch_Response(uint8_t current_channel){
  return CapTouch.Result[current_channel];
}

uint8_t CapTouch_Get_Reference_Timer_Use_Flag(void){
  if(CapTouch.UsingReferenceTimer){
    return 1;
  }else{
    return 0;
  }
}

uint8_t CapTouch_Get_ErrorCode(uint8_t current_channel){
  return CapTouch.ErrorCode[current_channel];
}

void CapTouch_Get_Debug_Data(uint8_t current_channel, int *data_out){
  data_out[0]=4;
  data_out[1]=current_channel;
  data_out[2]=CapTouch_Get_Touch_Duration(current_channel);
  data_out[3]=CapTouch_Get_Touch_Response(current_channel);
  data_out[4]=CapTouch_Get_Reference_Timer_Use_Flag();
}

uint8_t Captouch_Get_Reference_Timer_Use_Flag(void){
  return CapTouch.UsingReferenceTimer;
}

void CapTouch_Init(void){
  CapTouch_Struct_Init();
	I2C2_Init();
}
