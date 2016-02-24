/*
Water pump controller V 0.1
Built upon Adafruit TFT library examples
Daniel Lameka 2/12/2016

speaker pin can only be changed in lib file

pin 39 dedicated to +5 of RTC module
pin 41 dedicated to GND of RTC module

pin - 24 button with 10K pull up 
pin 25 - sensor with 10K pull up

pin 48 - SSR relay
       
*/




#include "SPFD5408_Adafruit_GFX.h"    // Core graphics library
#include "SPFD5408_Adafruit_TFTLCD.h" // Hardware-specific library
#include "SPFD5408_TouchScreen.h"
#include "Wire.h"   // for time module lib
#include "RTClib.h" // time module lib
#include "PCM.h"
#include "SPI.h"
#include "SD.h"
#include "Button.h"

#define DS1307_ADDRESS 0x68
#define BAUD 9600
#define ON LOW
#define OFF HIGH
#define BUTTONPIN 24
#define SENSORPIN 25
#define SSRPIN 48
#define RTCPOSITIVE 39
#define RTCNEGATIVE 41

byte zero = 0x00; // workaround for issue #52 time module related
RTC_DS1307 RTC;   // time module related

#if defined(__SAM3X8E__)
    #undef __FlashStringHelper::F(string_literal)
    #define F(string_literal) string_literal
#endif

#define YP A3  // must be an analog pin, use "An" notation!
#define XM A2  // must be an analog pin, use "An" notation!
#define YM 9   // can be a digital pin
#define XP 8   // can be a digital pin

// Calibrate values
#define TS_MINX 115
#define TS_MINY 115
#define TS_MAXX 840
#define TS_MAXY 840

// For better pressure precision, we need to know the resistance
// between X+ and X- Use any multimeter to read it
// For the one we're using, its 300 ohms across the X plate
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

#define LCD_CS A3
#define LCD_CD A2
#define LCD_WR A1
#define LCD_RD A0
// optional
#define LCD_RESET A4

// Assign human-readable names to some common 16-bit color values:
#define	BLACK   0x0000
#define	BLUE    0x001F
#define	DARKPINK 0xD20B // my value
#define	GREEN   0x2631 //my value
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xECE0 //my value
#define WHITE   0xFFFF
#define LIGHTGREY 0x2145 // myvalue
#define DARKGREY 0x1083 //my value

#define MINPRESSURE 5
#define MAXPRESSURE 1000

#define STATUSBAR 40
#define MENUW 105
#define MENUH 125
#define BORDER 10
#define TBUTTON 75

Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

Button button = Button(BUTTONPIN,PULLUP);
Button sensor = Button(SENSORPIN, PULLUP);


