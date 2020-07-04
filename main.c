#include <stdio.h>
#include <cmsis_os2.h>
#include <LPC17xx.h>
#include "periphutils.h"
#define ISR 0


//TODO: interrupt for LEDtoggle
//TODO: interrupt for Joystick?
//question: do interrupts only work on 1->0 transition??

void joystickDisplay(void* arg){
	// read joystick direction
	// display binary number {0: C, N:1, E: 2, S: 3, W: 4} on LED
	// if joystick pressed in light LED 5
	while(1){
		uint16_t dir_pins = (LPC_GPIO1->FIOPINH) >> 7;
		uint16_t press_pin = ((LPC_GPIO1->FIOPINH) >> 4) & 1; // and with 1 to isolate bit 20
		
		writeLED(0,0); writeLED(1,0); writeLED(2,0); writeLED(5,0);
		// note this treats 1 as on and 0 as off by doing ~dir_pins
		// & with 0xf to isolate last 4 bits (bits 23-26)
		switch(~dir_pins & 0xf){
			case 1:
				writeLED(0, 1); break;
			case 1 << 1:
				writeLED(1, 1); break;
			case 1 << 2:
				writeLED(1, 1);writeLED(0, 1); break;
			case 1 << 3:
				writeLED(2, 1); break;
			default:
				writeLED(0,0); writeLED(1,0); writeLED(2,0); break;
		}
		writeLED(5, !press_pin);
		osThreadYield();
	}
}

void printADC(void* arg){
	// read ADC vals and output
	//osDelay instead of interrupt?
	while(1){
		float adc_val = readADC();
		printf("ADC val: %.2f\n", adc_val);
		osThreadYield();
	}
}

#if ISR == 0
void toggleLED(void* arg){
	static bool button_was_pressed = false;
	while(1){
		// wait for button press and button release
		// on release toggle LED 7 state
		bool button_state = (LPC_GPIO2->FIOPIN >> 10) & 0x1;
		if( !button_state ){
			button_was_pressed = true;
			osThreadYield();
		}else if( button_was_pressed ){
			//bool led_state = (LPC_GPIO2->FIOPIN >> 6) & 0x1;
			//writeLED(7, !led_state);
			//toggle LED 7
			LPC_GPIO2->FIOPIN ^= (1 << 6);
			button_was_pressed = false;
			osThreadYield();
		}
		
		//debugging
		//printf("bstate: %d %d\n", button_state, button_was_pressed);
		/*
		if( button_state && button_was_pressed){
			//toggle LED 7
			//LPC_GPIO2->FIOPIN ^= (1 << 6);
			writeLED(7, !led_state);
			button_was_pressed = false;
		}else if( !button_state){
			button_was_pressed = true;
		}
		osThreadYield();*/
	}
	

}
#endif
#if ISR == 1
void toggleLED(void* arg)
	static bool button_was_pressed = false;
	while(1){
		// wait for button press and button release
		//osEventFlagsWait (use any) flags for button down and button up
		// on release toggle LED 7 state
		bool button_state = (LPC_GPIO2->FIOPIN >> 10) & 0x1;
		if( !button_state ){
			button_was_pressed = true;
			osThreadYield();
		}else if( button_was_pressed ){
			//bool led_state = (LPC_GPIO2->FIOPIN >> 6) & 0x1;
			//writeLED(7, !led_state);
			//toggle LED 7
			LPC_GPIO2->FIOPIN ^= (1 << 6);
			button_was_pressed = false;
			osThreadYield();
		}
	}
}
#endif

int main(void){
	/*osKernelInitialize();
	osThreadNew(counter, NULL, NULL);
	osThreadNew(printer, "Printer says: ", NULL);
	osKernelStart();*/
	setupLED();
	initializeADC();
	
	osKernelInitialize();
	
	osThreadNew(printADC, NULL, NULL);
	osThreadNew(joystickDisplay, NULL, NULL);
	osThreadNew(toggleLED, NULL, NULL);
	
	printf("start\n");
	osKernelStart();

}
