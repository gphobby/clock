#include "h/tube.h"
#include <math.h>

// ANIMATION NONE
animation_state_generic animation_none = {
  .animation_init = NULL,
  .animation_step = NULL,
};


// ANIMATION TRANSITION
display_digits* display_animation_transition_step(display_state* ds);

animation_state_generic animation_transition = {
  .animation_init = NULL,
  .animation_step = display_animation_transition_step,
  .duration = 10,
};


display_digits* display_animation_transition_step(display_state* ds){
  animation_state_generic* a = ds->animation;
  
  if(a->step > a->duration || a->step % 2){
    return &ds->digits;
  }else {
    return &a->previous;
  }
  
}

//ANIMATION ROLL
display_digits* display_animation_roll_step(display_state* ds);

typedef struct {
  union {
    animation_state_generic;
    animation_state_generic as_generic;
  };
  
  display_digits digits;
  
} animation_state_roll;

animation_state_roll animation_roll = {
  .animation_init = NULL,
  .animation_step = display_animation_roll_step,
  .duration = 20
};


display_digits* display_animation_roll_step(display_state* ds){
  animation_state_roll* a = (animation_state_roll*)ds->animation;
  
  
  if(a->step > a->duration){
    return &ds->digits;
  }
  
  uint16_t step = a->step / (uint16_t) ceil( a->duration / 10 );
  
  
  display_digits* curr = &ds->digits;
  display_digits* prev = &a->previous;
  display_digits* out = &a->digits;
  
  for(uint8_t i = 0; i < DISPLAY_DIGITS_COUNT; i++){ 
    
    if(curr->digit[i] != prev->digit[i]){
      
      out->digit[i] = (ds->digits.digit[i] + step) % 10;
      
    } else {
      out->digit[i] = curr->digit[i];
    }
      
  }
  
  out->dots = curr->dots;
  
  return out;
}




//ANIMATION SWITCH

void display_animation_switch_init(display_state* ds);
display_digits* display_animation_switch_step(display_state* ds);

typedef struct {
  union {
    animation_state_generic;
    animation_state_generic as_generic;
  };
  
  uint16_t cnt;
  
  uint16_t curr_period;
  uint16_t prev_period;
} animation_state_switch;



animation_state_switch animation_switch = {
  .animation_init = display_animation_switch_init,
  .animation_step = display_animation_switch_step,
  .duration = 20,
};




void display_animation_switch_init(display_state* ds){
  animation_state_switch* state = (animation_state_switch*) ds->animation;
  
  state->curr_period = 4;
  state->prev_period = 4;
}  

display_digits* display_animation_switch_step(display_state* ds){
  animation_state_switch* a = (animation_state_switch*) ds->animation;
  
  
  a->cnt++;
  
  uint8_t ret_curr_d = 1;
  
  if(a->curr_period > 1){
    
    if(a->cnt % a->curr_period){
      ret_curr_d = 1;
    } else {
      ret_curr_d = 0;
      
      a->cnt = 0; 
      a->curr_period--;
    }
      
  }else if(a->prev_period > 1){
    
    if(a->cnt % a->prev_period){
      ret_curr_d = 0;
    } else {
      ret_curr_d = 1;
      
      a->cnt = 0; 
      a->prev_period--;
    }
    
  }else {
    ret_curr_d = 1;
  }
  
  if(ret_curr_d){
    return &ds->digits;
  }else {
    return &a->previous;
  }
  
  
}       



 // ANIMATION_NONE = 0,
 // ANIMATION_TRANSITION = 1,
 // ANIMATION_SWITCH = 2,
 // ANIMATION_ROLL = 3,
  
void* animations_list[4] = {
  &animation_none,
  &animation_transition,
  &animation_switch,
  &animation_roll,
};

