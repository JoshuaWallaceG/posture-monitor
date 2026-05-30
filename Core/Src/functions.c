#include "functions.h"


uint16_t cursor_x = 0; // start at left
uint16_t cursor_y = 0; // start at top
uint16_t frame[MAX_Y][MAX_X] = {0};
uint16_t sensor_percentage[3] = {0};
uint16_t sensor_weighted_percentage[2] = {0};
uint16_t sensor_RGB[3] = {0};


/**
 * @brief Transmits the provided bytes as data to the ST7735 over SPI1
 * @param data: the array of data / pointer to where the data starts
 * @param size: the number of bytes of data to transit
 * @retval None
 */
void ST7735_WriteData(uint8_t data[], uint16_t size) {
	// Change the DC (Data/Command) pin to "Data" mode
	HAL_GPIO_WritePin(DC_GPIO_Port, DC_Pin, 1);
	// Send the data
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, 0); // select the device
	HAL_SPI_Transmit(&hspi1, data, size, HAL_MAX_DELAY); // transmit bytes
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, 1); // deselect the device
}

/**
 * @brief Transmits the provided byte as a command to the ST7735 over SPI1
 * @param cmd: the 1-byte command to transmit
 * @retval None
 */
void ST7735_WriteCommand(uint8_t cmd) {
	// Change DC (Data/Command) pin to "Command" mode
	HAL_GPIO_WritePin(DC_GPIO_Port, DC_Pin, 0);
	// Send the command
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, 0); // select the device
	HAL_SPI_Transmit (&hspi1, &cmd, 1, HAL_MAX_DELAY); // transmit byte
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, 1); // deselect the device
}

/**
 * @brief Initializes the ST7735 over SPI1 with startup commands
 * @param None
 * @retval None
 */
void ST7735_Init(void) {
	// Perform a hardware reset (reset low pulse)
	HAL_GPIO_WritePin(RST_GPIO_Port, RST_Pin, GPIO_PIN_RESET); // RST pin low
	HAL_Delay(5); // wait 5ms
	HAL_GPIO_WritePin(RST_GPIO_Port, RST_Pin, GPIO_PIN_SET); // RST pin high
	HAL_Delay(5); // wait 5ms
	// Perform a software reset
	ST7735_WriteCommand(SWRESET); // send a SWRESET (Software reset) command
	HAL_Delay(150);
	// Wake the display (exit low-power sleep mode)
	ST7735_WriteCommand(SLPOUT); // Send a SLPOUT (Sleep out) command
	HAL_Delay(150);
	// Enter "Interface Pixel Format" mode and set to 16-bit color
	ST7735_WriteCommand(COLMOD); // Send a COLMOD (Color Mode) command
	uint8_t colorMode = 0x05; // color mode = 16-bit/pixel
	ST7735_WriteData(&colorMode, 1); // send color mode value as a data message
	// Enter "Memory Data Access Control" mode and set to row/column order
	ST7735_WriteCommand(MADCTL); // Send a MADCTL (Memory Access Data Control) command
	uint8_t accessMode = 0xC8; // access mode = row/column order
	ST7735_WriteData(&accessMode, 1); // send access mode value as a data message
	// Turn on display
	ST7735_WriteCommand(DISPON); // send a DISPON (Display On) command
	HAL_Delay(10);
}

/**
 * @brief Specifies the address window where the next pixel data will go
 * @param xStart: the starting x address
 * @param yStart: the starting y address
 * @param xEnd: the ending x address
 * @param yEnd: the ending y address
 * @retval None
 */
void ST7735_SetAddressWindow(uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd) {
	// Early exit if any dimensions are out of bounds
	if ((xStart > xEnd) || (yStart > yEnd) || (xEnd >= MAX_X) || (yEnd >= MAX_Y))
	return;
	// Adjust x,y internal offsets if needed by changing the zeros below:
	xStart += 2;
	xEnd += 2;
	yStart += 1;
	yEnd += 1;
	// Data array for configuration data
	uint8_t address[4];
	// Set column start/end
	ST7735_WriteCommand(0x2A); // Enter "Column address set" mode
	address[0] = xStart >> 8; // upper 8 bits of x starting address
	address[1] = xStart & 0xFF; // lower 8 bits of x starting address
	address[2] = xEnd >> 8; // upper 8 bits of x ending address
	address[3] = xEnd & 0xFF; // lower 8 bits of x ending address
	ST7735_WriteData(address, 4); // Transmit the 4 bytes of parameter data
	// Set row start/end
	ST7735_WriteCommand(0x2B); // Enter "Row address set" mode
	address[0] = yStart >> 8; // upper 8 bits of y starting address
	address[1] = yStart & 0xFF; // lower 8 bits of y starting address
	address[2] = yEnd >> 8; // upper 8 bits of y ending address
	address[3] = yEnd & 0xFF; // lower 8 bits of y ending address
	ST7735_WriteData(address, 4); // Transmit the 4 bytes of parameter data
	// Get ready to send pixel data
	ST7735_WriteCommand(0x2C); // Enter "RAM Write" mode (pixel writing mode)
}


