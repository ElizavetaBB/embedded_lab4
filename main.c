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
#include "i2c.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "stdint.h"
#include "string.h"
#include <stdio.h>
#include <stdbool.h>
#include "ctype.h"
#include "sdk_uart.h"
#include "kb.h"
#include "oled.h"

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

uint8_t live_points = 1;
bool is_optimal = false;

uint8_t print_buf[256];
uint8_t cursor[2] = {0, 0};
OLED_COLOR blink_color = White;
uint8_t draw_color = 0;
uint8_t step = 1;
bool previous_color_set = false;
OLED_COLOR previous_color = Black;
bool start_game = false;

uint8_t OLED_Buffer_current[OLED_WIDTH][OLED_HEIGHT];

static uint8_t OLED_Buffer_next[OLED_WIDTH][OLED_HEIGHT] = {0x00};
static uint8_t alive_neighbours[OLED_WIDTH][OLED_HEIGHT] = {0};


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void count_alive_neighbours(){
	for (int h = 0; h < OLED_HEIGHT; h++){
			for (int w = 0; w < OLED_WIDTH; w++){
				int counter = 0;
				for (int i = w - 1; i <= w + 1; i++){
					for (int j = h - 1; j <= h + 1; j++){
						if (i == w && j == h) continue;
						uint8_t x1 = (i < 0) ? 127 : ((i > 127) ? 0 : i);
						uint8_t y1 = (j < 0) ? 63 : ((j > 63) ? 0 : j);
						if (oled_getPixel(x1, y1) == White) counter++;
					}
			}
			alive_neighbours[w][h] = counter;
		}
	}
}

void next_generation(){
	count_alive_neighbours();

	for (int i = 0; i < OLED_WIDTH; i++){
		for (int j = 0; j < OLED_HEIGHT; j++){

			uint8_t counter = alive_neighbours[i][j];

			if (oled_getPixel(i, j) == Black){
				if (counter == 3) OLED_Buffer_next[i][j] = White;
				else OLED_Buffer_next[i][j] = Black;
			} else {
				if (counter < 2 || counter > 3) OLED_Buffer_next[i][j] = Black;
				else OLED_Buffer_next[i][j] = White;
			}

		}
	}
}

bool cmp_worlds(){
	for (int i = 0; i < OLED_WIDTH; i++){
		for (int j = 0; j < OLED_HEIGHT; j++){

			if (OLED_Buffer_next[i][j] != OLED_Buffer_current[i][j]){
				return false;
			}
		}
	}
	return true;
}

int get_live_count(){
	int counter = 0;
	for (int i = 0; i < OLED_WIDTH; i++){
		for (int j = 0; j < OLED_HEIGHT; j++){
			if (OLED_Buffer_next[i][j] == White) counter++;
		}
	}
	return counter;
}

void play_generation(){
	if (live_points != 0 && !is_optimal){
		next_generation();
		is_optimal = cmp_worlds();
		live_points = get_live_count();
		if (is_optimal){
			snprintf(print_buf, sizeof(print_buf), "The current state is stable\r\n");
			UART_Transmit((uint8_t*) print_buf);
		}

		if (live_points == 0){
			snprintf(print_buf, sizeof(print_buf), "All cells are dead\r\n");
			UART_Transmit((uint8_t*) print_buf);
		}

		oled_DrawMap(OLED_Buffer_next);
		oled_UpdateScreen();
	}
}

void update_cursor(uint8_t x, uint8_t y){
	cursor[0] = x;
	cursor[1] = y;
	previous_color_set = false;
	oled_SetCursor(x, y);
}

void set_previous_color(){
	previous_color = oled_getPixel(cursor[0], cursor[1]);
	previous_color_set = true;
}

void draw_cursorPath(uint8_t direction){
	for (int i = 0; i < step; i++){
		if (draw_color != 2){
			oled_DrawPixel(cursor[0], cursor[1], (draw_color == 0) ? White: Black);
		} else {
			if (previous_color_set && i == 0){
				oled_DrawPixel(cursor[0], cursor[1], previous_color);
			}
		}
		if (direction == 0){ // вверх
			if (cursor[1] != 0) cursor[1]--;
			else break;
		} else if (direction == 1){ // вправо
			if (cursor[0] != 127) cursor[0]++;
			else break;
		} else if (direction == 2){ // влево
			if (cursor[0] != 0) cursor[0]--;
			else break;
		} else if (direction == 3){ // вниз
			if (cursor[1] != 63) cursor[1]++;
			else break;
		}
	}
	update_cursor(cursor[0], cursor[1]);
	oled_UpdateScreen();
}

