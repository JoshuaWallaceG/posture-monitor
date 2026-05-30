/*
 * functions.h
 *
 *  Created on: Mar 30, 2026
 *      Author: joshua
 */


#ifndef SRC_FUNCTIONS_H_
#define SRC_FUNCTIONS_H_


//Includes
#include "main.h"
#include "font.h"


//Macros
// Display dimensions
#define MAX_X 128
#define NUMCOLS 128
#define MAX_Y 160
#define NUMROWS 160
// Common 16-bit RGB colors: bbbbb gggggg rrrrr
#define BLACK 0x0000 // 00000 000000 00000
#define WHITE 0xFFFF // 11111 111111 11111 (red + green + blue)
#define RED 0x001F // 00000 000000 11111
#define GREEN 0x07E0 // 00000 111111 00000
#define BLUE 0xF800 // 11111 000000 00000
#define CYAN 0xFFE0 // 11111 111111 00000 (green + blue)
#define MAGENTA 0xF81F // 11111 000000 11111 (red + blue)
#define YELLOW 0x07FF // 00000 111111 11111 (red + green)
#define ORANGE 0x041F // 00000 100000 11111 (red + 50% green)
#define PURPLE 0x8010 // 10000 000000 10000 (50% red + 50% blue)
#define GRAY 0x8410 // 10000 100000 10000 (50% red + 50% green + 50% blue)
#define LIGHTGRAY 0xC618 // 11000 110000 11000 (75% red + 75% green + 75% blue)
#define DARKGRAY 0x4208 // 01000 010000 01000 (25% red + 25% green + 25% blue)

// ST7735 command codes
#define SWRESET 0x01
#define SLPOUT 0x11
#define COLMOD 0x3A
#define MADCTL 0x36
#define DISPON 0x29
#define CASET 0x2A
#define RASET 0x2B
#define RAMWR 0x2C
#define COLOR_MODE_16BIT 0x05
#define MADCTL_DEFAULT 0xC8


//Prototypes
void ST7735_WriteData(uint8_t data[], uint16_t size);
void ST7735_WriteCommand(uint8_t cmd);
void ST7735_Init(void);
void ST7735_SetAddressWindow(uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd);
void ST7735_DrawScreen(uint16_t frame[MAX_Y][MAX_X]);
void ST7735_WriteCharToFrame(char character, uint16_t frame[MAX_Y][MAX_X], uint16_t quadrant);
void ST7735_WritePercentageToFrame(uint16_t percentage, uint16_t frame[MAX_Y][MAX_X], uint16_t quadrant);
void ST7735_WriteColorQuadrantFrame(uint16_t color, uint16_t frame[MAX_Y][MAX_X], uint16_t quadrant);
bool checkIfBadPosture(uint16_t sensor_percentage[], uint16_t sensor_weighted_percentage[]);
void convertPercentageToWeightedPercentage(uint16_t sensor_percentage[], uint16_t sensor_weighted_percentage[]);
void convertStepToRGB(uint16_t sensor_step_number[], uint16_t sensor_RGB[]);
void convertStepToPercentage(uint16_t sensor_step_number[], uint16_t sensor_percentage[]);

//Variables
extern uint16_t cursor_x; // start at left
extern uint16_t cursor_y;; // start at top
extern uint16_t frame[MAX_Y][MAX_X];
extern uint16_t sensor_percentage[3];
extern uint16_t sensor_weighted_percentage[2];
extern uint16_t sensor_RGB[3];
extern SPI_HandleTypeDef hspi1;



#endif /* SRC_FUNCTIONS_H_ */
