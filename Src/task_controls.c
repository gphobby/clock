#include "main.h"
#include "cmsis_os.h"
#include "adc.h"
#include "gpio.h"
#include "rtc.h"

#include "math.h"
#include "h/tube.h"


#include "eeprom.h"





void btn_onpress( button_t * btn ){
  char *str = (char *)btn->ctx;
  CDC_Transmit_FS(str, strlen(str) );

  CDC_Transmit_FS("\n", 1);
}






typedef enum 
{ 
  DISPLAY_TIME,
  
  SET_TIME_HR_BLINK,
  SET_TIME_MIN_BLINK,
  SET_TIME_SEC_BLINK,
  
  SET_ANIMATION,
  
  SET_BRIGHTNESS,
  
} clock_state;

void clk_go_SET_TIME_HR_BLINK(void);

void clk_go_SET_TIME_MIN_BLINK(void);

void clk_go_DISPLAY_TIME(void);



clock_state clk_state = DISPLAY_TIME;




typedef struct {
  int8_t hr_inc;
  int8_t min_inc;
  int8_t sec_inc;
  
} action_btn_change_time_ctx;

#define CLOCK_SET_INCR_SPEED 50


void action_btn_change_time( button_t * btn ){
  action_btn_change_time_ctx *ctx = (action_btn_change_time_ctx *)btn->ctx;
  
  RTC_TimeTypeDef sTime = {0};
  
  if (HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK){
      return;
  }
  
  
  if( (sTime.Hours + ctx->hr_inc) < 0 ){
    sTime.Hours = 23;
  }else {
    sTime.Hours += ctx->hr_inc;
  }
  
  if( (sTime.Minutes + ctx->min_inc) < 0 ){
    sTime.Minutes = 59;
  }else {
    sTime.Minutes += ctx->min_inc;
  }
  
  if( (sTime.Seconds + ctx->sec_inc) < 0 ){
    sTime.Seconds = 59;
  }else {
    sTime.Seconds += ctx->sec_inc;
  }
  
  

  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }
  
}

static uint32_t changetime_is_held = 0;

int action_btn_change_time_hold( button_t * btn ){
  action_btn_change_time(btn);
    
  changetime_is_held = 1;
  
  return 50;
}

void action_btn_change_time_release( button_t * btn ){
  changetime_is_held = 0;
}



#define ENABLED 1


button_t btn_blue, btn_black, btn_red;

action_btn_change_time_ctx hr_inc_ctx = {
  .hr_inc = 1,
  .min_inc = 0,
  .sec_inc = 0,
};

action_btn_change_time_ctx hr_dec_ctx = {
  .hr_inc = -1,
  .min_inc = 0,
  .sec_inc = 0,
};

action_btn_change_time_ctx min_inc_ctx = {
  .hr_inc = 0,
  .min_inc = 1,
  .sec_inc = 0,
};

action_btn_change_time_ctx min_dec_ctx = {
  .hr_inc = 0,
  .min_inc = -1,
  .sec_inc = 0,
};

//add sec here



#define btn_SET_TIME_enter btn_red
#define btn_SET_TIME_inc btn_black
#define btn_SET_TIME_dec btn_blue

void btncb_SET_TIME_setbtn_onpress( button_t * btn ){
  //switch between hr min sec setting modes on press
  
  switch ( clk_state ){
  case SET_TIME_HR_BLINK:
      clk_go_SET_TIME_MIN_BLINK();
    break;
    
  case SET_TIME_MIN_BLINK:
      clk_go_SET_TIME_HR_BLINK();
    break;
    
    //add minutes here
    
    default: 
       clk_go_DISPLAY_TIME(); //shouldn't get here...
  }
}


int btncb_SET_TIME_setbtn_onhold( button_t * btn ){
  // on first hold enter hoours set mode and set onpress handler
  if(clk_state == DISPLAY_TIME){
    
    // switch setmode on press
    btn_SET_TIME_enter.onpress = btncb_SET_TIME_setbtn_onpress;
    
    //starting from hour set mode
    clk_go_SET_TIME_HR_BLINK();
    
    
  // exit settings mode on second hold
  }else {
    changetime_is_held = 0;
    
    clk_go_DISPLAY_TIME();
  }
  
  return 0;
}


