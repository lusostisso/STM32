/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
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

/* USER CODE BEGIN PV */
#define TIMEOUT 10000 //TIMEOUT DO CALLBACK
#define TAM_COMANDO 2 //TAMANHO DO INDICE DO COMANDO CASO 2, PQ EH 1 LETRA E 1 NUMERO
#define DISTANCIA 1 //INDICE QUE REPRESENTA A DISTANCIA QUE O USER DIGITAR
#define CH 0 //INDICE DA DIREÇÃO QUE O USER DIGITAR
#define TAM_MENSAGEM 100 //TAMANHO DE RETORNO DA SERIAL
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
//Variaveis Movimento do Carro
uint8_t mensagem [TAM_MENSAGEM] = {"Carro LUD a postos \r\n"}; //mensagem inicial serial
uint8_t comando [TAM_COMANDO] ; //comando do usuario
uint32_t novoARR = 0 ; //variavel que vai ser definida, para determinar o tempo
HAL_StatusTypeDef status; //retorno do recebimento da serial
uint16_t distancia_definida, razao, tempo_definido, duty_cicle;

//Varieveis Farois
int16_t count = 0;
uint32_t counter, valor_atualizado;

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
  MX_USART2_UART_Init();
  MX_TIM3_Init();
  MX_TIM2_Init();
  MX_TIM10_Init();
  MX_TIM4_Init();
  MX_TIM1_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_Encoder_Start_IT(&htim4, TIM_CHANNEL_ALL);
  HAL_TIM_PWM_Start(&htim10, TIM_CHANNEL_1);
  HAL_UART_Transmit(&huart2, mensagem, strlen(mensagem), TIMEOUT);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  while (1){
	status = HAL_UART_Receive(&huart2, comando, TAM_COMANDO, TIMEOUT);
	if(status == HAL_OK){ //sucesso no  recebimento do comando, vale 0
	razao = comando[DISTANCIA] - '0';
	if(razao>= 0 || razao<= 9){ //dentro dos parametros
		tempo_definido = 2 * (razao*10) / 20;
		novoARR = ((16000000*tempo_definido) / 16001) ;
	 	}
	 else { comando[DISTANCIA] = 'X'; }
	distancia_definida = razao*10;
	comando[CH] = toupper(comando[CH]);
	switch (comando[CH]){
		case 'F':
		sprintf(mensagem, "Carro LUD andando %d cm para frente \r\n", distancia_definida);
		__HAL_TIM_SET_AUTORELOAD(&htim2, novoARR);
		HAL_TIM_Base_Start_IT(&htim2);

		andarPraFrente();
		break;

		case 'D':
		sprintf(mensagem, "Carro LUD andando %d cm para direita \r\n", distancia_definida);
		__HAL_TIM_SET_AUTORELOAD(&htim2, novoARR);
		//virar para direita
		HAL_TIM_OC_Start(&htim1, TIM_CHANNEL_1);
		HAL_TIM_Base_Start_IT(&htim3);
		andarPraDireita();
		break;

		case 'E':
			sprintf(mensagem, "Carro LUD andando %d cm para esquerda \r\n", distancia_definida);
			__HAL_TIM_SET_AUTORELOAD(&htim2, novoARR);
			//virar para direita
			HAL_TIM_Base_Start_IT(&htim3);
			HAL_TIM_OC_Start(&htim1, TIM_CHANNEL_2);
			andarPraEsquerda();
			break;

		case 'R':
			sprintf(mensagem, "Carro LUD andando %d cm para trás \r\n", distancia_definida);
			__HAL_TIM_SET_AUTORELOAD(&htim2, novoARR);
			HAL_TIM_Base_Start_IT(&htim2);
			andarPraTras();
			break;

		default:
			strncpy(mensagem, "Direcao nao encontrada \r\n", TAM_MENSAGEM);
			break;
		}
	 	HAL_UART_Transmit(&huart2, mensagem, strlen(mensagem), TIMEOUT);
		status = 0;

  }

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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void HAL_TIM_PeriodElapsedCallback (TIM_HandleTypeDef *htim ){
	if(htim ==  &htim3){
		HAL_TIM_Base_Start_IT(&htim2);
		andarPraFrente();
		HAL_TIM_Base_Stop(&htim3);
		HAL_TIM_OC_Stop(&htim1, TIM_CHANNEL_1);
		HAL_TIM_OC_Stop(&htim1, TIM_CHANNEL_2);
	}
	if (htim == &htim2){
		parar();
		auxiliar=0;
		HAL_TIM_Base_Stop(&htim2);
	}
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim){
	if(htim==&htim4){
	counter = __HAL_TIM_GET_COUNTER(htim);
	count = (int16_t)counter;
	count = count/4; //conta a borda de subida e descida dos dois timer, ent divide por 4
	if(count>35){
		count=35;
	}
	else if (count < 0){
		count =0;
	}
	duty_cicle = (count * 1000 / 35) ;
	//controle de brilho de farol
	__HAL_TIM_SET_COMPARE(&htim10, TIM_CHANNEL_1, duty_cicle);
	}
}
void andarPraFrente () {
	//RODA ESQUERDA
	HAL_GPIO_WritePin(GPIOA, FIO_CINZA_ESQ_Pin, 1);
	HAL_GPIO_WritePin(GPIOA, FIO_BRANCO_ESQ_Pin, 0);
	//RODA DIREITA
	HAL_GPIO_WritePin(GPIOB, FIO_ROSA_DIR_Pin, 0);
	HAL_GPIO_WritePin(GPIOB, FIO_AZUL_DIR_Pin, 1);


}
void andarPraDireita (){
	//RODA ESQUERDA
	HAL_GPIO_WritePin(GPIOA, FIO_CINZA_ESQ_Pin, 1);
	HAL_GPIO_WritePin(GPIOA, FIO_BRANCO_ESQ_Pin, 1);
	//RODA DIREITA
	HAL_GPIO_WritePin(GPIOB, FIO_ROSA_DIR_Pin, 0);
	HAL_GPIO_WritePin(GPIOB, FIO_AZUL_DIR_Pin, 1);
}
void andarPraTras (){
	//RODA ESQUERDA
		HAL_GPIO_WritePin(GPIOA, FIO_CINZA_ESQ_Pin, 0);
		HAL_GPIO_WritePin(GPIOA, FIO_BRANCO_ESQ_Pin, 1);
		//RODA DIREITA
		HAL_GPIO_WritePin(GPIOB, FIO_ROSA_DIR_Pin, 1);
		HAL_GPIO_WritePin(GPIOB, FIO_AZUL_DIR_Pin, 0);
}
void andarPraEsquerda (){
	//RODA ESQUERDA
	HAL_GPIO_WritePin(GPIOA, FIO_CINZA_ESQ_Pin, 1);
	HAL_GPIO_WritePin(GPIOA, FIO_BRANCO_ESQ_Pin, 0);
	//RODA DIREITA
	HAL_GPIO_WritePin(GPIOB, FIO_ROSA_DIR_Pin, 1);
	HAL_GPIO_WritePin(GPIOB, FIO_AZUL_DIR_Pin, 1);
}
void parar () {
	HAL_GPIO_WritePin(GPIOA, FIO_CINZA_ESQ_Pin, 1);
	HAL_GPIO_WritePin(GPIOA, FIO_BRANCO_ESQ_Pin, 1);
	HAL_GPIO_WritePin(GPIOB, FIO_ROSA_DIR_Pin, 1);
	HAL_GPIO_WritePin(GPIOB, FIO_AZUL_DIR_Pin, 1);
}
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

#ifdef  USE_FULL_ASSERT
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