const unsigned char R2D2[] PROGMEM = {
127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 125, 125, 126, 126, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 126, 126, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 126, 126, 126, 126, 126, 125, 125, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 125, 125, 125, 125, 125, 125, 125, 125, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 127, 127, 127, 127, 126, 126, 126, 127, 127, 127, 127, 127, 127, 127, 127, 127, 126, 126, 126, 126, 126, 126, 126, 127, 127, 127, 126, 126, 127, 126, 126, 126, 126, 126, 126, 126, 127, 127, 127, 127, 127, 127, 127, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 129, 129, 129, 129, 129, 129, 129, 129, 129, 129, 129, 129, 129, 130, 130, 130, 130, 130, 130, 130, 130, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 130, 130, 130, 130, 129, 129, 129, 128, 128, 127, 127, 126, 126, 126, 125, 124, 123, 123, 122, 121, 120, 119, 119, 118, 118, 117, 117, 117, 116, 116, 115, 115, 115, 115, 114, 114, 114, 114, 113, 113, 113, 113, 114, 113, 112, 111, 112, 113, 112, 111, 111, 113, 112, 111, 117, 132, 146, 151, 148, 144, 140, 134, 130, 134, 147, 161, 168, 171, 174, 178, 180, 173, 165, 165, 175, 187, 191, 191, 193, 197, 200, 195, 187, 182, 177, 171, 163, 157, 152, 149, 147, 148, 154, 158, 158, 154, 148, 143, 138, 135, 133, 131, 127, 121, 115, 109, 103, 98, 93, 88, 83, 77, 69, 60, 51, 45, 45, 47, 48, 46, 44, 43, 42, 40, 35, 32, 34, 39, 44, 47, 49, 52, 55, 60, 69, 80, 92, 101, 106, 112, 119, 126, 132, 138, 148, 162, 179, 191, 199, 204, 208, 213, 220, 228, 235, 241, 245, 247, 247, 247, 248, 249, 251, 254, 255, 254, 247, 234, 221, 210, 203, 198, 193, 186, 178, 169, 158, 145, 132, 120, 110, 100, 91, 80, 68, 56, 45, 36, 31, 29, 29, 28, 24, 17, 11, 6, 3, 2, 4, 10, 19, 29, 35, 40, 45, 52, 62, 74, 85, 95, 104, 112, 121, 131, 142, 154, 168, 182, 195, 206, 212, 216, 218, 222, 227, 234, 238, 242, 244, 245, 244, 242, 239, 235, 230, 224, 217, 209, 200, 188, 177, 166, 158, 151, 143, 134, 124, 112, 100, 86, 73, 61, 53, 49, 45, 41, 36, 30, 26, 23, 22, 25, 28, 32, 34, 36, 38, 43, 51, 62, 75, 88, 101, 113, 122, 129, 137, 147, 159, 171, 182, 192, 200, 208, 214, 220, 225, 230, 234, 236, 236, 234, 229, 223, 217, 212, 207, 202, 195, 186, 176, 164, 152, 138, 125, 113, 103, 93, 83, 73, 64, 56, 49, 44, 40, 37, 34, 32, 30, 29, 30, 33, 39, 47, 56, 67, 76, 85, 94, 102, 111, 121, 131, 143, 155, 166, 176, 184, 191, 199, 206, 211, 215, 218, 219, 218, 215, 211, 208, 205, 202, 197, 191, 182, 172, 160, 147, 136, 126, 117, 108, 99, 90, 81, 73, 65, 59, 54, 50, 47, 45, 43, 42, 42, 45, 50, 57, 65, 73, 81, 89, 97, 105, 114, 124, 134, 145, 156, 166, 175, 182, 189, 195, 201, 205, 208, 210, 210, 210, 208, 205, 201, 197, 192, 187, 180, 171, 161, 150, 139, 128, 118, 109, 100, 92, 84, 76, 69, 62, 56, 51, 47, 45, 44, 44, 46, 48, 52, 57, 65, 72, 80, 89, 97, 105, 113, 122, 132, 142, 152, 162, 172, 180, 186, 191, 195, 199, 202, 203, 204, 204, 202, 199, 194, 190, 184, 178, 171, 162, 153, 143, 133, 122, 113, 104, 95, 88, 81, 74, 68, 62, 58, 54, 52, 50, 51, 52, 55, 58, 63, 69, 75, 83, 91, 99, 108, 116, 124, 133, 141, 150, 158, 166, 174, 180, 186, 189, 192, 194, 195, 196, 195, 193, 189, 185, 179, 174, 167, 161, 154, 146, 138, 129, 120, 111, 103, 95, 88, 82, 77, 72, 69, 66, 64, 64, 64, 65, 67, 70, 73, 78, 84, 91, 98, 105, 113, 121, 129, 136, 143, 150, 156, 162, 167, 171, 175, 178, 181, 182, 182, 180, 178, 175, 171, 166, 161, 155, 148, 141, 134, 127, 120, 114, 106, 99, 92, 86, 81, 76, 72, 69, 67, 66, 66, 67, 69, 72, 75, 80, 85, 91, 97, 104, 111, 119, 127, 135, 143, 150, 157, 163, 168, 172, 176, 179, 181, 183, 184, 183, 182, 179, 175, 171, 165, 160, 153, 147, 140, 133, 125, 118, 111, 104, 98, 92, 86, 81, 77, 73, 70, 68, 68, 69, 70, 73, 76, 80, 84, 89, 94, 100, 106, 113, 121, 128, 135, 141, 147, 153, 158, 163, 167, 170, 172, 174, 174, 174, 173, 172, 169, 166, 162, 157, 151, 145, 139, 132, 126, 120, 114, 108, 103, 97, 92, 87, 83, 80, 78, 77, 76, 77, 78, 80, 82, 85, 89, 94, 99, 104, 110, 116, 121, 127, 133, 139, 145, 150, 155, 158, 161, 163, 165, 166, 166, 166, 165, 163, 161, 158, 154, 150, 145, 140, 135, 130, 124, 119, 113, 108, 104, 99, 96, 93, 91, 89, 88, 87, 87, 87, 89, 91, 94, 98, 102, 107, 111, 116, 121, 126, 131, 136, 141, 146, 150, 153, 156, 159, 160, 162, 162, 162, 161, 159, 156, 153, 150, 147, 143, 138, 134, 129, 123, 118, 113, 108, 103, 99, 95, 92, 90, 88, 87, 86, 86, 86, 88, 90, 92, 96, 99, 103, 108, 112, 117, 123, 128, 133, 138, 143, 147, 150, 153, 156, 158, 160, 161, 161, 161, 160, 159, 156, 154, 151, 147, 144, 139, 135, 130, 125, 120, 116, 112, 108, 105, 102, 99, 96, 95, 94, 93, 93, 94, 95, 97, 100, 102, 106, 109, 114, 118, 122, 126, 131, 135, 138, 142, 146, 149, 152, 154, 157, 158, 158, 158, 157, 156, 154, 152, 150, 146, 143, 139, 135, 130, 126, 122, 119, 115, 111, 107, 104, 101, 99, 97, 96, 96, 96, 97, 98, 100, 102, 105, 107, 110, 114, 118, 122, 125, 129, 133, 137, 140, 143, 146, 148, 150, 152, 152, 153, 153, 153, 152, 151, 149, 147, 144, 141, 138, 135, 132, 129, 126, 122, 119, 116, 114, 112, 110, 108, 107, 107, 107, 107, 108, 109, 110, 112, 114, 117, 119, 123, 126, 129, 132, 135, 138, 141, 143, 146, 148, 149, 150, 150, 150, 150, 150, 149, 147, 146, 143, 141, 138, 134, 131, 128, 124, 121, 118, 116, 113, 110, 108, 106, 105, 104, 103, 103, 104, 104, 106, 107, 109, 112, 115, 118, 121, 125, 128, 131, 135, 138, 141, 144, 146, 149, 151, 152, 153, 154, 155, 154, 154, 152, 151, 149, 147, 145, 142, 139, 136, 133, 130, 127, 125, 122, 119, 116, 114, 112, 110, 109, 109, 108, 108, 109, 110, 111, 112, 114, 117, 119, 122, 124, 127, 130, 132, 135, 138, 140, 142, 144, 146, 147, 148, 148, 148, 148, 147, 146, 145, 144, 142, 140, 138, 136, 133, 131, 128, 125, 123, 121, 119, 117, 115, 114, 113, 113, 113, 113, 113, 114, 115, 116, 118, 120, 122, 124, 126, 129, 131, 134, 136, 138, 140, 142, 143, 144, 145, 145, 145, 145, 145, 144, 144, 142, 141, 139, 137, 135, 133, 131, 128, 126, 124, 122, 120, 119, 117, 116, 115, 115, 114, 115, 115, 115, 116, 117, 119, 121, 123, 125, 128, 130, 132, 134, 137, 139, 141, 142, 144, 145, 146, 147, 148, 148, 147, 147, 146, 145, 144, 142, 140, 138, 136, 134, 132, 130, 128, 126, 124, 122, 120, 119, 117, 116, 116, 115, 115, 116, 116, 117, 118, 119, 121, 122, 123, 125, 127, 129, 130, 132, 133, 135, 136, 137, 138, 139, 139, 139, 139, 139, 138, 138, 137, 137, 135, 134, 133, 132, 130, 129, 128, 127, 125, 124, 124, 123, 123, 122, 122, 122, 122, 123, 124, 125, 126, 127, 128, 129, 131, 132, 134, 135, 136, 137, 138, 138, 139, 140, 140, 140, 140, 140, 139, 138, 137, 135, 134, 132, 130, 129, 127, 125, 124, 122, 121, 120, 119, 118, 117, 116, 115, 115, 116, 116, 117, 118, 120, 121, 123, 125, 126, 128, 130, 131, 133, 135, 137, 139, 140, 141, 142, 143, 144, 144, 144, 144, 143, 142, 141, 140, 139, 138, 136, 135, 133, 131, 130, 128, 126, 124, 122, 121, 120, 119, 118, 118, 117, 117, 117, 118, 118, 119, 119, 120, 121, 122, 123, 125, 126, 128, 129, 131, 132, 133, 134, 134, 135, 135, 136, 136, 136, 135, 135, 134, 133, 133, 131, 130, 129, 128, 126, 125, 124, 123, 122, 122, 121, 121, 121, 121, 121, 121, 122, 122, 123, 123, 124, 125, 126, 128, 129, 130, 131, 133, 133, 134, 134, 135, 135, 135, 135, 135, 135, 135, 135, 134, 133, 132, 131, 130, 129, 128, 127, 126, 125, 124, 123, 122, 121, 121, 121, 121, 121, 121, 121, 121, 121, 122, 123, 124, 125, 126, 128, 129, 130, 131, 132, 133, 133, 134, 135, 135, 136, 136, 136, 136, 135, 135, 134, 133, 132, 131, 130, 128, 127, 126, 124, 123, 122, 121, 120, 119, 118, 117, 117, 116, 116, 116, 116, 116, 117, 117, 118, 119, 120, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 134, 134, 134, 134, 134, 133, 133, 133, 132, 132, 131, 130, 130, 129, 128, 127, 127, 127, 126, 126, 125, 125, 125, 125, 125, 125, 126, 126, 127, 127, 128, 128, 128, 128, 128, 129, 129, 129, 129, 129, 129, 128, 128, 127, 126, 126, 125, 124, 123, 122, 121, 120, 119, 119, 118, 118, 117, 117, 117, 117, 117, 117, 117, 118, 119, 120, 121, 122, 123, 125, 126, 128, 129, 130, 131, 132, 133, 134, 135, 136, 136, 137, 137, 137, 136, 136, 135, 135, 134, 133, 132, 131, 129, 128, 127, 126, 125, 123, 122, 121, 120, 119, 118, 118, 117, 117, 117, 118, 118, 119, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 133, 133, 133, 133, 133, 132, 132, 131, 131, 130, 129, 128, 127, 126, 125, 124, 123, 122, 121, 120, 119, 118, 118, 118, 118, 118, 118, 118, 118, 119, 119, 120, 121, 122, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 133, 134, 134, 134, 134, 134, 133, 133, 132, 132, 131, 130, 128, 128, 127, 126, 125, 124, 123, 122, 121, 120, 120, 119, 119, 119, 119, 119, 119, 119, 120, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 129, 130, 131, 132, 132, 133, 133, 133, 133, 132, 132, 132, 131, 130, 130, 129, 128, 127, 126, 125, 125, 124, 123, 122, 121, 121, 120, 120, 120, 120, 120, 120, 120, 120, 120, 121, 121, 122, 123, 124, 124, 125, 126, 127, 128, 128, 129, 130, 130, 131, 131, 132, 132, 132, 133, 133, 133, 133, 133, 133, 132, 132, 132, 131, 131, 130, 130, 130, 129, 129, 128, 128, 127, 127, 126, 125, 125, 124, 124, 123, 123, 123, 123, 123, 122, 122, 122, 122, 122, 122, 122, 122, 122, 122, 122, 122, 122, 123, 123, 124, 124, 125, 126, 126, 127, 127, 128, 129, 129, 130, 131, 131, 132, 132, 133, 133, 133, 134, 134, 134, 134, 134, 134, 133, 133, 133, 132, 132, 131, 130, 129, 128, 127, 126, 125, 124, 123, 123, 122, 121, 120, 120, 119, 119, 119, 119, 119, 119, 120, 120, 121, 121, 122, 123, 124, 125, 126, 127, 128, 129, 129, 130, 131, 132, 133, 134, 134, 134, 135, 135, 135, 135, 135, 135, 134, 134, 133, 133, 132, 131, 131, 130, 130, 129, 128, 128, 127, 127, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 127, 127, 127, 128, 128, 128, 129, 129, 129, 129, 129, 130, 130, 129, 129, 129, 128, 128, 128, 128, 127, 127, 126, 126, 125, 124, 124, 123, 123, 123, 122, 122, 121, 121, 121, 122, 122, 123, 123, 124, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 135, 136, 137, 138, 138, 138, 139, 139, 139, 139, 138, 138, 138, 137, 136, 135, 135, 134, 133, 132, 131, 130, 129, 128, 126, 125, 125, 124, 123, 122, 122, 121, 121, 120, 120, 120, 120, 120, 120, 121, 122, 122, 123, 123, 124, 124, 125, 126, 127, 128, 129, 130, 131, 132, 132, 133, 134, 134, 135, 135, 135, 135, 135, 135, 135, 135, 135, 134, 134, 133, 133, 132, 131, 131, 130, 130, 129, 128, 128, 127, 127, 126, 126, 126, 126, 125, 125, 125, 125, 125, 125, 125, 125, 126, 126, 127, 127, 127, 128, 128, 128, 129, 129, 130, 130, 131, 131, 131, 132, 132, 132, 132, 132, 133, 133, 133, 133, 132, 132, 132, 132, 132, 131, 131, 131, 130, 130, 130, 129, 129, 129, 129, 129, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 129, 129, 129, 129, 130, 130, 130, 130, 130, 130, 130, 130, 130, 130, 130, 130, 130, 130, 129, 129, 129, 129, 128, 128, 127, 127, 126, 126, 126, 126, 126, 125, 125, 125, 125, 125, 126, 126, 126, 126, 127, 127, 128, 128, 129, 129, 130, 131, 131, 132, 133, 133, 133, 134, 134, 134, 134, 135, 135, 135, 135, 135, 134, 134, 133, 133, 132, 131, 131, 130, 129, 129, 128, 127, 127, 126, 125, 125, 124, 124, 123, 123, 123, 123, 123, 123, 123, 123, 124, 124, 125, 125, 126, 126, 127, 128, 128, 129, 130, 130, 131, 131, 132, 132, 132, 133, 133, 133, 133, 133, 133, 133, 132, 132, 132, 131, 131, 130, 130, 129, 129, 128, 128, 127, 127, 127, 126, 126, 126, 126, 126, 125, 125, 125, 126, 126, 126, 126, 127, 127, 127, 127, 128, 128, 129, 129, 129, 130, 130, 130, 131, 131, 131, 132, 131, 131, 131, 131, 131, 131, 131, 131, 130, 130, 130, 129, 129, 128, 128, 128, 127, 127, 126, 126, 125, 125, 125, 124, 124, 124, 124, 124, 123, 123, 123, 123, 123, 124, 124, 124, 124, 124, 125, 125, 125, 126, 126, 126,
                                                                                                                                
};

