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





uint16_t is_refreshing = 0;


uint32_t getUs(void) {

uint32_t usTicks = HAL_RCC_GetSysClockFreq() / 1000000;

 

register uint32_t ms, cycle_cnt;
 

do {
 

ms = HAL_GetTick();
 

cycle_cnt = SysTick->VAL;
 

} while (ms != HAL_GetTick());
 

return (ms * 1000) + (usTicks * 1000 - cycle_cnt) / usTicks;
 

}
void delayUs(uint16_t micros) {

 

uint32_t start = getUs();
 

while (getUs()-start < (uint32_t) micros) {
}
 

}



