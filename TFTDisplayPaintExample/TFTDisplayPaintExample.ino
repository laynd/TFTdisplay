/*
Water pump controller V 1.23
Built upon Adafruit TFT library examples

Using Arduino Mega

Daniel Lameka 2/12/2016

Audio output is connected to slave arduino board communication via Serial1

Using SD card to store settings and usage log

pin 53 SD pin
pin 39 dedicated to +5 of RTC module
pin 41 dedicated to GND of RTC module

pin - 24 reset button with 10K pull up 
pin 25 - sensor with 10K pull up

pin 48 - SSR relay
       
*/




#include "SPFD5408_Adafruit_GFX.h"    // Core graphics library
#include "SPFD5408_Adafruit_TFTLCD.h" // Hardware-specific library
#include "SPFD5408_TouchScreen.h"
#include "Wire.h"   // for time module lib
#include "RTClib.h" // time module lib
#include "SPI.h"
#include "SD.h"
#include "Button.h"

#define GALMIN 7.5  //gallons per hour need to adjust
#define DS1307_ADDRESS 0x68
#define BAUD 9600
#define OFF LOW
#define ON HIGH
#define BUTTONPIN 24
#define SENSORPIN 25
#define SSRPIN 48
#define RTCPOSITIVE 39
#define RTCNEGATIVE 41
#define SDPIN 53

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
#define GRAPHB 53
#define GRAPHS 13
#define GRAPHMX 175
#define GRAPHSI 14
#define GRAPHBI 54

Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

Button button = Button(BUTTONPIN, BUTTON_PULLUP_INTERNAL, true, 100);
Button sensor = Button(SENSORPIN, BUTTON_PULLUP_INTERNAL, true, 1000);


int menuSelect = 0;
int currentHour=0;
int currentMinute=0;
int pastMinute=0;
int x,y, setHour, setMin, alarmHour, alarmMin, shutHour, shutMin, statCount, stateSensor;
int sensorTriggerMin, sensorTriggerHour, statTH, statTM, PSDown, str, timePassedForStat;
long timePassedONtrigger, alarmsec, shutoffsec, dataread, settingsRead; 
bool state, firstTimeTrigger, readSet;
String datareadST, settingsReadST;

DateTime now;
DateTime alarm;
TimeSpan difference;
File WPFile;
File SettingF;

void setup(void) {
  Serial.begin(BAUD);
  Serial1.begin(BAUD);
  Serial.println("Water Pump Controller V 1.20");
  pinMode(RTCPOSITIVE, OUTPUT); // rtc +5V
  pinMode(RTCNEGATIVE, OUTPUT); // rtc GND
  digitalWrite(RTCPOSITIVE, HIGH);
  digitalWrite(RTCNEGATIVE, LOW);
  pinMode(SSRPIN, OUTPUT); // init for solid state relay
    
  
  RTC.begin(); 
  if (! RTC.isrunning()) {Serial.println("RTC is NOT running!");}
  //RTC.adjust(DateTime(__DATE__, __TIME__));
  now = RTC.now();
  currentHour=now.hour();
  currentMinute=now.minute();
  pastMinute=currentMinute;
  
  
  
  
  delay(10);
  timePassedForStat=0;
  setHour = 0;
  setMin = 0;
  alarmHour = 1;
  alarmMin = 0;
  shutHour = 1;
  shutMin = 0;
  x = 0;
  y = 0;
  stateSensor = 0;
  firstTimeTrigger=true;
  sensorTriggerMin=-1;
  sensorTriggerHour=-1;
  timePassedONtrigger=0;
  statTH=0;
  statTM=0;
  alarmsec=3600L;
  shutoffsec=3600L;
  PSDown=0;
  readSet=true;
  datareadST=""; 
  settingsReadST="";
  str=0;
  

  if (!SD.begin(SDPIN)) {
    Serial.println("Card failed, or not present");
  }
  // file settings.set for storing settings on SD card and restoring in case of power down 
  
  // reading settings 
  readSettings();
  SettingF = SD.open("settings.set", FILE_WRITE);
  SettingF.close();


  
  
  // open a new file and immediately close it:
  Serial.println("Creating wp.log...");
  WPFile = SD.open("wp.log", FILE_WRITE);
  WPFile.close();
  // Check to see if the file exists:
  if (SD.exists("wp.log")) {
    Serial.println("wp.log exists.");
  } else {
    Serial.println("wp.log doesn't exist.");
  }
/*
  // delete the file:
  Serial.println("Removing file");
  SD.remove("wp.log");
  */
  

  // readinng data file 
  /*
  WPFile = SD.open("wp.log");
  if (WPFile) {
    while (WPFile.available()) {
      //Serial.write(WPFile.read());
      dataread=WPFile.read();
      if (isDigit(dataread)) {
        datareadST += (char)dataread;
      }
      //if  (dataread == 35) {break;}
      if (dataread == '\n') {
        dataread=datareadST.toInt();
        Serial.println(dataread);
        datareadST="";
      }
      //
      
      
    }
    WPFile.close();
  }

 */
  
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
  tft.println("V 1.23");
  tft.setCursor (75, 250);
  tft.setTextSize (1);
  

  
  delay(1000);
  //waitOneTouch();
  tft.fillScreen(DARKGREY);
  statusBar(currentHour, currentMinute, 0, 0);
  drawMenu();
  pinMode(13, OUTPUT);
  
}

