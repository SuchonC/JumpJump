/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

int game_state = 0; // 0 = start_scene, 1 = playing, 2 = dead_scene
int obs_row_begin = 20; int obs_col_begin = 70;
int plyr_row_begin = 16; int plyr_col_begin = 6;

int obs_row = 20; int obs_col = 70;
int plyr_row = 16; int plyr_col = 6;

int is_jumping = 0;
int jump_height = 10;
int max_jump_height = 0; // set in init frame function
int going_up = 1;

int score = 0;

int is_plyr_green = 1;
uint8_t _clear_screen[4] = "\033[2J";
uint8_t _player_yellow[88] =  "\033[1;33m"
					   " xxxx \n\033[6D"
					   " xxxx \n\033[6D"
					   "  xx  \n\033[6D"
					   "xxxxxx\n\033[6D"
					   "  xx  \n\033[6D"
					   " x  x \n\033[6D"
					   " x  x \n\033[6D"
					   "\033[0m";
uint8_t _player_jump_yellow[88] =  "\033[1;33m"
					   " xxxx \n\033[6D"
					   " xxxx \n\033[6D"
					   "x xx x\n\033[6D"
					   " xxxx \n\033[6D"
					   "  xx  \n\033[6D"
					   " x  x \n\033[6D"
					   "x    x\n\033[6D"
					   "\033[0m";
uint8_t _player_green[88] =  "\033[1;32m"
					   " xxxx \n\033[6D"
					   " xxxx \n\033[6D"
					   "  xx  \n\033[6D"
					   "xxxxxx\n\033[6D"
					   "  xx  \n\033[6D"
					   " x  x \n\033[6D"
					   " x  x \n\033[6D"
					   "\033[0m";
uint8_t _player_jump_green[88] =  "\033[1;32m"
					   " xxxx \n\033[6D"
					   " xxxx \n\033[6D"
					   "x xx x\n\033[6D"
					   " xxxx \n\033[6D"
					   "  xx  \n\033[6D"
					   " x  x \n\033[6D"
					   "x    x\n\033[6D"
					   "\033[0m";
uint8_t _player_remove[77] = "      \n\033[6D"
							 "      \n\033[6D"
							 "      \n\033[6D"
							 "      \n\033[6D"
							 "      \n\033[6D"
							 "      \n\033[6D"
							 "      \n\033[6D";
uint8_t _obs[41] =  "\033[1;31m"
					"  +  \n\033[5D"
				    " +++ \n\033[5D"
				    "+++++\n\033[5D"
				    "\033[0m";
uint8_t _obs_remove[30] =  "     \n\033[5D"
						   "     \n\033[5D"
			 	  	       "     \n\033[5D";
uint8_t _floor[320] = "################################################################################\n\033[80D"
					  "################################################################################\n\033[80D"
					  "################################################################################";
uint8_t _end_scene[100] = "\n\033[1;31mYOU ARE DEAD !\n\033[0m\033[22D"
						 "\nPress ENTER to play again...";
uint8_t _start_scene[100] = "\033[1;32mWelcome to JUMPJUMP !!\n\033[0m\033[22D"
							"\nPress ENTER to play.";
uint8_t _red_jumpjump[50] = "\033[31mJUMPJUMP\033[0m";
uint8_t _green_jumpjump[50] = "\033[32mJUMPJUMP\033[0m";
uint8_t _warning[100] = "Hit SPACE BAR to jump\n\033[21D"
						"Press P to pause";
uint8_t _set_size[10] = "\033[8;25;80t";
uint8_t _pause_message[100] = "\033[1;37mPAUSE\033[0m";
uint8_t _pause_message_remove[100] = "     ";

uint8_t temp[50] = ""; // used at disp dead scene
uint8_t temp2; // used at disp start scene
uint8_t temp3[50] = "";
int temp_count = 0; // used at disp start scene

uint8_t keyboardInput;

// used in main
int obs_rate_begin = 50000;
int plyr_rate_begin = 40000;
int obs_rate_limit = 20000;

int plyr_frame = 0;
int obs_frame = 0;
int obs_rate = 0;
int plyr_rate = 0;
int has_init_frame = 0;
int has_show_end_scene = 0;
int has_show_start_scene = 0;