void clk_SET_TIME_set_btn_cb(void){
  btn_SET_TIME_inc.onpress = action_btn_change_time;
  btn_SET_TIME_dec.onpress = action_btn_change_time;
  
  btn_SET_TIME_inc.onhold = action_btn_change_time_hold;
  btn_SET_TIME_dec.onhold = action_btn_change_time_hold;
  
  btn_SET_TIME_inc.onrelease = action_btn_change_time_release;
  btn_SET_TIME_dec.onrelease = action_btn_change_time_release; 
  
  //maybe sec here...
}

void clk_go_SET_TIME_HR_BLINK(void){
    clk_state = SET_TIME_HR_BLINK;
      
    clk_SET_TIME_set_btn_cb();
    
    btn_SET_TIME_inc.ctx = (void*) &hr_inc_ctx;
    btn_SET_TIME_dec.ctx = (void*) &hr_dec_ctx; 
    
    
}

void clk_go_SET_TIME_MIN_BLINK(void){
    clk_state = SET_TIME_MIN_BLINK;
    
    clk_SET_TIME_set_btn_cb();
    
    btn_SET_TIME_inc.ctx = (void*) &min_inc_ctx;
    btn_SET_TIME_dec.ctx = (void*) &min_dec_ctx; 
}


int btncb_SET_ANIMATION_modebtn_onhold( button_t * btn );
int btncb_SET_BRIGHTNESS_modebtn_onhold( button_t * btn );


void clk_go_DISPLAY_TIME(void){
    clk_state = DISPLAY_TIME;
    
    // shouldn't react on first press, enter setting mode only on hold
    // btn_red
    btn_SET_TIME_enter.onpress = NULL;
    btn_SET_TIME_enter.onhold = btncb_SET_TIME_setbtn_onhold;
    
    btn_black.onpress = NULL;
    btn_black.onhold = btncb_SET_BRIGHTNESS_modebtn_onhold;
    
    btn_blue.onpress = NULL;
    btn_blue.onhold = btncb_SET_ANIMATION_modebtn_onhold;
    
    
    //make sure all tubes are enabled if some were blinking
    for(uint8_t i = 0; i < DISPLAY_DIGITS_COUNT; i++){ 
      display_set_tube_state(i, 1);
    }
}




typedef struct {
  uint16_t max;
  uint16_t min;
} dimming_level;

typedef enum {
  DOTS_NONE,
  DOTS_R2L,
  DOTS_L2R,
  DOTS_C_OUT,
  DOTS_C_IN,
  
  DOTS_C_BLINK,
  DOTS_RANDOM,
  
  #define DOTS_ANIMATIONS_COUNT 7
  
} dot_animation;


typedef struct {
  #define CLOCK_CFG_IS_INIT 0xFF335577
  uint32_t is_init_flag;
  
  dimming_level dimm_level;
  
  display_animation animation;
  
  dot_animation dots_mode;
  
  uint32_t dummy;
} clock_cfg_t;


clock_cfg_t clock_cfg = {
  .is_init_flag = CLOCK_CFG_IS_INIT,
  
  .animation = ANIMATION_NONE,
  
  .dimm_level = {
    .max = 10,
    .min = 2,
  },
};


void clk_write_cfg(void);

void clk_read_cfg(void){
  clock_cfg_t clock_cfg_read;
  
  EE_Reads(0, sizeof(clock_cfg_read)/4, (uint32_t*)&clock_cfg_read);
  
  if(clock_cfg_read.is_init_flag != CLOCK_CFG_IS_INIT){
    clk_write_cfg();
    
    return;
  }
  
  clock_cfg = clock_cfg_read;
}