int menuSelect = 0;
int currentHour=0;
int currentMinute=0;
int pastMinute=0;
int x,y, setHour, setMin, alarmHour, alarmMin, shutHour, shutMin, statCount, stateSensor;
bool state;

DateTime now;


void setup(void) {
  Serial.begin(BAUD);
  Serial.println("Water Pump Controller V 0.1");
  pinMode(RTCPOSITIVE, OUTPUT); // rtc +5V
  pinMode(RTCNEGATIVE, OUTPUT); // rtc GND
  digitalWrite(RTCPOSITIVE, HIGH);
  digitalWrite(RTCNEGATIVE, LOW);
  pinMode(SSRPIN, OUTPUT); // init for solid state relay
    
  
  RTC.begin(); 
  if (! RTC.isrunning()) {Serial.println("RTC is NOT running!");}
  now = RTC.now();
  currentHour=now.hour();
  currentMinute=now.minute();
  pastMinute=currentMinute;
  /*
  Serial.print(currentHour);
  Serial.print(":");
  Serial.println(currentMinute);
  */
  
  delay(10);
  setHour = 0;
  setMin = 0;
  alarmHour = 0;
  alarmMin = 0;
  shutHour = 0;
  shutMin = 0;
  x = 0;
  y = 0;
  stateSensor = 0;

 
  
  //startPlayback(R2D2, sizeof(R2D2)); //connect speaker positive wire to pin 11 and negative to ground 
  // to stop use stopPlayback(); function
  tft.reset();
  tft.begin(0x9341); // SDFP5408
  tft.setRotation(0); // Need for the Mega, please changed for your choice or rotation initial
  drawBorder();

  // Initial screen
  
  tft.setCursor (35, 85);
  tft.setTextSize (3);
  tft.setTextColor(YELLOW);
  tft.println("Water Pump");
  tft.setCursor (35, 120);
  tft.println("Controller");
  tft.setCursor (90, 160);
  tft.setTextSize (2);
  tft.println("V 0.1");
  tft.setCursor (75, 250);
  tft.setTextSize (1);
  tft.setTextColor(DARKPINK);
  tft.println("Touch to proceed");

  
  delay(5000);
  //waitOneTouch();
  tft.fillScreen(DARKGREY);
  statusBar(currentHour, currentMinute);
  drawMenu();
  pinMode(13, OUTPUT);
}



