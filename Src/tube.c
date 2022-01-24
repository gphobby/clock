#include "main.h"
#include "cmsis_os.h"
#include "adc.h"
#include "rtc.h"
#include "spi.h"
#include "tim.h"
#include "usb_device.h"
#include "gpio.h"
#include <math.h>

#include "h/tube.h"

typedef struct { 
  TIM_HandleTypeDef *timer;
  uint32_t channel;
} anode_channel;

SPI_HandleTypeDef *tubes_spi = &hspi1;
anode_channel tube_anode[4] = {
  { .timer = &htim2, .channel = TIM_CHANNEL_1 },
  { .timer = &htim4, .channel = TIM_CHANNEL_1 },
  { .timer = &htim4, .channel = TIM_CHANNEL_2 },
  { .timer = &htim2, .channel = TIM_CHANNEL_2 },
};


uint8_t tube_bits[4][11] = {
  // [dot] [0 1 ... 9]
  {43, 37, 32, 47, 46, 45, 33, 34, 35, 44, 36},
  {52, 50, 39, 38, 42, 41, 40, 55, 54, 53, 51},
  {12, 26, 25, 14, 13, 11, 10,  9,  8, 23, 22},
  {21, 17, 27, 28, 29, 20, 30, 31, 16, 19, 18},
};


// tube class

typedef struct {
  uint8_t state[8];
} tubes_state;



typedef struct { 
  tubes_state state;
  tubes_state displayed_state;
  
  float current;
  uint16_t pwm;
  uint8_t awaiting_transmit;
  
} tube_display;

tube_display tdisplay = {
  .state = { .state = {0,0,0,0, 0,0,0,0} },
  .displayed_state = { .state = {0,0,0,0, 0,0,0,0} },
  
  .awaiting_transmit = 1,
  
  .current = 2,
  
  .pwm = 15,
};


void tube_reset_state(void){
  memset(&tdisplay.state, 0, sizeof(tdisplay.state)); 
}

void tube_set_current(uint8_t tube, float current){
  float pwm_period = current / ANODE_CURRENT_CONST;
    
  __HAL_TIM_SET_COMPARE(tube_anode[tube].timer, tube_anode[tube].channel, (uint32_t)pwm_period);
}


void tube_set_digit(uint8_t tube, int8_t digit){
  // d < 0 - blank;
  if(digit < 0 || digit > 9) {
    return;
  }
  
  uint8_t bitnum = tube_bits[tube][digit+1];
  
  tdisplay.state.state[bitnum / 8] |= 1 << (bitnum % 8);
}

void tube_set_dot(uint8_t tube){
  uint8_t bitnum = tube_bits[tube][0];
  
  tdisplay.state.state[bitnum / 8] |= 1 << (bitnum % 8);
}

void tube_refresh_all(void){
  tdisplay.displayed_state = tdisplay.state;
  
  tdisplay.awaiting_transmit = 1;
}

void tube__transmit_state(void){
  HAL_SPI_Transmit_IT(tubes_spi, tdisplay.displayed_state.state, sizeof(tdisplay.displayed_state.state));
  
  tdisplay.awaiting_transmit = 0;
}

void tube_set_pwm(uint16_t pwm){
  // -1 because if there is no blanking time indication won't work,
  // indication code does register refresh when tubes are blank
  
  if(pwm >= (DISPLAY_PWM_LEVELS-1) ){
    pwm = (DISPLAY_PWM_LEVELS-1);
  }
  
  if(pwm <= 0 ){
    pwm = 1;
  }
  

  tdisplay.pwm = pwm;
}

void tube_set_current_all(float current){
  tdisplay.current = current;
}




// display class


display_state d_state = {
  .animation = NULL,
  // init disabled digits too if will move this to stack or heap
};

void display_number(uint16_t number){
  int16_t i = DISPLAY_DIGITS_COUNT - 1;
  
  while (i >= 0) {
    int digit = number % 10;
    
    display_digit(i, digit);
    
    i--;
    number /= 10;
  }
  
}


void display_init_animation(display_animation ani){
  
  d_state.animation = animations_list[ani];
  
  
  d_state.animation->previous = d_state.digits;
  d_state.animation->step = 0;
  
  if(d_state.animation->animation_init ){
    d_state.animation->animation_init(&d_state);
  }
}

void display_start_animation(void){
   
}

void display_digit(uint8_t tube, int8_t digit){
  if(d_state.digits.disabled[tube]){
    digit = -1;
  }
  
  d_state.digits.digit[tube] = digit;
}

void display_set_dot(uint8_t tube, uint8_t enabled){
  d_state.digits.dot[tube] = enabled;
}

void display_reset_dots(void){
   for(uint8_t i = 0; i < DISPLAY_DIGITS_COUNT; i++){ 
     display_set_dot(i, 0);
   }
}



void display_set_tube_state(uint8_t tube, uint8_t enabled){
  d_state.digits.disabled[tube] = !enabled;
}


void display_finish_animation(display_animation ani){
  
}










void DisplayIndicationTask(void const * argument)
{
  
  for(;;){
    display_digits* to_display;
    
    if(d_state.animation){
      d_state.animation->step++;
      
      if(d_state.animation->animation_step ){
        to_display = d_state.animation->animation_step(&d_state);
      } else {
        to_display = &d_state.digits;
      }
    }
    
    tube_reset_state();
    
    for(uint8_t i = 0; i < DISPLAY_DIGITS_COUNT; i++){ 
      tube_set_digit(i, to_display->digit[i]);
      
      if(to_display->dot[i]){
        tube_set_dot(i);
      }
      
    }
    
    tube_refresh_all();
    
    
    osDelay(10);
  }
}

    

static uint16_t step = 0;
float td_current = 0.0;
 
void DisplayIrqHandler(void){
  
  if(td_current == 0.0){ // it was blanked for some time, so flush state now to prevent unwanted glow
    
    if(tdisplay.awaiting_transmit){
      tube__transmit_state();
    }
    
  }
  
  
  float current_new = 0.0; 
  
  if(step++ % (DISPLAY_PWM_LEVELS+1) > tdisplay.pwm){
    current_new = 0.0;
  } else{
    current_new = tdisplay.current;
  }
  
  if(current_new != td_current){
    td_current = current_new;
    
    for(uint8_t i = 0; i < DISPLAY_DIGITS_COUNT; i++){ 
      tube_set_current(i, td_current);
    }
  }
  
}