void clk_write_cfg(void){
  
  EE_Format();
  EE_Writes(0, sizeof(clock_cfg)/4, (uint32_t*)&clock_cfg);
}




void clk_go_SET_ANIMATION(void);

void btncb_SET_ANIMATION_setbtn_onpress_next( button_t * btn ){
  clock_cfg.animation = (clock_cfg.animation + 1)% ANIMATIONS_COUNT;
}

void btncb_SET_ANIMATION_DOTS_setbtn_onpress_next( button_t * btn ){
  clock_cfg.dots_mode = (clock_cfg.dots_mode + 1) % DOTS_ANIMATIONS_COUNT;
}


void btncb_SET_ANIMATION_setbtn_onpress_prev( button_t * btn ){
  int8_t val = clock_cfg.animation - 1;
  
  if(val < 0){
    val = ANIMATIONS_COUNT - 1;
  }
  
  clock_cfg.animation = val;
}

int btncb_SET_ANIMATION_modebtn_onhold( button_t * btn ){
  if(clk_state == DISPLAY_TIME){
    clk_go_SET_ANIMATION();
  } else {
    
    //save config before exiting
    clk_write_cfg();
    
    
    clk_go_DISPLAY_TIME();
  }
  
  return 0;
}

void clk_go_SET_ANIMATION(void){
    clk_state = SET_ANIMATION;
    
    btn_blue.onpress = NULL;
    btn_blue.onhold = btncb_SET_ANIMATION_modebtn_onhold;

    //btn_black.onpress = btncb_SET_ANIMATION_setbtn_onpress_prev;
    btn_black.onpress = btncb_SET_ANIMATION_DOTS_setbtn_onpress_next;
    
    btn_black.onhold = NULL;
    
    
    
    btn_red.onpress = btncb_SET_ANIMATION_setbtn_onpress_next;
    btn_red.onhold = NULL;
    
    
    //make sure all tubes are enabled if some were blinking
    for(uint8_t i = 0; i < DISPLAY_DIGITS_COUNT; i++){ 
      display_set_tube_state(i, 1);
    }
}


typedef enum {
  BRIGHTNESS_SHOW_MIN = 0,
  BRIGHTNESS_SHOW_MAX = 1,
  
  BRIGHTNESS_SHOW_CURRENT = 2,
  
} brighntess_menu_state_t;

brighntess_menu_state_t brightness_menu_state = BRIGHTNESS_SHOW_MIN;


uint16_t clk_get_adc_light_pwm_level(void){
  HAL_ADC_Start(&hadc1);

  uint32_t value = 0; 
  if (HAL_ADC_PollForConversion(&hadc1, 1000000) == HAL_OK)
  {
       value = HAL_ADC_GetValue(&hadc1);
  }

  uint32_t light_level = (4096 - value);

  light_level =  light_level / (  4096/100  );
  // 100 - very bright room
  // 80 - is light room
  // 50 is moderately light ( evening sun trough windows ) 
  // 0 is dark
  
  if(light_level > 90){
    light_level = 100;
  }
  
  uint16_t pwm_level = 
    ( light_level * ( clock_cfg.dimm_level.max - clock_cfg.dimm_level.min )/90) + clock_cfg.dimm_level.min;

  return pwm_level;
}


void btncb_SET_BRIGHTNESS_modebtn_onpress( button_t * btn ){
  brightness_menu_state = (brightness_menu_state + 1) % 3;
}

void btncb_SET_BRIGHTNESS_setbtn_onpress( button_t * btn ){
   int8_t inc = (int8_t) btn->ctx;
  
   uint16_t *levelptr;
   
   if(brightness_menu_state == BRIGHTNESS_SHOW_MIN){
       levelptr = &clock_cfg.dimm_level.min;
   }
    
   if(brightness_menu_state == BRIGHTNESS_SHOW_MAX){
       levelptr = &clock_cfg.dimm_level.max;
   }
   
   
   int16_t new_val = *levelptr + inc;
   
   if(new_val < 0){
    new_val = 0;
   }
   
   if(new_val > 20){
    new_val = 20;
   }
   
   *levelptr = new_val;
   
   if(clock_cfg.dimm_level.min > clock_cfg.dimm_level.max){
    clock_cfg.dimm_level.max = clock_cfg.dimm_level.min;
   } 
   
}