void loop()
{
  now = RTC.now();
  currentHour=now.hour();
  currentMinute=now.minute();
  if (currentMinute != pastMinute){
    statusBar(currentHour, currentMinute);
    pastMinute = currentMinute;
  }
  // touch related
  digitalWrite(13, HIGH);
  TSPoint p = ts.getPoint();
  delay(1);
  digitalWrite(13, LOW);
  //pinMode(XP, OUTPUT);
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);
  //pinMode(YM, OUTPUT);
  if (p.z > MINPRESSURE && p.z < MAXPRESSURE) {
    p.x = map(p.x, TS_MINX, TS_MAXX, 0, tft.width());
    p.y = map(p.y, TS_MINY, TS_MAXY, 0, tft.height());
    x=p.x;
    y=p.y;
    menuSelect = menuSelected(x, y);
  }
  if ( menuSelect!=0 ){
    options(menuSelect);
  }

  if ( button.isPressed() && button.stateChanged() ) { 
    Serial.print("button status changed");
    stateSensor++;
  }
  if ( !button.isPressed() && button.stateChanged() ) { 
    Serial.print("button status changed");
    //digitalWrite(SSRPIN, LOW);
      Serial.println("Button not pressed");
    //stateSensor++;
  }
  
  if ( stateSensor == 1 ) {
    if ( button.isPressed() ){
      //digitalWrite(SSRPIN, HIGH);
      Serial.println("Button pressed");
    }
    else {
      //digitalWrite(SSRPIN, LOW);
      Serial.println("Button not pressed");
    }
    stateSensor = 0;
  }
        

}