/**
 * @brief Updated the character cursor
 * @param x: the desired x coordinate (0 to 127)
 * @param y: the desired y coordinate (0 to 159)
 * @retval 0: success, 1: failure
 */
void ST7735_UpdateCharCursor(uint16_t x, uint16_t y){
	// Update
	cursor_x = x;
	cursor_y = y;
	// Perform any necessary wrap-around
	if (cursor_x > (MAX_X-8)){ // cursor goes off the right
	cursor_x = 0; // wrap back to left
	cursor_y += 8; // move cursor down one row
	}
	if (cursor_y > (MAX_Y-9)){ // cursor goes off the bottom
	cursor_x = 0;
	cursor_y = 0; // wrap back to top
	}
}



/**
 * @brief Redraws the entire ST7735 display with provided frame
 * @param frame: the 2-dimensional array of 16-bit pixel colors
 * @retval None
 */
void ST7735_DrawScreen(uint16_t frame[MAX_Y][MAX_X]) {
	// Select entire display
	ST7735_SetAddressWindow(0, 0, NUMCOLS-1, NUMROWS-1);
	// Transmit entire frame to display
	ST7735_WriteData((uint8_t*) frame, NUMROWS*NUMCOLS*2);
}


/**
 * @brief Writes a 16x8, 2 digit percentage value the provided frame to the specified quadrant
 * @param percentage: a percentage INT value ranging from 0-99
 * @param frame: the 2-dimensional array of 16-bit pixel colors
 * @param quadrant: the quadrant the percentage should go (1 = top_left, 2 = top_right, 3 = bottom_middle)
 * @retval None
 */
void ST7735_WritePercentageToFrame(uint16_t percentage, uint16_t frame[MAX_Y][MAX_X], uint16_t quadrant) {

	//Make a char array of each number, [0] value is the 10s place, [1] is the 1's place
	char percentChar[2] = {
			percentage / 10 + '0',
			percentage % 10 + '0'
		};
	// Early exit if character is not supported
	if ((percentChar[0] < ' ') || (percentChar[0] > '~')) return;
	if ((percentChar[1] < ' ') || (percentChar[1] > '~')) return;

	// Realign ASCII character to start of font table
	int fontIndex1 = percentChar[0] - 0x20; // 0x20 is where printable characters begin in ASCII
	int fontIndex2 = percentChar[1] - 0x20; // 0x20 is where printable characters begin in ASCII
	// Get character data from font table

	const uint8_t* charData[2] = {fontTable[fontIndex1], fontTable[fontIndex2]};
	// Set address window for 8x8 character

	//Setting the cursor starter to be in the middle of the desired quadrant
	switch(quadrant){
	case 1:
		cursor_x = 64 - 32 - 8;
		cursor_y = 80 - 40 - 4;
		break;
	case 2:
		cursor_x = 64 + 32 - 8;
		cursor_y = 80 - 40 - 4;
		break;
	case 3:
		cursor_x = 64 - 8;
		cursor_y = 80 + 40 - 4;
		break;
	default:
		break;
	}

	//For loop each character (2 times total)
	for(int i = 0; i < 2; i++){
		ST7735_SetAddressWindow(cursor_x, cursor_y, cursor_x + 7, cursor_y + 7);
		// Process each row of the character, one at a time
		for (int y = 0; y < 8; y++) {
			// Get the data for the current row from the character array
			uint8_t rowData = charData[i][y];
			// Process each pixel in the row (right to left, lsb to msb) (Can use 7 if needed because there is an extra pixel)
				for (int x = 0; x < 8; x++) {
					// See if that pixel is black (1) or white (0)
					uint16_t color;
					if ((rowData & (0x80 >> x)) == 0){ // bitmask one bit at a time
						color = WHITE;
					}
					else {
						color = BLACK;
					}
				frame[cursor_y + y][cursor_x + x] = (color << 8) | (color >> 8); // Set frame value w/ swapped Endian color
			}
		}
		cursor_x = cursor_x + 8; //Shift to the right one character
	}
}

/**
 * @brief Writes a color value to the provided frame to every pixel in the specified quadrant
 * @param color: an RGB565 value to paint quadrant
 * @param frame: the 2-dimensional array of 16-bit pixel colors
 * @param quadrant: the quadrant to be painted should go (1 = top_left, 2 = top_right, 3 = bottom_middle)
 * @retval None
 */