void handle_command(uint8_t key){
	switch(key){
		case 1:
			if (!start_game){
				if (step < 127) step++;
				snprintf(print_buf, sizeof(print_buf), "Current step = %d\r\n", step);
				UART_Transmit((uint8_t*) print_buf);
			}
			break;
		case 2:
			if (!start_game){
				draw_cursorPath(0);
			}
			break;
		case 3:
			if (!start_game){
				if (step > 1) step--;
				snprintf(print_buf, sizeof(print_buf), "Current step = %d\r\n", step);
				UART_Transmit((uint8_t*) print_buf);
			}
			break;
		case 4:
			if (!start_game){
				draw_cursorPath(2);
			}
			break;
		case 5:
			break;
		case 6:
			if (!start_game){
				draw_cursorPath(1);
			}
			break;
		case 7:
			break;
		case 8:
			if (!start_game){
				draw_cursorPath(3);
			}
			break;
		case 9:
			break;
		case 10:
			if (!start_game){
				draw_color = (draw_color + 1) % 3;
				snprintf(print_buf, sizeof(print_buf), (draw_color == 0) ? "Draw color is WHITE\r\n" : ((draw_color == 1) ? "Draw color is BLACK\r\n" : "Draw color isn't set\r\n"));
				UART_Transmit((uint8_t*) print_buf);
			}
			break;
		case 11:
			start_game = !start_game;
			if (start_game){
				snprintf(print_buf, sizeof(print_buf), "Enter game mode\r\n");
				UART_Transmit((uint8_t*) print_buf);
				live_points = 1;
				is_optimal = false;
			} else {
				snprintf(print_buf, sizeof(print_buf), "Leave game mode\r\n");
				UART_Transmit((uint8_t*) print_buf);
				update_cursor(0, 0);
			}
			break;
		case 12:
			if (start_game){
				snprintf(print_buf, sizeof(print_buf), "Leave game mode\r\n");
				UART_Transmit((uint8_t*) print_buf);
				start_game = false;
			}
			oled_Fill(Black);
			update_cursor(0, 0);
			oled_UpdateScreen();
			break;
	}
}

void blinkCursor(){
	if (!previous_color_set) {
		set_previous_color();
	}
	oled_DrawPixel(cursor[0], cursor[1], blink_color);
	oled_UpdateScreen();
	blink_color = (blink_color == White)? Black: White;
}

void KB_Test() {
	static const uint8_t rows[] = { 0x1E, 0x3D, 0x7B, 0xF7 };
	static uint8_t old_keys[] = { 0, 0, 0, 0 };
	static int current_row = 0;
	static const uint8_t key_chars[] = {1,2,3,4,5,6,7,8,9,10,11,12};

	uint8_t current_key = Check_Row(rows[current_row]);
	uint8_t *old_key = &old_keys[current_row];

	for (int i = 0; i < 3; ++i) {
		int mask = 1 << i;
		if ((current_key & mask) && !(*old_key & mask)) {
			handle_command(key_chars[i + 3 * current_row]);
		}
	}

	*old_key = current_key;
	current_row = (current_row + 1) % 4;
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
  MX_I2C1_Init();
  MX_USART6_UART_Init();
  /* USER CODE BEGIN 2 */
  oled_Init();
  uint8_t counter = 0;
  uint8_t game_counter = 0;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  KB_Test();
	  HAL_Delay(10);

	  if (!start_game){
		  counter++;

		  if (counter == 50){
			  blinkCursor();
			  counter = 0;
		  }
	  } else {
		  game_counter++;
		  counter = 0;
		  if (game_counter == 50){
			  play_generation();
			  game_counter = 0;
		  }
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
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 15;
  RCC_OscInitStruct.PLL.PLLN = 216;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
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