void set_cur_to(int row, int col){
	uint8_t buffer[8] = "";
	sprintf(buffer, "\033[%d;%dH", row, col);
	HAL_UART_Transmit(&huart2, buffer, sizeof(buffer), HAL_MAX_DELAY);
}

// set screen size to 80x25
void set_size(){
	 HAL_UART_Transmit(&huart2, _set_size, sizeof(_set_size), HAL_MAX_DELAY);
}

void clear_screen(){
	HAL_UART_Transmit(&huart2, _clear_screen, 4, HAL_MAX_DELAY);
}

void draw_floor(){
	set_cur_to(23, 0);
	HAL_UART_Transmit(&huart2, _floor, sizeof(_floor), HAL_MAX_DELAY);
}

void remove_obs(){
	set_cur_to(obs_row, obs_col);
	HAL_UART_Transmit(&huart2, _obs_remove, sizeof(_obs_remove), HAL_MAX_DELAY);
}

void remove_plyr(){
	set_cur_to(plyr_row, plyr_col);
	HAL_UART_Transmit(&huart2, _player_remove, sizeof(_player_remove), HAL_MAX_DELAY);
}

void draw_score(){
	set_cur_to(2, 70);
	uint8_t buffer[10] = "";
	sprintf(buffer, "%d", score);
	HAL_UART_Transmit(&huart2, buffer, sizeof(buffer), HAL_MAX_DELAY);
}

void draw_score_at(int row, int col){
	set_cur_to(row, col);
	uint8_t buffer[10] = "";
	sprintf(buffer, "%d", score);
	HAL_UART_Transmit(&huart2, buffer, sizeof(buffer), HAL_MAX_DELAY);
}

void draw_player(){
	set_cur_to(plyr_row, plyr_col);
	if(is_plyr_green){
		HAL_UART_Transmit(&huart2, _player_green, sizeof(_player_green), HAL_MAX_DELAY);
	}else{
		HAL_UART_Transmit(&huart2, _player_yellow, sizeof(_player_yellow), HAL_MAX_DELAY);
	}
}

void draw_player_jump(){
	set_cur_to(plyr_row, plyr_col);
	if(is_plyr_green){
		HAL_UART_Transmit(&huart2, _player_jump_green, sizeof(_player_jump_green), HAL_MAX_DELAY);
	}else{
		HAL_UART_Transmit(&huart2, _player_jump_yellow, sizeof(_player_jump_yellow), HAL_MAX_DELAY);
	}
}

void draw_obs(){
	set_cur_to(obs_row, obs_col);
	HAL_UART_Transmit(&huart2, _obs, sizeof(_obs), HAL_MAX_DELAY);
}

void update_player(){
	remove_plyr();
	if(is_jumping){
		 if(going_up){
			 plyr_row--;
			 if(plyr_row <= max_jump_height) going_up = 0;
		 }
		 // going down
		 else{
			 plyr_row++;
			 if(plyr_row >= plyr_row_begin) {is_jumping = 0; going_up = 1;} // set going up for next jump
		 }
		 draw_player_jump();
	}else{
		draw_player();
	}
}

void update_obs(){
	// remove --> update position --> draw again
	remove_obs();
	obs_col--;
	if(obs_col < 0) obs_col = obs_col_begin;
	draw_obs();
}

void update_score(){
	score++;
	draw_score();
}

void update_frame(){
	update_obs();
	update_player();
	update_score();
}

void init_frame(){
	 plyr_row = plyr_row_begin; plyr_col = plyr_col_begin;
	 obs_row = obs_row_begin; obs_col = obs_col_begin;

	 max_jump_height = plyr_row_begin - jump_height;
	 going_up = 1;
	 is_jumping = 0;
	 obs_frame = 0; obs_rate = obs_rate_begin;
	 plyr_frame = 0; plyr_rate = plyr_rate_begin;

	 score = 0;

	 set_size();

	 clear_screen();
	 draw_player(); draw_obs(); draw_floor(); draw_score();
}

void jump(){
	if(!is_jumping) is_jumping = 1; // true
}