// utility functions

TSPoint waitOneTouch() {
 // wait 1 touch to exit function
  TSPoint p;
  do {
    p= ts.getPoint(); 
    pinMode(XM, OUTPUT); //Pins configures again for TFT control
    pinMode(YP, OUTPUT);
  } while((p.z < MINPRESSURE )|| (p.z > MAXPRESSURE));
  return p;
}

// loop function for selected option
void options (int opt){
  int x1 = 0;
  int y1 = 0;
  int buttonSelect = 0;
  drawSubmenu(opt);
  delay(50);
  if (opt != 1) {
    reDrawInput(opt);
  }

  // while "back" button is not pressed, loop
  while (!back(x1, y1)){
  now = RTC.now();
  currentHour=now.hour();
  currentMinute=now.minute();
  if (currentMinute != pastMinute){
    statusBar(currentHour, currentMinute);
    pastMinute = currentMinute;
  }
  digitalWrite(13, HIGH);
  TSPoint p = ts.getPoint();
  digitalWrite(13, LOW);
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);
  if (p.z > MINPRESSURE && p.z < MAXPRESSURE) {
    p.x = map(p.x, TS_MINX, TS_MAXX, 0, tft.width());
    p.y = map(p.y, TS_MINY, TS_MAXY, 0, tft.height());
    x1 = p.x;
    y1 = p.y;
    buttonSelect = buttonSelected(x1, y1, opt);
    }
    if (buttonSelect == 1){
      reDrawInput(opt);
      buttonSelect = 0;
      delay(150);
      
    }
  
 }

  // on press of "back" button
  drawMenu();
  x=tft.height();
  y=tft.width();
  menuSelect=0;
}