void clk_go_SET_BRIGHTNESS(void);

int btncb_SET_BRIGHTNESS_modebtn_onhold( button_t * btn ){
  if(clk_state == SET_BRIGHTNESS){
    //save config before exiting
    clk_write_cfg();
    
    clk_go_DISPLAY_TIME();
  }else {
    clk_go_SET_BRIGHTNESS();
  }
  
  return 0;
}


void clk_go_SET_BRIGHTNESS(void){
    clk_state = SET_BRIGHTNESS;
    
    btn_blue.onpress = btncb_SET_BRIGHTNESS_setbtn_onpress;
    btn_blue.onhold = NULL;
    btn_blue.ctx = (void *) -1; // decr
    
    btn_black.onpress = btncb_SET_BRIGHTNESS_modebtn_onpress;
    btn_black.onhold = btncb_SET_BRIGHTNESS_modebtn_onhold;
    
    btn_red.onpress = btncb_SET_BRIGHTNESS_setbtn_onpress;
    btn_red.onhold = NULL;
    btn_red.ctx = (void *) 1; // incr
    
}


static uint32_t rndseed = 355;
uint32_t get_random(void) {
  rndseed ^= (rndseed << 27);
  rndseed ^= (rndseed >> 14);
  rndseed ^= (rndseed << 4);
  return rndseed;
}



