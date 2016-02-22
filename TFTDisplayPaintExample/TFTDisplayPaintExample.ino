/*
Water pump controller V 0.1
Built upon Adafruit TFT library examples
Daniel Lameka 2/12/2016

*/




#include "SPFD5408_Adafruit_GFX.h"    // Core graphics library
#include "SPFD5408_Adafruit_TFTLCD.h" // Hardware-specific library
#include "SPFD5408_TouchScreen.h"
#include "Wire.h"   // for time module lib
#include "RTClib.h" // time module lib
#include "PCM.h"
#include "SPI.h"
#include "SD.h"

#define DS1307_ADDRESS 0x68
#define BAUD 9600
#define ON LOW
#define OFF HIGH
#define RELAY 4

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

/*
const unsigned char R2D2[] PROGMEM = {
80, 78, 152, 189, 133, 70, 91, 167, 185,
};
*/
int menuSelect = 0;
int currentHour=0;
int currentMinute=0;
int pastMinute=0;
int x,y, setHour, setMin, alarmHour, alarmMin, shutHour, shutMin, statCount;

DateTime now;


void setup(void) {
  Serial.begin(BAUD);
  Serial.println("Water Pump Controller V 0.1");
  pinMode(53, OUTPUT);
  digitalWrite(53, HIGH);
  
  RTC.begin(); 
  if (! RTC.isrunning()) {Serial.println("RTC is NOT running!");}
  now = RTC.now();
  currentHour=now.hour();
  currentMinute=now.minute();
  Serial.print(currentHour);
    Serial.print(":");
    Serial.println(currentMinute);

  
  delay(10);

  setHour = 0;
  setMin = 0;
  alarmHour = 0;
  alarmMin = 0;
  shutHour = 0;
  shutMin = 0;

 
  
   //startPlayback(R2D2, sizeof(R2D2)); //connect speaker positive wire to pin 11 and negative to ground 
  // to stop use stopPlayback(); function
  tft.reset();
  tft.begin(0x9341); // SDFP5408
  tft.setRotation(0); // Need for the Mega, please changed for your choice or rotation initial

  drawBorder();

/*
 ;
 */   
  pastMinute=currentMinute;
  
  
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
  

  statusBar(currentHour, currentMinute);
  drawMenu();
  pinMode(13, OUTPUT);
  

  x=0;
  y=0;

 
  
}





void loop()
{
  
  digitalWrite(13, HIGH);
  TSPoint p = ts.getPoint();
  delay(1);
  digitalWrite(13, LOW);
  
  //pinMode(XP, OUTPUT);
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);
  //pinMode(YM, OUTPUT);
  
  now = RTC.now();
  currentHour=now.hour();
  currentMinute=now.minute();

   Serial.print(currentHour);
    Serial.print(":");
    Serial.println(currentMinute);
 
  if (currentMinute != pastMinute){
    statusBar(currentHour, currentMinute);
    pastMinute = currentMinute;
    Serial.print(currentHour);
    Serial.print(":");
    Serial.println(currentMinute);
  }

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
  digitalWrite(13, HIGH);
  TSPoint p = ts.getPoint();
  digitalWrite(13, LOW);
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);
  
  now = RTC.now();
  currentHour=now.hour();
  currentMinute=now.minute();
  if (currentMinute != pastMinute){
    statusBar(currentHour, currentMinute);
    pastMinute = currentMinute;
  }
 
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
 



