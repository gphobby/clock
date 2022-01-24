#include "h/button.h"


static button_t* first_btn = NULL;
static button_t* last_btn = NULL;


void button_init(button_t *btn, GPIO_TypeDef* port, uint8_t pin){
    memset(btn, 0, sizeof(button_t) );
  
    btn->port = port;
    btn->pin = pin;
    
    btn->hold_thresh = BUTTON_HOLD_THRESH;
    btn->prev_state = ! PRESSED_STATE;
    
    btn->hold_recall = 1;
    
    if(!first_btn){
      first_btn = btn;
    }
    
    if(last_btn){ // replace with search maybe?
      last_btn->next = btn;
    }
    
    last_btn = btn;
}


void button_proccess(button_t *btn){
    int current_state = HAL_GPIO_ReadPin(btn->port, btn->pin);

    
    if(current_state == PRESSED_STATE){
      
      btn->pressed_time += BUTTON_POLL_INTERVAL;

      if( btn->onpress && (btn->prev_state != current_state)){
        btn->onpress(btn);
      } 
      
      else if(btn->onhold && btn->hold_recall && (btn->pressed_time >= btn->hold_thresh) ){
        btn->hold_recall = btn->onhold(btn);
      }
      
        
    } else {
      
      if(btn->prev_state != current_state){
        btn->hold_recall = 1;
        btn->pressed_time = 0;
        
        if(btn->onrelease){
          btn->onrelease(btn);
        }
      } 
      
    }
    
    btn->prev_state = current_state;
}


void button_process_all(void){
    button_t* next = first_btn;
    
    while(next){
      button_proccess(next);
      
      next = next->next;
    }
}

/* button class */