// returns 1 if player is colliding with obstacle, 0 otherwise
int collide(){
	if( (obs_row+2 >= plyr_row && obs_row+2 <= plyr_row+6) && (obs_col+3 >= plyr_col && obs_col+3 <= plyr_col+5) ){
		return 1;
	}
	if( (obs_row+2 >= plyr_row && obs_row+2 <= plyr_row+6) && (obs_col+1 <= plyr_col+5 && obs_col+1 >= plyr_col) ){
		return 1;
	}
	if( (obs_row+1 <= plyr_row+6 && obs_row+1 >= plyr_row) && (obs_col+3 >= plyr_col && obs_col+3 <= plyr_col+5) ){
		return 1;
	}
	if( (obs_row+1 <= plyr_row+6 && obs_row+1 >= plyr_row) && (obs_col+1 <= plyr_col+5 && obs_col+1 >= plyr_col) ){
		return 1;
	}

	return 0;
}

void disp_dead_scene(){
	clear_screen();

	// display score
	set_cur_to(10,33);
	sprintf(temp, "\033[1;34m SCORE : %d\033[0m", score);
	HAL_UART_Transmit(&huart2, temp, sizeof(temp), HAL_MAX_DELAY);

	// prompt use to press enter
	set_cur_to(11, 33);
	HAL_UART_Transmit(&huart2, _end_scene, sizeof(_end_scene), HAL_MAX_DELAY);

	keyboardInput = 0;
	while(keyboardInput != 13){
//		HAL_UART_Receive(&huart2, &keyboardInput, sizeof(keyboardInput), 10);
	}
}

void disp_start_scene(){
	clear_screen();

	set_cur_to(11, 30);
	HAL_UART_Transmit(&huart2, _start_scene, sizeof(_start_scene), HAL_MAX_DELAY);

	set_cur_to(23, 1);
	HAL_UART_Transmit(&huart2, _warning, sizeof(_warning), HAL_MAX_DELAY);

	// for blinking "JUMPJUMP"
	// keeps promting user to press enter
	keyboardInput = 0;
	while(keyboardInput != 13){
//		HAL_UART_Receive(&huart2, &keyboardInput, sizeof(keyboardInput), 10);
		temp_count++;
		if(temp_count == 4000){
			set_cur_to(11, 41);
			HAL_UART_Transmit(&huart2, _red_jumpjump, sizeof(_red_jumpjump), HAL_MAX_DELAY);
		}else if(temp_count == 5000){
			set_cur_to(11, 41);
			HAL_UART_Transmit(&huart2, _green_jumpjump, sizeof(_green_jumpjump), HAL_MAX_DELAY);
			temp_count = 0;
		}
		set_cur_to(13, 50);
	}
	keyboardInput = 0;
}

// swapping between green and yellow
void change_color(){
	// if player color is green
	if(is_plyr_green){
		is_plyr_green = 0;
	}else{
		is_plyr_green = 1;
	}
}

void disp_pause(){
	set_cur_to(4, 35);
	HAL_UART_Transmit(&huart2, _pause_message, sizeof(_pause_message), HAL_MAX_DELAY);
}

void remove_pause(){
	set_cur_to(4, 35);
	HAL_UART_Transmit(&huart2, _pause_message_remove, sizeof(_pause_message_remove), HAL_MAX_DELAY);
}

void pause(){
	keyboardInput = 0;
	disp_pause();
	while(keyboardInput != 80 && keyboardInput != 112){}
	remove_pause();
}

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
  /* USER CODE BEGIN 2 */
  __HAL_UART_ENABLE_IT(&huart2, UART_IT_RXNE);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  uint8_t buffer[10];

  while (1)
  {
	  switch(game_state){
	  case 0: // start scene
		  disp_start_scene();
		  init_frame();
		  game_state = 1;
		  break;
	  case 1: // playing
		  if(keyboardInput == 32){
			  jump(); // space bar
			  keyboardInput = 0;
		  }
		  if(keyboardInput == 80 || keyboardInput == 112){
			  pause();
			  keyboardInput = 0;
		  }
		  plyr_frame++;
		  obs_frame++;
		  if(plyr_frame == plyr_rate){ // update player
			  update_player();
			  update_score();
			  plyr_frame = 0;
		  }

		  if(obs_frame == obs_rate){ // update obstacle
			  update_obs();
			  // faster
			  if(obs_rate > obs_rate_limit && score != 0 && score%100 == 0){obs_rate-=8000; score++;}
			  obs_frame = 0;
			  if(collide()) game_state = 2;
		  }
		  break;
	  case 2: // dead
		  disp_dead_scene();
		  init_frame();
		  game_state = 1;
		  break;
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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
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
}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 921600;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
