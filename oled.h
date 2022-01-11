#include "stm32f4xx_hal.h"

#ifndef oled
#define oled

#define OLED_I2C_PORT		hi2c1
#define OLED_I2C_ADDR        0x78
#define OLED_WIDTH           128
#define OLED_HEIGHT          64

typedef enum {
	Black = 0x00,
	White = 0x01
} OLED_COLOR;

typedef struct {
	uint16_t CurrentX;
	uint16_t CurrentY;
	uint8_t Inverted;
	uint8_t Initialized;
} OLED_t;

extern I2C_HandleTypeDef OLED_I2C_PORT;

uint8_t oled_Init(void);
void oled_Fill(OLED_COLOR color);
void oled_UpdateScreen(void);
void oled_DrawPixel(uint8_t x, uint8_t y, OLED_COLOR color);
void oled_DrawMap(uint8_t map[OLED_WIDTH][OLED_HEIGHT]);
void oled_SetCursor(uint8_t x, uint8_t y);
OLED_COLOR oled_getPixel(uint8_t x, uint8_t y);

#endif
