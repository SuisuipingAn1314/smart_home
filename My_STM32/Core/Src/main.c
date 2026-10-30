/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "sht3x.h"
#include <stdio.h>
#include <string.h>
#include "light_sensor.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define DEV_ID "hyt00001"
extern uint8_t rx_data[1]; // 【新增这行】声明外部变量
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_USART2_UART_Init();
  MX_TIM4_Init();
  MX_I2C2_Init();
  /* USER CODE BEGIN 2 */
	// 强制在初始化时关闭所有 LED
  HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin | LED3_Pin, GPIO_PIN_SET); // 2、3 拉高熄灭
  HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);     
	
	
	
  HAL_ADCEx_Calibration_Start(&hadc1);
  __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_4, 500);
  SHT3X_Init();
  Light_Init();

  // --- 第一阶段：初始化 ESP8266 ---
  uint8_t at_cmd[] = "AT\r\n";
  HAL_UART_Transmit(&huart2, at_cmd, strlen((char *)at_cmd), 1000);
  HAL_Delay(1000);

  uint8_t cwmode_cmd[] = "AT+CWMODE=1\r\n";
  HAL_UART_Transmit(&huart2, cwmode_cmd, strlen((char *)cwmode_cmd), 1000);
  HAL_Delay(1000);

  uint8_t cwjap_cmd[] = "AT+CWJAP=\"Hytos\",\"heibmj839\"\r\n";
  HAL_UART_Transmit(&huart2, cwjap_cmd, strlen((char *)cwjap_cmd), 1000);
  HAL_Delay(8000); 

  uint8_t cipstart_cmd[] = "AT+CIPSTART=\"TCP\",\"sspeace.top\",9413\r\n";
  HAL_UART_Transmit(&huart2, cipstart_cmd, strlen((char *)cipstart_cmd), 1000);
  HAL_Delay(3000); 

  uint8_t cipmode_cmd[] = "AT+CIPMODE=1\r\n";
  HAL_UART_Transmit(&huart2, cipmode_cmd, strlen((char *)cipmode_cmd), 1000);
  HAL_Delay(500);

  uint8_t cipsend_cmd[] = "AT+CIPSEND\r\n";
  HAL_UART_Transmit(&huart2, cipsend_cmd, strlen((char *)cipsend_cmd), 1000);
  HAL_Delay(500); 

  // --- 第二阶段：进入透传后，开启 DMA 接收指令 ---
  HAL_UART_Receive_DMA(&huart2, rx_data, 1);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  float temp, humi;
  uint8_t sound_data; 
  uint16_t light_data;		
  char upload_data[100]; 

  while (1)
  {
    if (!SHT3X_Get(&temp, &humi))
    {
      snprintf(upload_data, sizeof(upload_data), "%s/sensor/sht30 %.2f_%.2f\n", DEV_ID, temp, humi);
      HAL_UART_Transmit(&huart2, (uint8_t *)upload_data, strlen(upload_data), 1000);
    }

    sound_data = HAL_GPIO_ReadPin(SOUND_SENSOR_GPIO_Port, SOUND_SENSOR_Pin);
    snprintf(upload_data, sizeof(upload_data), "%s/sensor/sound %u\n", DEV_ID, sound_data);
    HAL_UART_Transmit(&huart2, (uint8_t *)upload_data, strlen(upload_data), 1000);

    light_data = Light_Get(1);
    snprintf(upload_data, sizeof(upload_data), "%s/sensor/light %u\n", DEV_ID, light_data);
    HAL_UART_Transmit(&huart2, (uint8_t *)upload_data, strlen(upload_data), 1000);
		
    HAL_Delay(2000); 
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
