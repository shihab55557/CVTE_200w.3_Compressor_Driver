#include "stm32g030xx.h"
#include "app.h"

int main(void){
	App_Setup();
	while(1){
		while(1){
			App_Main_Loop();
		}
	}
}
