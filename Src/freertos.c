/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2019 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */     
#include "adc.h"
#include "rtc.h"
#include "spi.h"
#include "tim.h"
#include "usb_device.h"
#include "gpio.h"


#include "h/task_controls.h"
#include "h/tube.h"


/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
osThreadId defaultTaskHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
   
void StartClockTask(void const * argument);

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);

extern void MX_USB_DEVICE_Init(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
       
  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  
  //osThreadDef(usbTask, task_usb_read, osPriorityNormal, 0, 128);
  //osThreadCreate(osThread(usbTask), NULL);

  osThreadDef(btnTask, StartControlsTask, osPriorityNormal, 0, 256);
  osThreadCreate(osThread(btnTask), NULL);
  
  osThreadDef(displayTask, DisplayIndicationTask, osPriorityNormal, 0, 128);
  osThreadCreate(osThread(displayTask), NULL);
  
   osThreadDef(clockTask, StartClockTask, osPriorityNormal, 0, 128);
   osThreadCreate(osThread(clockTask), NULL);
 
  
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */
}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used 
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  /* init code for USB_DEVICE */
  MX_USB_DEVICE_Init();

  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
    osDelay(1);
  
    
 HAL_TIM_Base_Start_IT(&htim3);
 
 HAL_TIM_Base_Start(&htim2); 
 HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_1); 
 HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_2); 
  
  //__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 7000); //sets the PWM duty cycle 
  
  
 HAL_TIM_Base_Start(&htim4);  
 HAL_TIM_PWM_Start(&htim4,TIM_CHANNEL_1); 
 HAL_TIM_PWM_Start(&htim4,TIM_CHANNEL_2); 
  
  //__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 10000); //sets the PWM duty cycle 
  
  
  


  
  tube_reset_state();
  tube_set_digit(0, 0);
  tube_set_digit(1, 1);
  tube_set_dot(1);
  
  tube_set_digit(2, 2);
  tube_set_digit(3, 3);
  tube_refresh_all();
  
  display_init_animation(ANIMATION_ROLL);
  
  
  
    osDelay(100);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
    osDelay(100);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
    
    
    HAL_GPIO_WritePin(Boost_shutdown_GPIO_Port, Boost_shutdown_Pin, GPIO_PIN_RESET);
    
    HAL_GPIO_WritePin(Strobe_GPIO_Port, Strobe_Pin, GPIO_PIN_SET);
    osDelay(1);
    
    uint8_t n = 0;
    
    vTaskDelete( NULL );
    for(;;)
  {
    
      
      RTC_TimeTypeDef sTime = {0};
      RTC_DateTypeDef sDate = {0};
    
    if (HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK){
      Error_Handler();
    }
    
    if (HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK){
      Error_Handler();
    }
    
    tube_set_current_all(2);
    display_init_animation(ANIMATION_ROLL);
    
    tube_set_pwm(3);
    
    display_digit(0, sTime.Minutes / 10 );
    display_digit(1, sTime.Minutes % 10 );
    

    tube_set_dot(sTime.Seconds % 4);
    
    display_digit(2, sTime.Seconds / 10 );
    display_digit(3, sTime.Seconds % 10 );
    
    display_start_animation();
    
    n = (n + 1) % 10;
    //HAL_GPIO_WritePin(Strobe_GPIO_Port, Strobe_Pin, GPIO_PIN_RESET);
    osDelay(1000);
  }
  
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
     
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
