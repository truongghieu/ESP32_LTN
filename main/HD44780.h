#pragma once
void LCD_init(int addr, int dataPin, int clockPin, int cols, int rows);
void LCD_setCursor(int col, int row);
void LCD_home(void);
void LCD_clearScreen(void);
void LCD_writeChar(char c);
void LCD_writeStr(char* str); 