/*********************************************MAIN LOOP*****************************************/

void loop()
{
  now = RTC.now();
  currentHour=now.hour();
  currentMinute=now.minute();
  setHour = currentHour;
  setMin = currentMinute;

  if (currentHour == 0 && readSet){
    readSettings();
    readSet = false;
  }
  if (currentHour > 0){ readSet = true; } 
  
    
  // sensor and button related processes
  if ( PSDown == 0 ){ // checking if timer didn't go over shut off mark if it did pump will be shut down until user reset
    
    //if (currentMinute != pastMinute){
     //   Serial.print("PSDown:");
      //  Serial.println(PSDown);
     //   }
    // checking if sensor is activated
    if ( sensor.isPressed() ){  
      delay(50);
      if ( firstTimeTrigger ){  // triggering turn on of SSR once, obtaining time stamp of event 
        digitalWrite(SSRPIN, ON);
        Serial.println("SSR ON");
        alarm=RTC.now();
        statTH=alarm.hour();
        statTM=alarm.minute();
        firstTimeTrigger = false;
      }
      if (currentMinute != pastMinute){ 
        timePassedONtrigger=timePassed(); //refreshing time passed on sensor trigger once per minute
        timePassedForStat=timePassedONtrigger; //sending data to status bar
        }
    
  }else {
    delay(50);
    if ( !firstTimeTrigger ){ 
      digitalWrite(SSRPIN, OFF);
      Serial.println("SSR OFF");
      firstTimeTrigger = true;
      alarm=RTC.now();
      timePassedONtrigger=timePassed();
      }
    }
  }
  //sending signal to secondary board to sound alarm
  if (timePassedONtrigger > alarmsec) {  
    if (currentMinute != pastMinute){
      Serial1.print("@");
      Serial.println("ALARM");
    }
  }
  // triggering pump shot down until user reset
  if ( timePassedONtrigger > shutoffsec ){ 
      PSDown=1;
      if (currentMinute != pastMinute){
      Serial.println("PumpShutDown triggered");
      }  
  }
  // triggering pump shot down until user reset
  if (PSDown==1){
    if (currentMinute != pastMinute){
      digitalWrite(SSRPIN, OFF);
      Serial.println("EMERGENCY SSR OFF");
     }
  }
  
  // reset on button press (might need to add screen interaction as well)
  if ( button.isPressed() ){ 
    delay(50);
    PSDown=0;
    alarm=RTC.now();
    timePassedONtrigger=timePassed();
    Serial.println("SYSTEM RESET");
  }
    
  
  // updating time in status bar
  if (currentMinute != pastMinute){
    statusBar(currentHour, currentMinute, statTH, statTM);
    pastMinute = currentMinute;
    Serial.print("time passed on trigger:");
    Serial.println(timePassedONtrigger);
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
    menuSelect = menuSelected(x, y); // checking if menu buttons were pressed
    Serial1.print("&");
  }
  
  
  
  
  // if menu selected go to loop "options"
  if ( menuSelect!=0 ){
    options(menuSelect);
  }
}

/*********************************************MAIN LOOP*****************************************/

