#include <msp430.h>
#include <stdlib.h>
#include "timerLib/libTimer.h"
#include "led.h"

void switch_interrupt_handler();
int inArray(int check);
int testUser(int count);
int array[3];
int userInput;

int main(void) {
  led_init();
  wdt_init();
  switch_init();
  buzzer_init();

  or_sr(0x18);
}

int inArray(int check){
  for(int i = 0; i < 3; i++)
    if(check == array[i])
      return 1;
  return 0;
}

void switch_interrupt_handler(){
  char p2val = P2IN;
  P2IES |= (p2val & SWITCHES);
  P2IES &= (p2val | ~SWITCHES);

  if(!(p2val & SW1)){
    buzzer_set_period(1000);
    userInput = 1;}
  else if(!(p2val & SW2)){
    buzzer_set_period(1500);
    userInput = 2;}
  else if(!(p2val & SW3)){
    buzzer_set_period(2000);
    userInput = 3;}
  else{
    buzzer_set_period(0);
    userInput = 0;
  }
}

int testUser(int count){
  static int waitForRest = 0;
  if(array[count] == userInput && waitForRest == 0){
    waitForRest = 1;
  }
  else if (waitForRest == 1 && userInput == 0){
    waitForRest = 0;
    return 1;
  }
  return 0;
}

void
__interrupt_vec(PORT2_VECTOR) Port_2(){
  if (P2IFG & SWITCHES){
    P2IFG &= ~SWITCHES;
    switch_interrupt_handler();
  }
}

void
__interrupt_vec(WDT_VECTOR) WDT(){
  static int secondCount = 0;
  static int state = 0;
  static int count = 0;
  secondCount++;

  //State 0 fills array with random values of 1-3
  while(state == 0){
    if(count == 0){
      array[count] = 1;   	//Array holds values from 1-3.
      count++;
    }
    else if (count > 0 && count < 3){
      int check = (rand() % 3)+1;
      while (inArray(check))
	check = (rand() % 3)+1;
      array[count] = check;
      count++;
    }
    if(count == 3){
      state++;
      count = 0;
    }
  }
  
  //State 1 show current position.
  if(state == 1 && secondCount == 150){
    switch(array[count]){
    case 1:
      buzzer_set_period(1000);
      P1OUT |= LED_RED;
      P1OUT &= ~LED_GREEN;
      break;
    case 2:
      buzzer_set_period(1500);
      P1OUT |= LED_GREEN;
      P1OUT &= ~LED_RED;
      break;
    case 3:
      buzzer_set_period(2000);
      P1OUT |= LEDS;
      break;
    }
    state = 2;
    secondCount = 0;
  }

  //State 2 turn off lights.
  else if(state == 2 && secondCount == 150){
    buzzer_set_period(0);
    P1OUT &= ~LEDS;
    state = 3;
    secondCount = 0;
  }

  //State 3 Ask user what number was shown.
  else if(state == 3){
    secondCount--;
    if(testUser(count)){
      count++;
      state = 1;}
    if(count == 3)
      state = 4;
  }

  else if (state == 4)
    P1OUT |= LEDS;
}


