#include "main.h"
#include "cmsis_os.h"


#define BUTTON_POLL_INTERVAL 50

#define BUTTON_HOLD_THRESH 700

#define PRESSED_STATE 0


typedef struct button_t button_t;


struct button_t { 
  void* ctx;
  
  GPIO_TypeDef* port;
  uint16_t pin;
  
  uint8_t 
    prev_state:1;
    
  uint8_t hold_recall;
 
  uint16_t pressed_time;
  
  uint16_t hold_thresh;
  
  void (*onpress)( button_t * btn);
  void (*onrelease)( button_t * btn);
  
  
  int (*onhold)( button_t * btn); //if returns 1 then called again each poll cycle, only once if 0
  
  button_t *next; //linked list for processing all buttons
};


void button_init(button_t *btn, GPIO_TypeDef* port, uint8_t pin);

void button_proccess(button_t *btn);


void button_process_all(void);