/******************************** loop function for selected option ***************************/
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
    statusBar(currentHour, currentMinute, statTH, statTM);
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
    Serial1.print("&");
    }
    if (buttonSelect == 1){
      reDrawInput(opt);
      buttonSelect = 0;
      delay(150);
      
    }
  // sensor and button related processes
  if ( sensor.isPressed() ){
    digitalWrite(SSRPIN, ON);
  }else {
    digitalWrite(SSRPIN, OFF);
  }
  
 }

  // on press of "back" button
  drawMenu();
  x=tft.height();
  y=tft.width();
  menuSelect=0;
}

/******************************** loop function for selected option ***************************/

// utility functions

void drawGraph(){
  int q1, q2;
  q1=0;
  q2=0;
  // GRAPHS 13 (graph start point from left), GRAPHB 53 (graph start point from bottom), GRAPHMX 175 (max height of graph)
  // GRAPHSI 14, GRAPHBI 54
  tft.drawFastHLine(GRAPHS, tft.height()-GRAPHB, tft.width()-GRAPHS*2, GREEN);
  tft.drawFastVLine(GRAPHS, tft.height()-GRAPHB-GRAPHMX, GRAPHMX, GREEN);
  tft.drawFastHLine(GRAPHS-3, tft.height()-GRAPHB-GRAPHMX, 3, GREEN);
  tft.drawFastHLine(GRAPHS-3, tft.height()-GRAPHB-GRAPHMX/2, 3, GREEN);
  tft.drawFastHLine(GRAPHS-3, tft.height()-GRAPHB-GRAPHMX/4-GRAPHMX/2, 3, GREEN);
  tft.drawFastHLine(GRAPHS-3, tft.height()-GRAPHB+GRAPHMX/4-GRAPHMX/2, 3, GREEN);
  tft.drawFastVLine(tft.width()-GRAPHS-1, tft.height()-GRAPHB-GRAPHMX, GRAPHMX, GREEN);
  tft.drawFastHLine(tft.width()-GRAPHS, tft.height()-GRAPHB-GRAPHMX, 3, GREEN);
  tft.drawFastHLine(tft.width()-GRAPHS, tft.height()-GRAPHB-GRAPHMX/2, 3, GREEN);
  tft.drawFastHLine(tft.width()-GRAPHS, tft.height()-GRAPHB-GRAPHMX/4-GRAPHMX/2, 3, GREEN);
  tft.drawFastHLine(tft.width()-GRAPHS, tft.height()-GRAPHB+GRAPHMX/4-GRAPHMX/2, 3, GREEN);
  tft.setTextSize (1);
  tft.setTextColor(CYAN);
  tft.setCursor(GRAPHS+3, tft.height()-GRAPHB-GRAPHMX);
  tft.print("500");
  tft.setTextColor(YELLOW);
  tft.setCursor(tft.width()-GRAPHS*2-7, tft.height()-GRAPHB-GRAPHMX);
  tft.print("120");

  //tft.drawFastHLine(GRAPHSI, tft.height()-GRAPHB-1, 212, WHITE);
  for (int x=15; x <= 190; x++){
    q1++;
    if(q1==6){
    tft.fillRect(GRAPHSI+x-3, tft.height()-GRAPHB-1-x, 3, x, CYAN);
    tft.fillRect(GRAPHSI+x, tft.height()-GRAPHB-1-x, 3, x, YELLOW);
    q1=0;
    q2++;
    Serial.print(q2);
    Serial.print(":");
    Serial.print(GRAPHSI+x);
    Serial.print(":");
    Serial.println(GRAPHSI+x+3);
    }
    
  }

  
  //tft.width()
  //tft.height()
  //tft.fillRect(x,y,x+i,y+i, color);
  //tft.drawFastHLine(BORDER, tft.height()-BORDER, tft.width()-BORDER*2, GREEN);
  //tft.drawFastVLine(x, 0, h, color2);
  //tft.setTextColor(RED);    tft.setTextSize(3);
}

void readSettings(){
  if (SD.exists("settings.set")){ 
    SettingF = SD.open("settings.set");
    if (SettingF) {
      while (SettingF.available()) {
        settingsRead=SettingF.read();
        if (isDigit(settingsRead)) {
         settingsReadST += (char)settingsRead;
        }
        if (settingsRead == '\n') {
          str++;
          settingsRead=settingsReadST.toInt();
          Serial.println(settingsRead);
          settingsReadST="";
          switch (str){
           case 1:
              alarmHour=settingsRead;
              break;
           case 2:
              alarmMin=settingsRead;
              break;
           case 3:
              shutHour=settingsRead;
              break;
           case 4:
              shutMin=settingsRead;
              break;
           default:
              break;
          }
          
      }
    }
      WPFile.close();
      alarmsec=HMtoSec(alarmHour, alarmMin);
      shutoffsec=HMtoSec(shutHour, shutMin);
  }
 }
}