void ST7735_WriteColorQuadrantFrame(uint16_t color, uint16_t frame[MAX_Y][MAX_X], uint16_t quadrant) {

	//Set up variables for where x and y need to start
	uint16_t starter_x = 0;
	uint16_t starter_y = 0;
	//Value for if we are writing to the bottom half, which is two quadrants
	uint16_t bottomMultiplier = 1;

	switch(quadrant){
	case 1:
		starter_x = 0;
		starter_y = 0;
		break;
	case 2:
		starter_x = 64;
		starter_y = 0;
		break;
	case 3:
		starter_x = 0;
		starter_y = 80;
		bottomMultiplier = 2;
		break;
	default:
		break;
	}

	for (int y = 0; y < 80; y++) { // iterate over each row
		for (int x = 0; x < 64 * bottomMultiplier; x++) { // within each row, iterate over each col
			frame[starter_y + y][starter_x + x] = (color << 8) | (color >> 8); // Set frame value w/ swapped Endian color
		}
	}
}


/**
 * @brief Checks if the current sensor values should be evaluated as bad posture
 * @param sensor_percentage[3]: an array of the sensors detected weight in percentage (0-99)
 * @param sensor_weighted_percentage[2]: an array of the sensors detected weight in percentage in relation to the other sensor (both add up to 100%)
 * @retval bool: TRUE if posture is evaluated as "bad," FALSE if posture is within okay threshold
 */
bool checkIfBadPosture(uint16_t sensor_percentage[], uint16_t sensor_weighted_percentage[]){

	//SITTING CHECK
	//Checks if someone is sitting by seeing if the total percentage is is less than 20
	if((sensor_percentage[0] + sensor_percentage[1]) < 20){
		return false; //If it is less than 20, then nobody is sitting, so no bad posture and return false
	}

	//LEANING LEFT-RIGHT CHECK
	//Checks to see if weight is within 25% of each other
	int16_t left_right_diff = sensor_weighted_percentage[0] - sensor_weighted_percentage[1];
	if(left_right_diff> 25 || left_right_diff < -25){
		return true; //If greater than 25%, then user is leaning too much, and bad posture is detected and return true
	}


	//SLOUCH/LEANING FORWARD CHECK
	//Checks to see if user is leaning back or not (aka slouched over or not)
	if(sensor_percentage[2] < 20){
		return true;  //If not detected, then user is slouching, bad postured is detected and return true
	}

	//If we have made it past the checks, then the users posture is within tolerance and return false;
	return false;
}

/**
 * @brief Converts an array of step values from sensors to 2 digit percentages (0-99)
 * @param sensor_step_number[3]: an array of the sensors detected weight in a value from 0-4095
 * @param sensor_percentage[3]: an array of the sensors detected weight in percentage (0-99)
 * @retval None
 */
void convertStepToPercentage(uint16_t sensor_step_number[], uint16_t sensor_percentage[]){

	//Takes each step value and first divides it by 4095. This gives us a decimal point
	//Multiply the decimal point by 99 to get a vale from 0 to 99
	for(int i = 0; i < 3; i++){
		sensor_percentage[i] = ((double)sensor_step_number[i] / 4095)*99;
	}
	return;
}

/**
 * @brief Converts an array of step values from sensors to 2 digit percentages (0-99)
 * @param sensor_step_number[3]: an array of the sensors detected weight in percentage (0-99)
 * @param sensor_RGB[2]: an array of the sensors step values in RGB565 form
 * @retval None
 */
void convertStepToRGB(uint16_t sensor_step_number[], uint16_t sensor_RGB[]){

	for(int i = 0; i < 3; i++){
		//Takes each step value and bitshifts it over by either 7 or 6, as we are only using the first few digits of the
		uint16_t r = sensor_step_number[i] >> 7;
		uint16_t g = sensor_step_number[i] >> 6;
		uint16_t b = sensor_step_number[i] >> 7;
		//Then combines them into a single value by bitshifting them and bitwise OR-ing them together
		sensor_RGB[i] = (r << 11) | (g <<  5) | (b);
	}
	return;
}

/**
 * @brief Converts an array of percentage values from sensors an array of "weighted" values, aka two values that always add up to 100%
 * @param sensor_percentage[3]: an array of the sensors detected weight in percentage (0-99)
 * @param sensor_weighted_percentage[2]: an array of the sensors detected weight in percentage in relation to the other sensor (both add up to 100%)
 * @retval None
 */
void convertPercentageToWeightedPercentage(uint16_t sensor_percentage[], uint16_t sensor_weighted_percentage[]){

	//Because we want the numbers to display not just the hard value, but a percentage in relation to the other value
	//Thus, we add them and then divide each percentage by the sum. This leaves us with both "weighted" against eachother
	sensor_weighted_percentage[0] = ((double)sensor_percentage[0] / (sensor_percentage[0] + sensor_percentage[1]))*99;
	sensor_weighted_percentage[1] = ((double)sensor_percentage[1] / (sensor_percentage[0] + sensor_percentage[1]))*99;
	//We again use *99 so that we never reach 100, because we only have 2 digits to display
	return;
}


