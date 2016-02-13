/*
Water pump controller V 0.1
Built upon Adafruit TFT library examples
Daniel Lameka 2/12/2016

*/




#include <SPFD5408_Adafruit_GFX.h>    // Core graphics library
#include <SPFD5408_Adafruit_TFTLCD.h> // Hardware-specific library
#include <SPFD5408_TouchScreen.h>

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

Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);


int menuSelect;

void setup(void) {
  Serial.begin(9600);
  Serial.println(F("Paint!"));
  
  tft.reset();
  tft.begin(0x9341); // SDFP5408
  tft.setRotation(0); // Need for the Mega, please changed for your choice or rotation initial

  // Border

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

  // Wait touch
  waitOneTouch();
  tft.fillScreen(DARKGREY);

  statusBar();
  drawMenu();
  pinMode(13, OUTPUT);
}



void loop()
{
  digitalWrite(13, HIGH);
  TSPoint p = ts.getPoint();
  digitalWrite(13, LOW);

  //pinMode(XP, OUTPUT);
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);
  //pinMode(YM, OUTPUT);

  if (p.z > MINPRESSURE && p.z < MAXPRESSURE) {
    p.x = map(p.x, TS_MINX, TS_MAXX, 0, tft.width());
    p.y = map(p.y, TS_MINY, TS_MAXY, 0, tft.height());
    
    menuSelect = menuSelected(p.x, p.y);
  }
  
  
}

// Wait one touch
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


void drawBorder (){
  tft.fillScreen(DARKGREY);
  tft.fillRect(BORDER, BORDER, (tft.width() - BORDER * 2), (tft.height() - BORDER * 2), LIGHTGREY);
}

void statusBar (){
  tft.fillRect(0, 0, tft.width(), STATUSBAR, LIGHTGREY);
  drawTime();
}

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


void drawTime(){
    
    String timeString = "10:33";
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


  
  