void saveSettings(){
  if (SD.exists("settings.set")) {
    SD.remove("settings.set");
  } else {
    Serial.println("settings.set doesn't exist.");
  }
  delay(5);
  SettingF = SD.open("settings.set", FILE_WRITE);
  if (SettingF){
    SettingF.println(alarmHour , DEC);
    SettingF.println(alarmMin , DEC);
    SettingF.println(shutHour , DEC);
    SettingF.println(shutMin , DEC);
    SettingF.close();
  }
}

void writeToFile (long dataToStore){
  WPFile = SD.open("wp.log", FILE_WRITE);
  if (WPFile) {
    WPFile.println(dataToStore, DEC);
    WPFile.close();
  }
}
  
int secTOmin(int sectomin){
  return sectomin/60;
}

long HMtoSec(int ht, int mt){
  return ht*3600L + mt*60L;
}

long timePassed(){
  long as = 0;
  difference = now - alarm;
  as = difference.totalseconds();
  Serial.print("total seconds passed: ");
  Serial.println(as);
  return as;
}


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



// redrawing input line when pressing on + - buttons in submenu
void reDrawInput(int optRe){
  tft.fillRect(BORDER, STATUSBAR+BORDER, MENUW*2+BORDER, STATUSBAR, YELLOW);
  tft.setTextSize (3);
  tft.setTextColor(DARKPINK);
  switch (optRe){
    case 1:
    break;
    case 2:
    tft.setCursor(BORDER+36, STATUSBAR+BORDER+10);
    printDigits(setHour);
    tft.setCursor((BORDER*2+MENUW*2)/2-3, STATUSBAR+BORDER+10);
    tft.print(":");
    tft.setCursor((BORDER*2+MENUW*2)/2+46, STATUSBAR+BORDER+10);
    printDigits(setMin);
    break;
    case 3:
    tft.setCursor(BORDER+36, STATUSBAR+BORDER+10);
    printDigits(alarmHour);
    tft.setCursor((BORDER*2+MENUW*2)/2-3, STATUSBAR+BORDER+10);
    tft.print(":");
    tft.setCursor((BORDER*2+MENUW*2)/2+46, STATUSBAR+BORDER+10);
    printDigits(alarmMin);
    alarmsec=HMtoSec(alarmHour, alarmMin);
    Serial.print("alarmsec");
    Serial.println(alarmsec);
    break;
    case 4:
    tft.setCursor(BORDER+36, STATUSBAR+BORDER+10);
    printDigits(shutHour);
    tft.setCursor((BORDER*2+MENUW*2)/2-3, STATUSBAR+BORDER+10);
    tft.print(":");
    tft.setCursor((BORDER*2+MENUW*2)/2+46, STATUSBAR+BORDER+10);
    printDigits(shutMin);
    shutoffsec=HMtoSec(shutHour, shutMin);
    Serial.print("shutoffsec");
    Serial.println(shutoffsec);
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
    drawGraph();

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
    saveSettings(); 
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
void statusBar (int thour, int tmin, int trhour, int trmin){
  tft.fillRect(0, 0, tft.width(), STATUSBAR, LIGHTGREY);
  drawTime(thour, tmin, trhour, trmin);
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
void drawTime(int a, int b, int c, int d){
    String timeString = "";
    if (a<10) {
      timeString+="0";
      timeString+=a;
    } else {
      timeString+=a;
    }
    timeString+=":";
    if (b<10) {
      timeString+="0";
      timeString+=b;
    } else {
      timeString+=b;
    }
    
    String lastTimeON = "";
    if (c<10) {
      lastTimeON+="0";
      lastTimeON+=c;
    } else {
      lastTimeON+=c;
    }
    lastTimeON+=":";
    if (d<10) {
      lastTimeON+="0";
      lastTimeON+=d;
    } else {
      lastTimeON+=d;
    }
    
    int duration = secTOmin(timePassedForStat);
    int galUsed = GALMIN*duration;
    
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

 
 // prints leading 0 for time 
void printDigits(int digits){
    if(digits < 10)
    tft.print('0');
    tft.print(digits);
 }



