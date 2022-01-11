#include "main.h"
#include "pca9538.h"
#include "kb.h"
#include "sdk_uart.h"

#define KBRD_RD_ADDR 0xE3
#define KBRD_WR_ADDR 0xE2
#define ROW1 0x1E
#define ROW2 0x3D
#define ROW3 0x7B
#define ROW4 0xF7

HAL_StatusTypeDef Set_Keyboard(void) {
 HAL_StatusTypeDef ret = HAL_OK;
 uint8_t buf;

 buf = 0x70;
 ret = PCA9538_Write_Register(KBRD_WR_ADDR, CONFIG, &buf);
 if (ret != HAL_OK) {
  UART_Transmit((uint8_t*) "Error write config\r\n");
  goto exit;
 }

 buf = 0;
 ret = PCA9538_Write_Register(KBRD_WR_ADDR, OUTPUT_PORT, &buf);
 if (ret != HAL_OK) {
  UART_Transmit((uint8_t*) "Error write output\r\n");
 }

 exit: return ret;
}

uint8_t Check_Row(uint8_t Nrow) {
 uint8_t Nkey = 0x00;
 HAL_StatusTypeDef ret = HAL_OK;
 uint8_t buf[4] = { 0, 0, 0, 0 };
 uint8_t kbd_in;

 ret = Set_Keyboard();
 if (ret != HAL_OK) {
  UART_Transmit((uint8_t*) "Error write config\r\n");
 }

 buf[0] = Nrow;

 ret = PCA9538_Write_Register(KBRD_WR_ADDR, OUTPUT_PORT, buf);
 if (ret != HAL_OK) {
  UART_Transmit((uint8_t*) "Error write output\r\n");
 }

 buf[0] = 0;
 ret = PCA9538_Read_Inputs(KBRD_RD_ADDR, buf);
 if (ret != HAL_OK) {
  UART_Transmit((uint8_t*) "Read error\r\n");
 }
 kbd_in = buf[0] & 0x70;
 Nkey = 0x0F ^ (kbd_in >> 4);
 return Nkey;
}
