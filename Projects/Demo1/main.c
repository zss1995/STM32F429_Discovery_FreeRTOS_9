/**
  ******************************************************************************
  * @file    IO_Toggle/main.c 
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    19-September-2011
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************  
  */ 

/* Includes ------------------------------------------------------------------*/
#include "stm32f429i_discovery.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"


/** @addtogroup STM32F4_Discovery_Peripheral_Examples
  * @{
  */

/** @addtogroup IO_Toggle
  * @{
  */ 

/* Private typedef -----------------------------------------------------------*/
GPIO_InitTypeDef  GPIO_InitStructure;

/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/
void Hardware_Init(void);
void Red_LED_On(void);
void Red_LED_Off(void);
void Green_LED_On(void);
void Green_LED_Off(void);
void ToggleLED1_Task(void*);
void ToggleLED2_Task(void*);
void Sender_Task(void*);
void Receiver_Task(void*);
void Monitor_Task(void*);

xQueueHandle MsgQueue; 
unsigned long SendNum;
unsigned long SendSum = 0;
unsigned long ReceiveSum = 0;

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
  /*!< At this stage the microcontroller clock setting is already configured, 
       this is done through SystemInit() function which is called from startup
       file (startup_stm32f4xx.s) before to branch to application main.
       To reconfigure the default setting of SystemInit() function, refer to
        system_stm32f4xx.c file
     */
       Hardware_Init();
 
       /* Init and start tracing*/
        vTraceEnable(TRC_INIT);
        vTraceEnable(TRC_START);
        MsgQueue=xQueueCreate(10000,sizeof(unsigned long)); /*The depth of Queue=10000*/
       /* Create tasks */
       xTaskCreate(
		  Sender_Task,			   /* Function pointer */
                  "Task_Sender",			/* Task name - for debugging only*/
                  configMINIMAL_STACK_SIZE,	    /* Stack depth in words */
                  (void*) NULL,			    /* Pointer to tasks arguments (parameter) */
                  tskIDLE_PRIORITY+4UL,             /* Task priority zuigaoyouxianji*/
                  NULL                              /* Task handle */
);

       xTaskCreate(
		  Receiver_Task,                 /* Function pointer */
		  "Task_Receiver",                          /* Task name - for debugging only*/
		  configMINIMAL_STACK_SIZE,         /* Stack depth in words */
		  (void*) NULL,                     /* Pointer to tasks arguments (parameter) */
		  tskIDLE_PRIORITY + 3UL,           /* Task priority*/
		  NULL                              /* Task handle */
       );
      xTaskCreate(
		  Monitor_Task,                 /* Function pointer */
		  "Task_Monitor",                          /* Task name - for debugging only*/
		  configMINIMAL_STACK_SIZE,         /* Stack depth in words */
		  (void*) NULL,                     /* Pointer to tasks arguments (parameter) */
		  tskIDLE_PRIORITY + 2UL,           /* Task priority*/
		  NULL                              /* Task handle */
       );

	/* Start the scheduler. */
	vTaskStartScheduler();

	/* If all is well, the scheduler will now be running, and the following line
	will never be reached.  If the following line does execute, then there was
	insufficient FreeRTOS heap memory available for the idle and/or timer tasks
	to be created.  See the memory management section on the FreeRTOS web site
	for more details. */
	for( ;; );

}


/**
 * Hardware_Init: 
 */
void Hardware_Init(void)
{
        /* GPIOG Periph clock enable */
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);

        /* Configure PG13, PG14 in output pushpull mode */
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13| GPIO_Pin_14;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
        GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
        GPIO_Init(GPIOG, &GPIO_InitStructure);

}
/**
 * Red_LED_On: 
 */
void Red_LED_On(void)
{
//    GPIO_SetBits(GPIOG, GPIO_Pin_14);
    GPIOG->ODR |= 0x4000;
}

/**
 * Red_LED_Off: 
 */
void Red_LED_Off(void)
{
//    GPIO_ResetBits(GPIOG, GPIO_Pin_14);
  GPIOG->ODR &= 0xBFFF;
}

/**
 * Green_LED_On: 
 */
void Green_LED_On(void)
{
//    GPIO_SetBits(GPIOG, GPIO_Pin_13);
    GPIOG->ODR |= 0x2000;
}

/**
 * Green_LED_Off: 
 */
void Green_LED_Off(void)
{
//    GPIO_ResetBits(GPIOG, GPIO_Pin_13);
    GPIOG->ODR &= 0xDFFF;
}
/**
 * Sender_Task: Send a number every 2 ms.
 */
void Sender_Task(void *pvParameters)
{
    SendNum=0;

    while (1) 
    {
        SendNum++;
        if(SendNum > 10000)
        {
            SendNum=1;
        }
        xQueueSend(MsgQueue,(void*)&SendNum,0);
        SendSum = SendNum + SendSum;
        vTaskDelay(2);
    }
}

/**
 * Receive_Task: Receive numbers every 1000 ms.
 */
void Receiver_Task(void *pvParameters)
{
    unsigned long ReceiveNum;

    while(1)
    {
        while(xQueueReceive(MsgQueue,&ReceiveNum,0/portTICK_RATE_MS ) == pdTRUE )
	{
	    ReceiveSum = ReceiveNum + ReceiveSum;
	}
	vTaskDelay(1000);
    }
}

/**
 * Monitor_Task: 
 */
void Monitor_Task(void *pvParameters)
{
    while(1)
    {
	if(ReceiveSum==SendSum)
          { Green_LED_On();
            Red_LED_Off();}
        else
          {Green_LED_Off();
           Red_LED_On();}
        vTaskDelay(1000);
     }
}


void vApplicationTickHook( void )
{
}
/*-----------------------------------------------------------*/


/*-----------------------------------------------------------*/

void vApplicationMallocFailedHook( void )
{
	/* vApplicationMallocFailedHook() will only be called if
	configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h.  It is a hook
	function that will get called if a call to pvPortMalloc() fails.
	pvPortMalloc() is called internally by the kernel whenever a task, queue,
	timer or semaphore is created.  It is also called by various parts of the
	demo application.  If heap_1.c or heap_2.c are used, then the size of the
	heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
	FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
	to query the size of free heap space that remains (although it does not
	provide information on how the remaining heap might be fragmented). */
	taskDISABLE_INTERRUPTS();
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook( void )
{
	/* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
	to 1 in FreeRTOSConfig.h.  It will be called on each iteration of the idle
	task.  It is essential that code added to this hook function never attempts
	to block in any way (for example, call xQueueReceive() with a block time
	specified, or call vTaskDelay()).  If the application makes use of the
	vTaskDelete() API function (as this demo application does) then it is also
	important that vApplicationIdleHook() is permitted to return to its calling
	function, because it is the responsibility of the idle task to clean up
	memory allocated by the kernel to any task that has since been deleted. */
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
	( void ) pcTaskName;
	( void ) pxTask;

	/* Run time stack overflow checking is performed if
	configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
	function is called if a stack overflow is detected. */
	taskDISABLE_INTERRUPTS();
	for( ;; );
}
/*-----------------------------------------------------------*/
#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/**
  * @}
  */ 

/**
  * @}
  */ 

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
