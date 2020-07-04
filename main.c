#include <stdio.h>
#include <cmsis_os2.h>
#include <LPC17xx.h>
#include "periphutils.h"

#define BUTTON_FLAG 0x00000001UL
#define JOYSTICK_FLAG 0x2UL
osEventFlagsId_t push_button;


void setupPushbutton(void){
	// enable interrupt when button up
	LPC_GPIOINT->IO2IntEnR |= (1 << 10);
	//should below be 3?
	NVIC_EnableIRQ(EINT3_IRQn);
}

void EINT3_IRQHandler(void){
	// clear interrupt
	LPC_GPIOINT->IO2IntClr |= (1 << 10);
	osEventFlagsSet(push_button, BUTTON_FLAG);
}


void joystickDisplay(void* arg){
	// read joystick direction
	// display binary number {0: C, N:1, E: 2, S: 3, W: 4} on LED
	// if joystick pressed in light LED 5
	
	
	//track joystick state: bits 0..3 are joystick directions, bit 4 is joystick press
	// off is 1 so state starts with all off
	static int dir_state = 0x1f;
	while(1){
		uint16_t dir_pins = (LPC_GPIO1->FIOPINH) >> 7;
		uint16_t press_pin = ((LPC_GPIO1->FIOPINH) >> 4) & 1; // and with 1 to isolate bit 20
		
		int new_dir_state = (dir_pins & 0xf) | (press_pin << 4);
		
		// only change leds if the state has changed
		if(new_dir_state != dir_state){
			//rest leds 1..5
			writeLED(0,0); writeLED(1,0); writeLED(2,0); writeLED(5,0);
			// note this treats 1 as on and 0 as off by doing ~dir_pins
			// & with 0xf to isolate last 4 bits (bits 23-26)
			switch(~dir_pins & 0xf){
				case 1:
					writeLED(0, 1); break;
				case (1 << 1):
					writeLED(1, 1); break;
				case (1 << 2):
					writeLED(1, 1);writeLED(0, 1); break;
				case (1 << 3):
					writeLED(2, 1); break;
				default:
					writeLED(0,0); writeLED(1,0); writeLED(2,0); break;
			}
			writeLED(5, !press_pin);
			dir_state = new_dir_state;
		}
		osThreadYield();
	}
}

void printADC(void* arg){
	// read ADC vals and output
	while(1){
		float adc_val = readADC();
		printf("ADC val: %.2f\n", adc_val);
		osDelay(osKernelGetTickFreq() * 0.1 );
	}
}


void toggleLED(void* arg){
	static bool button_was_pressed = false;
	while(1){
		// wait for button press and button release
		// on release toggle LED 7 state

		//waits on button release
		osEventFlagsWait(push_button, BUTTON_FLAG, osFlagsWaitAny, osWaitForever);
		LPC_GPIO2->FIOPIN ^= (1 << 6);
	}
}

int main(void){
	osKernelInitialize();
	
	push_button = osEventFlagsNew(NULL);

	setupLED();
	initializeADC();
	setupPushbutton();
	
	
	osThreadNew(printADC, NULL, NULL);
	osThreadNew(joystickDisplay, NULL, NULL);
	osThreadNew(toggleLED, NULL, NULL);
	
	printf("start\n");
	osKernelStart();

}

