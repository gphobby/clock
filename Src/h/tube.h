#include "main.h"
#include "cmsis_os.h"

#ifndef TUBE_H 
#define TUBE_H

#define ANODE_CURRENT_CONST (3.6/1000)
#define DISPLAY_DIGITS_COUNT 4

#define DISPLAY_PWM_LEVELS 16

#include "h/button.h"








// tube class
void tube_reset_state(void);

void tube_set_current(uint8_t tube, float current);

void tube_set_digit(uint8_t tube, int8_t digit);

void tube_set_dot(uint8_t tube);

void tube_refresh_all(void);

void tube_set_pwm(uint16_t pwm);

void tube_set_current_all(float current);


//display class
// maybe move this to display.h ?

typedef struct {
  uint8_t dot[DISPLAY_DIGITS_COUNT];
} display_dots;

typedef struct {
  uint8_t digit[DISPLAY_DIGITS_COUNT];
  uint8_t disabled[DISPLAY_DIGITS_COUNT];
  
  union {
    display_dots;
    display_dots dots;
  };
  
} display_digits;


typedef struct animation_state_generic animation_state_generic;


typedef struct {
  union {
    display_digits;
    
    display_digits digits;
  };
  
  animation_state_generic* animation;
} display_state;



//animations
typedef enum 
{ 
  ANIMATION_NONE = 0,
  ANIMATION_TRANSITION = 1,
  ANIMATION_SWITCH = 2,
  ANIMATION_ROLL = 3,
  
  //for settings code
  #define ANIMATIONS_COUNT 4
  
} display_animation;


extern void* animations_list[4];

struct animation_state_generic {
  display_digits previous;
  
  uint16_t step;
  uint16_t duration;
  
  void (*animation_init)(display_state* ds);
  
  display_digits* (*animation_step)(display_state* ds);
  
};
//



void display_number(uint16_t number);

void display_init_animation(display_animation ani);

void display_start_animation(void);

void display_digit(uint8_t tube, int8_t digit);

void display_set_dot(uint8_t tube, uint8_t enabled);

void display_reset_dots(void);

void display_set_tube_state(uint8_t tube, uint8_t enabled);

void display_finish_animation(display_animation ani);


void DisplayIndicationTask(void const * argument);


void DisplayIrqHandler(void);




#endif
