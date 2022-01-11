#include "oled.h"
#include "i2c.h"
#include "sdk_uart.h"
#include <stdio.h>

static uint8_t OLED_Buffer[1024];

extern uint8_t OLED_Buffer_current[OLED_WIDTH][OLED_HEIGHT];

uint8_t print_buf[256];
static OLED_t OLED;


static void oled_WriteCommand(uint8_t command) {
	HAL_I2C_Mem_Write(&hi2c1,OLED_I2C_ADDR,0x00,1,&command,1,10);
}


uint8_t oled_Init(void) {
	HAL_Delay(100);

	oled_WriteCommand(0xAE); // выключить дисплей

	oled_WriteCommand(0x20);
	oled_WriteCommand(0x00); // горизонтальный режим адресации

	oled_WriteCommand(0xB0);

	oled_WriteCommand(0xC8); // инферсия ихображения сигналов COM

	oled_WriteCommand(0x00);

	oled_WriteCommand(0x10);

	oled_WriteCommand(0x40); // Начальная строка: 0

	oled_WriteCommand(0x81);
	oled_WriteCommand(0xFF); // Контраст: 256

	oled_WriteCommand(0xA1); // Инверсия отображения сигналов SEG

	oled_WriteCommand(0xA6); // Нормальный (неинвертированный) цвет

	oled_WriteCommand(0xA8);
	oled_WriteCommand(0x3F); // Настройка мультиплексора

	oled_WriteCommand(0xA4); // Отображать содержимое видеопамяти

	oled_WriteCommand(0xD3);
	oled_WriteCommand(0x00); // Вертикальный сдвиг: 0

	oled_WriteCommand(0xD5);
	oled_WriteCommand(0xF0); // Настройка синхросигналов

	oled_WriteCommand(0xD9);
	oled_WriteCommand(0x22); // Настройка перезарядки

	oled_WriteCommand(0xDA);
	oled_WriteCommand(0x12); // Настройка конфигурации

	oled_WriteCommand(0xDB);
	oled_WriteCommand(0x20); // настройка напряжения

	oled_WriteCommand(0x8D);
	oled_WriteCommand(0x14); // Включение встроенного источника напряжения

	oled_WriteCommand(0xAF); // Включить дисплей

	oled_Fill(Black);

	oled_UpdateScreen();

	OLED.CurrentX = 0;
	OLED.CurrentY = 0;

	OLED.Initialized = 1;

	return 1;
}

void oled_Fill(OLED_COLOR color) {
	uint32_t i;

	for(i = 0; i < sizeof(OLED_Buffer); i++) {
		OLED_Buffer[i] = (color == Black) ? 0x00 : 0xFF;
	}
	for (i = 0; i < OLED_WIDTH; i++){
		for (int j = 0; j < OLED_HEIGHT; j++){
			OLED_Buffer_current[i][j] = color;
		}
	}
}


void oled_UpdateScreen(void) {

	for (uint8_t i = 0; i < 8; i++) {
		HAL_I2C_Mem_Write(&hi2c1,OLED_I2C_ADDR,0x40,1,&OLED_Buffer[OLED_WIDTH * i],OLED_WIDTH,25);
	}
}

void oled_DrawPixel(uint8_t x, uint8_t y, OLED_COLOR color) {
	if (x >= OLED_WIDTH || y >= OLED_HEIGHT) {
		return;
	}

	if (OLED.Inverted) {
		color = (OLED_COLOR)!color;
	}

	if (color == White) {
		OLED_Buffer[x + (y / 8) * OLED_WIDTH] |= 1 << (y % 8);
		OLED_Buffer_current[x][y] = White;
	} else {
		OLED_Buffer[x + (y / 8) * OLED_WIDTH] &= ~(1 << (y % 8));
		OLED_Buffer_current[x][y] = Black;
	}
}

OLED_COLOR oled_getPixel(uint8_t x, uint8_t y){
	return OLED_Buffer_current[x][y];
}

void oled_DrawMap(uint8_t map[OLED_WIDTH][OLED_HEIGHT]){
	for (uint8_t i = 0; i < OLED_HEIGHT; i++){
		for (uint8_t j = 0; j < OLED_WIDTH; j++){
			oled_DrawPixel(i, j, map[i][j]);
		}
	}
}

void oled_SetCursor(uint8_t x, uint8_t y) {
	OLED.CurrentX = x;
	OLED.CurrentY = y;
}

