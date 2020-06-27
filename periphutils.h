#include <LPC17xx.h>
#include <stdbool.h>
#include <stdio.h>

uint32_t IntToBits(uint8_t n);

void writeLED(uint8_t led_num, bool bit_value);

void set_all_LED(bool bit_value);

void setupLED(void);

void setLEDs(uint8_t* arr);

void initializeADC(void);

float readADC(void);