// redrawing input line when pressing on + - buttons in submenu
void reDrawInput(int optRe){
  tft.fillRect(BORDER, STATUSBAR+BORDER, MENUW*2+BORDER, STATUSBAR, YELLOW);
  tft.setTextSize (3);
  tft.setTextColor(DARKPINK);
  switch (optRe){
    case 1:
    break;
    case 2:
    tft.setCursor(BORDER+45, STATUSBAR+BORDER+10);
    tft.print(setHour);
    tft.setCursor((BORDER*2+MENUW*2)/2-3, STATUSBAR+BORDER+10);
    tft.print(":");
    tft.setCursor((BORDER*2+MENUW*2)/2+45, STATUSBAR+BORDER+10);
    tft.print(setMin);
    break;
    case 3:
    tft.setCursor(BORDER+45, STATUSBAR+BORDER+10);
    tft.print(alarmHour);
    tft.setCursor((BORDER*2+MENUW*2)/2-3, STATUSBAR+BORDER+10);
    tft.print(":");
    tft.setCursor((BORDER*2+MENUW*2)/2+45, STATUSBAR+BORDER+10);
    tft.print(alarmMin);
    break;
    case 4:
    tft.setCursor(BORDER+45, STATUSBAR+BORDER+10);
    tft.print(shutHour);
    tft.setCursor((BORDER*2+MENUW*2)/2-3, STATUSBAR+BORDER+10);
    tft.print(":");
    tft.setCursor((BORDER*2+MENUW*2)/2+45, STATUSBAR+BORDER+10);
    tft.print(shutMin);
    break;
    default:
    break;
  }
}
// drawing submenu based on menu chosen
void drawSubmenu (int optSub){
  tft.setTextSize (2);
  tft.setTextColor(DARKGREY);
  secondMenublank();
  tft.fillRect(BORDER, tft.height()-BORDER-STATUSBAR, MENUW*2+BORDER, STATUSBAR, DARKPINK);
  tft.setCursor (tft.width()/2-25, tft.height()-BORDER-STATUSBAR+12);
  tft.println("BACK");

  switch (optSub){
    case 1:
    

    break;
    case 2:
    timeButtons();
    break;
    case 3:
    timeButtons();
    break;
    case 4:
    timeButtons();
    break;
    default:
    break;
  }



  
}
// drawing menu buttons for + - submenu
void timeButtons (){
  tft.setTextSize (5);
  tft.setTextColor(GREEN);

  tft.fillRect(BORDER, STATUSBAR+BORDER, MENUW*2+BORDER, STATUSBAR, YELLOW);
  tft.fillRect(BORDER, STATUSBAR*2+BORDER*2, MENUW, TBUTTON, LIGHTGREY);
  tft.setCursor (BORDER+40, STATUSBAR*2+BORDER*2+20);
  tft.println("+");
  tft.fillRect(BORDER*2+MENUW, STATUSBAR*2+BORDER*2, MENUW, TBUTTON, LIGHTGREY);
  tft.setCursor (BORDER*2+MENUW+40, STATUSBAR*2+BORDER*2+20);
  tft.println("+");
  tft.fillRect(BORDER, STATUSBAR*2+BORDER*3+TBUTTON, MENUW, TBUTTON, LIGHTGREY);
  tft.setCursor (BORDER+40, STATUSBAR*2+BORDER*3+TBUTTON+20);
  tft.println("-");
  tft.fillRect(BORDER*2+MENUW, STATUSBAR*2+BORDER*3+TBUTTON, MENUW, TBUTTON, LIGHTGREY);
  tft.setCursor (BORDER*2+MENUW+40, STATUSBAR*2+BORDER*3+TBUTTON+20);
  tft.println("-");
    
}
// function recognising touch press on submenu and adjusting time selection accordingly
int buttonSelected (int xb, int yb, int optSel){
  switch (optSel){
    case 1:
    break;
    case 2:
    if (xb > BORDER && yb > STATUSBAR*2+BORDER*2 && xb < BORDER+MENUW && yb < STATUSBAR*2+BORDER*2+TBUTTON) {
      setHour++;
      if (setHour > 23) { setHour = 0; }
      return 1;
    }
    if (xb > BORDER*2+MENUW && yb > STATUSBAR*2+BORDER*2 && xb < BORDER*2+MENUW*2 && yb < STATUSBAR*2+BORDER*2+TBUTTON) {
      setMin++;
      if (setMin > 59) { setMin = 0; }
      return 1;
    }
    if (xb > BORDER && yb > STATUSBAR*2+BORDER*3+TBUTTON && xb < BORDER+MENUW && yb < STATUSBAR*2+BORDER*3+TBUTTON*2) {
      setHour--;
      if (setHour < 0) { setHour = 23; }
      return 1;
    }
    if (xb > BORDER*2+MENUW && yb > STATUSBAR*2+BORDER*3+TBUTTON && xb < BORDER*2+MENUW*2 && yb < STATUSBAR*2+BORDER*3+TBUTTON*2) {
      setMin--;
      if (setMin < 0) { setMin = 59; }
      return 1;
    }
    setTime(setHour, setMin);
    break;
    
    case 3:
    if (xb > BORDER && yb > STATUSBAR*2+BORDER*2 && xb < BORDER+MENUW && yb < STATUSBAR*2+BORDER*2+TBUTTON) {
      alarmHour++;
      if (alarmHour > 23) { alarmHour = 0; }
      return 1;
    }
    if (xb > BORDER*2+MENUW && yb > STATUSBAR*2+BORDER*2 && xb < BORDER*2+MENUW*2 && yb < STATUSBAR*2+BORDER*2+TBUTTON) {
      alarmMin++;
      if (alarmMin > 59) { alarmMin = 0; }
      return 1;
    }
    if (xb > BORDER && yb > STATUSBAR*2+BORDER*3+TBUTTON && xb < BORDER+MENUW && yb < STATUSBAR*2+BORDER*3+TBUTTON*2) {
      alarmHour--;
      if (alarmHour < 0) { alarmHour = 23; }
      return 1;
    }
    if (xb > BORDER*2+MENUW && yb > STATUSBAR*2+BORDER*3+TBUTTON && xb < BORDER*2+MENUW*2 && yb < STATUSBAR*2+BORDER*3+TBUTTON*2) {
      alarmMin--;
      if (alarmMin < 0) { alarmMin = 59; }
      return 1;
    }
    break;
    
    case 4:
    if (xb > BORDER && yb > STATUSBAR*2+BORDER*2 && xb < BORDER+MENUW && yb < STATUSBAR*2+BORDER*2+TBUTTON) {
      shutHour++;
      if (shutHour > 23) { shutHour = 0; }
      return 1;
    }
    if (xb > BORDER*2+MENUW && yb > STATUSBAR*2+BORDER*2 && xb < BORDER*2+MENUW*2 && yb < STATUSBAR*2+BORDER*2+TBUTTON) {
      shutMin++;
      if (shutMin > 59) { shutMin = 0; }
      return 1;
    }
    if (xb > BORDER && yb > STATUSBAR*2+BORDER*3+TBUTTON && xb < BORDER+MENUW && yb < STATUSBAR*2+BORDER*3+TBUTTON*2) {
      shutHour--;
      if (shutHour < 0) { shutHour = 23; }
      return 1;
    }
    if (xb > BORDER*2+MENUW && yb > STATUSBAR*2+BORDER*3+TBUTTON && xb < BORDER*2+MENUW*2 && yb < STATUSBAR*2+BORDER*3+TBUTTON*2) {
      shutMin--;
      if (shutMin < 0) { shutMin = 59; }
      return 1;
    }
    break;
    default:
    break;
  }
  return 0;
}
// back button returns TRUE or FALSE on press
bool back (int xx, int yy){
  if ( xx > BORDER && xx < BORDER*2+MENUW*2 && yy > tft.height()-BORDER-STATUSBAR && yy < tft.height()-BORDER) { 
  return true; 
}
  else { return false; }
}
// drawing border around buttons
void drawBorder (){
  tft.fillScreen(DARKGREY);
  tft.fillRect(BORDER, BORDER, (tft.width() - BORDER * 2), (tft.height() - BORDER * 2), LIGHTGREY);
}
// drawing status bar
void statusBar (int thour, int tmin){
  tft.fillRect(0, 0, tft.width(), STATUSBAR, LIGHTGREY);
  drawTime(thour, tmin);
}
// drawing main menu
void drawMenu (){
  tft.setTextSize (2);
  tft.setTextColor(YELLOW);
  tft.fillRect(0, STATUSBAR, tft.width(), tft.height(), DARKGREY);
  tft.fillRect(BORDER, STATUSBAR+BORDER, MENUW, MENUH, LIGHTGREY);
  tft.setCursor (BORDER+17, STATUSBAR+BORDER+55);
  tft.println("Report");
  tft.fillRect(MENUW+BORDER*2, STATUSBAR+BORDER, MENUW, MENUH, LIGHTGREY);
  tft.setCursor (MENUW+BORDER*2+5, STATUSBAR+BORDER+55);
  tft.println("Set Time");
  tft.fillRect(BORDER, STATUSBAR+MENUH+BORDER*2, MENUW, MENUH, LIGHTGREY);
  tft.setCursor (BORDER+30, STATUSBAR+MENUH+BORDER*2+40);
  tft.println("Till");
  tft.setCursor (BORDER+25, STATUSBAR+MENUH+BORDER*2+70);
  tft.println("Alarm");
  tft.fillRect(MENUW+BORDER*2, STATUSBAR+MENUH+BORDER*2, MENUW, MENUH, LIGHTGREY);
  tft.setCursor (MENUW+BORDER*2+30, STATUSBAR+MENUH+BORDER*2+40);
  tft.println("Till");
  tft.setCursor (MENUW+BORDER*2+10, STATUSBAR+MENUH+BORDER*2+70);
  tft.println("ShutOFF");
}
// touch press function for main menu selection
int menuSelected(int x, int y){
  int a;
  if(x > BORDER && x < BORDER+MENUW && y > STATUSBAR+BORDER && y < MENUH+BORDER+STATUSBAR) { a = 1; }
  if(x > MENUW+BORDER*2 && x < MENUW*2+BORDER*2 && y > STATUSBAR+BORDER && y < MENUH+BORDER+STATUSBAR) { a = 2; }
  if(x > BORDER && x < BORDER+MENUW && y > MENUH+STATUSBAR+BORDER*2 && y < MENUH*2+STATUSBAR+BORDER*2) { a = 3; }
  if(x > MENUW+BORDER*2 && x < MENUW*2+BORDER*2 && y > MENUH+STATUSBAR+BORDER*2 && y < MENUH*2+STATUSBAR+BORDER*2) { a = 4; }
  Serial.print("A: ");
  Serial.println(a);
  return a;
}
// clearning screen before drawing submenu
void secondMenublank (){
    tft.fillRect(0, STATUSBAR, tft.width(), tft.height() - STATUSBAR, DARKGREY);
}
// status bar refresh
void drawTime(int a, int b){
    String timeString = "";
    timeString+=a;
    timeString+=":";
    timeString+=b;
    String lastTimeON = "09:16";
    int duration = 34;
    int galUsed = 14;
    
    tft.setTextSize (2);
    tft.setTextColor(GREEN);
    tft.setCursor(170, 13);
    tft.println(timeString);
    
    tft.setTextSize (1);
    tft.setTextColor(DARKPINK);
    tft.setCursor(10, 6);
    tft.println("Last time ON:");
    tft.setCursor(10, 16);
    tft.println("Duration:");
    tft.setCursor(85, 16);
    tft.println("minutes");
    tft.setCursor(10, 26);
    tft.println("Gallons used:");
    
    tft.setTextColor(GREEN);
    tft.setCursor(90, 6);
    tft.println(lastTimeON);
    tft.setCursor(65, 16);
    tft.println(duration);
    tft.setCursor(88, 26);
    tft.println(galUsed);
}
// Convert normal decimal numbers to binary coded decimal
byte decToBcd(byte val){
  return ( (val/10*16) + (val%10) );
}

void setTime(byte hour, byte minute) {

  Wire.beginTransmission(DS1307_ADDRESS);
  Wire.write(zero); //stop Oscillator
  Wire.write(decToBcd(10));
  Wire.write(decToBcd(minute));
  Wire.write(decToBcd(hour));
  Wire.write(zero); //start
  Wire.endTransmission();

}
 