void StartClockTask(void const * argument)
{
  tube_set_current_all(2);
  
  display_number(0);
  
  #define CLOCK_DISPLAY_INTERVAL 100
  
  clk_read_cfg();
  
  clk_go_DISPLAY_TIME();
  
  
  
  for(;;){
    static uint16_t tick = 0;
    
    osDelay(CLOCK_DISPLAY_INTERVAL); //don't move this to end of cycle, or "continue" will skip waiting
    tick += CLOCK_DISPLAY_INTERVAL;
    
    
    if(tick > 10000){ // power of ten based overflow, so code that rely on modulus == 0 doesnt break on overflow
      tick = 0;
    }
    
    uint8_t blink_state = tick % 200 < 100;
    if(changetime_is_held){
      blink_state = 1;
    }
    
    display_init_animation(clock_cfg.animation);
    
    
    uint16_t dots_animation_source;
    
    switch(clk_state){

      case SET_TIME_HR_BLINK:
        display_init_animation(ANIMATION_NONE);
        
        display_set_tube_state(0, blink_state);
        display_set_tube_state(1, blink_state);
        
        display_set_tube_state(2, 1);
        display_set_tube_state(3, 1);
        
        
        goto display_time_label;
     
      case SET_TIME_MIN_BLINK:
        display_init_animation(ANIMATION_NONE);
        
        display_set_tube_state(0, 1);
        display_set_tube_state(1, 1);
        
        display_set_tube_state(2, blink_state);
        display_set_tube_state(3, blink_state);
        
        goto display_time_label;
        
      case SET_ANIMATION:
        if( (tick % 300) != 0 ) { 
          continue;
        };
        
        display_reset_dots();
        display_number(tick / 200);
        
        dots_animation_source = tick / 300;
        
        goto display_dots_label;
      break;
      

    case SET_BRIGHTNESS:
        display_reset_dots();
        

        
        if(brightness_menu_state == BRIGHTNESS_SHOW_CURRENT){
          uint16_t pwm = clk_get_adc_light_pwm_level();
          tube_set_pwm(pwm);
          
          display_set_dot(1, 1);
          display_set_dot(2, 1);
          
          display_digit(0, -1 );
          display_digit(1, pwm / 10 );
          display_digit(2, pwm % 10 );
          display_digit(3, -1 );
          
        } else {
          
          if(brightness_menu_state == BRIGHTNESS_SHOW_MIN){
            tube_set_pwm(clock_cfg.dimm_level.min);
            
            display_set_dot(3, 1);
            display_set_dot(2, 1);
          }
          
          if(brightness_menu_state == BRIGHTNESS_SHOW_MAX){
            tube_set_pwm(clock_cfg.dimm_level.max);
            
            display_set_dot(0, 1);
            display_set_dot(1, 1);
          }
          
          display_digit(0, clock_cfg.dimm_level.max / 10 );
          display_digit(1, clock_cfg.dimm_level.max % 10 );
          
          display_digit(2, clock_cfg.dimm_level.min / 10 );
          display_digit(3, clock_cfg.dimm_level.min % 10 );
          
        }
        

      
      break;
      
      default: //DISPLAY_TIME
        
        
        display_time_label:;
        
        //run every 500ms to let animation finish between displaying cycles
        if( (tick % 500) != 0 ) { 
          continue;
        };
        
      uint16_t pwm = clk_get_adc_light_pwm_level();
      tube_set_pwm(pwm);
        
      RTC_TimeTypeDef sTime = {0};
      if (HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK){
        continue;
      }
      
      
      
      display_digit(0, sTime.Hours / 10 );
      display_digit(1, sTime.Hours % 10 );
      
      display_digit(2, sTime.Minutes / 10 );
      display_digit(3, sTime.Minutes % 10 );
      
      
      //display_digit(3, sTime.Seconds / 10 );
      //display_digit(4, sTime.Seconds % 10 );
      
      dots_animation_source = sTime.Seconds;
      
      display_dots_label:
      
      display_reset_dots();
        
      switch(clock_cfg.dots_mode){
          case DOTS_L2R:
            display_set_dot(dots_animation_source % 4, 1);
          break;
          
          case DOTS_R2L:
            display_set_dot( 3 - (dots_animation_source % 4) , 1);
          break;
         
          
          uint8_t dot_center_phase;
          
          case DOTS_C_IN:
            dot_center_phase = 2 - (dots_animation_source % 3);
            
            goto label_dots_center_animation;
            
          case DOTS_C_OUT:
            dot_center_phase = (dots_animation_source % 3);
           
            label_dots_center_animation:
            
            switch(dot_center_phase){
              case 0:
                display_set_dot(1, 1);
                display_set_dot(2, 1);
              break;
              
              case 1:
                display_set_dot(0, 1);
                display_set_dot(3, 1);
              break;
              
              case 2:
                
              break;
            }
            
          break;
          
          case DOTS_C_BLINK:
            uint8_t centerblink_state = (dots_animation_source % 2);
            display_set_dot( 1, centerblink_state);
            display_set_dot( 2, centerblink_state);
          break;
          
          
          case DOTS_RANDOM:
            static uint16_t prev_dots_animation_source;
            
            //ontransition
            if( prev_dots_animation_source != dots_animation_source ){
              get_random();
              
              prev_dots_animation_source = dots_animation_source;
            }
            
            //display_set_dot(rndseed % 4, 1);
            
            display_set_dot(0 , rndseed & (1 << 0) );
            display_set_dot(1 , rndseed & (1 << 1) );
            display_set_dot(2 , rndseed & (1 << 2) );
            display_set_dot(3 , rndseed & (1 << 3) );
            
          break;
          
          case DOTS_NONE:
          default:
            ;
      }
        
      
      
      display_start_animation();
      
      
      break;
    }
    
  }
}






void StartControlsTask(void const * argument)
{
  //vTaskDelete( NULL );
  
  
  button_init(&btn_blue, GPIOA, GPIO_PIN_3);
  button_init(&btn_black, GPIOA, GPIO_PIN_1);
  button_init(&btn_red, GPIOA, GPIO_PIN_2);
  
  clk_go_DISPLAY_TIME(); // init btns
  
  
  
  
 
  for(;;)
  {
      button_process_all();
      
      osDelay(BUTTON_POLL_INTERVAL);
  }
}
