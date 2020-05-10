/*

    2013 Copyright (c) Seeed Technology Inc.  All right reserved.

    Author:Loovee, modified by Mastafa Awal for use with the LPC804 board
    2013-9-18

    The MIT License (MIT)

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.1  USA
*/

#include "LPC804.h"
#include <stdio.h>
#include <string.h>
#include "lcd_display.h"
#include "lcd_driver.h"
#include "ir_buttons.h"

uint8_t _lcd_displayfunction;
uint8_t _lcd_displaycontrol;
uint8_t _lcd_displaymode;

uint8_t _lcd_initialized;

uint8_t _lcd_numlines, _lcd_currline;

void i2c_send_byte(unsigned char dta) {
	i2c_lcd_begin_trans();						// transmit to device #4
    i2c_lcd_send_byte(dta);                            // sends five bytes
    i2c_lcd_end_trans();                     // stop transmitting
}

void i2c_send_byteS(unsigned char* dta, unsigned char len) {
	i2c_lcd_begin_trans();						// transmit to device #4
    for (int i = 0; i < len; i++) {
    	i2c_lcd_send_byte(dta[i]);
    }
    i2c_lcd_end_trans();                     // stop transmitting
}

void lcd_begin(uint8_t cols, uint8_t lines, uint8_t dotsize) {

	init_i2c_lcd();

    if (lines > 1) {
        _lcd_displayfunction |= LCD_2LINE;
    }
    _lcd_numlines = lines;
    _lcd_currline = 0;

    // for some 1 line displays you can select a 10 pixel high font
    if ((dotsize != 0) && (lines == 1)) {
        _lcd_displayfunction |= LCD_5x10DOTS;
    }

    // SEE PAGE 45/46 FOR INITIALIZATION SPECIFICATION!
    // according to datasheet, we need at least 40ms after power rises above 2.7V
    // before sending commands. Arduino can turn on way befer 4.5V so we'll wait 50
    delayMicroseconds(50000);


    // this is according to the hitachi HD44780 datasheet
    // page 45 figure 23

    // Send function set command sequence
    lcd_command(LCD_FUNCTIONSET | _lcd_displayfunction);
    delayMicroseconds(4500);  // wait more than 4.1ms

    // second try
    lcd_command(LCD_FUNCTIONSET | _lcd_displayfunction);
    delayMicroseconds(150);

    // third go
    lcd_command(LCD_FUNCTIONSET | _lcd_displayfunction);

    // finally, set # lines, font size, etc.
    lcd_command(LCD_FUNCTIONSET | _lcd_displayfunction);

    // turn the display on with no cursor or blinking default
    _lcd_displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
    lcd_display();

    // clear it off
    lcd_clear();

    // Initialize to default text direction (for romance languages)
    _lcd_displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
    // set the entry mode
    lcd_command(LCD_ENTRYMODESET | _lcd_displaymode);


void lcd_clear() {
    lcd_command(LCD_CLEARDISPLAY);        // clear display, set cursor position to zero
    delayMicroseconds(2000);          // this command takes a long time!
}

void lcd_home() {
    lcd_command(LCD_RETURNHOME);        // set cursor position to zero
    delayMicroseconds(2000);        // this command takes a long time!
}

void lcd_setCursor(uint8_t col, uint8_t row) {
    col = (row == 0 ? col | 0x80 : col | 0xc0);
    unsigned char dta[2] = {0x80, col};
    i2c_send_byteS(dta, 2);
}

void lcd_noDisplay() {
	_lcd_displaycontrol &= ~LCD_DISPLAYON;
    lcd_command(LCD_DISPLAYCONTROL | _lcd_displaycontrol);
}

void lcd_display() {
	_lcd_displaycontrol |= LCD_DISPLAYON;
    lcd_command(LCD_DISPLAYCONTROL | _lcd_displaycontrol);
}

// Turns the underline cursor on/off
void lcd_noCursor() {
	_lcd_displaycontrol &= ~LCD_CURSORON;
    lcd_command(LCD_DISPLAYCONTROL | _lcd_displaycontrol);
}

void lcd_cursor() {
	_lcd_displaycontrol |= LCD_CURSORON;
    lcd_command(LCD_DISPLAYCONTROL | _lcd_displaycontrol);
}

// Turn on and off the blinking cursor
void lcd_noBlink() {
    _lcd_displaycontrol &= ~LCD_BLINKON;
    lcd_command(LCD_DISPLAYCONTROL | _lcd_displaycontrol);
}
void lcd_blink() {
    _lcd_displaycontrol |= LCD_BLINKON;
    lcd_command(LCD_DISPLAYCONTROL | _lcd_displaycontrol);
}

// Scroll the display
void lcd_scrollDisplayLeft(void) {
    lcd_command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}
void lcd_scrollDisplayRight(void) {
    lcd_command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

// This is for text that flows Right to Left
void lcd_rightToLeft(void) {
    _lcd_displaymode &= ~LCD_ENTRYLEFT;
    lcd_command(LCD_ENTRYMODESET | _lcd_displaymode);
}

// This will 'right justify' text from the cursor
void lcd_autoscroll(void) {
    _lcd_displaymode |= LCD_ENTRYSHIFTINCREMENT;
    lcd_command(LCD_ENTRYMODESET | _lcd_displaymode);
}

// This will 'left justify' text from the cursor
void lcd_noAutoscroll(void) {
    _lcd_displaymode &= ~LCD_ENTRYSHIFTINCREMENT;
    lcd_command(LCD_ENTRYMODESET | _lcd_displaymode);
}

// send command
void lcd_command(uint8_t value) {
    unsigned char dta[2] = {0x80, value};
    i2c_send_byteS(dta, 2);
}

// send data
void lcd_write_char(uint8_t value) {
    unsigned char dta[2] = {0x40, value};
    i2c_send_byteS(dta, 2);
    return; // assume sucess
}

void lcd_write_str(char * str) {
	for (int i = 0; i < strlen(str); i++) {
		lcd_write_char(str[i]);
	}
}
void lcd_display_button(int but) {
	lcd_setCursor(0,1);
	lcd_write_str("Button: ");
	switch (but) {
		case BTN_UP:
			lcd_write_str("UP      ");
			break;
		case BTN_RIGHT:
			lcd_write_str("RIGHT   ");
			break;
		case BTN_DOWN:
			lcd_write_str("DOWN    ");
		case BTN_LEFT:
			lcd_write_str("LEFT    ");
			break;
		case BTN_OK:
			lcd_write_str("START/STOP");
			break;
		case BTN_1:
			lcd_write_str("1       ");
			break;
		case BTN_2:
			lcd_write_str("2       ");
			break;
		case BTN_3:
			lcd_write_str("3       ");
			break;
		case BTN_4:
			lcd_write_str("4       ");
			break;
		case BTN_5:
			lcd_write_str("5       ");
			break;
		case BTN_6:
			lcd_write_str("6       ");
			break;
		case BTN_7:
			lcd_write_str("7       ");
			break;
		case BTN_8:
			lcd_write_str("8       ");
			break;
		case BTN_9:
			lcd_write_str("9       ");
			break;
		case BTN_0:
			lcd_write_str("0       ");
			break;
		case BTN_AST:
			lcd_write_str("*       ");
			break;
		case BTN_PND:
			lcd_write_str("#       ");
			break;
	}

}

