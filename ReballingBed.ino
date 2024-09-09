#include <TickTwo.h>
#include <LiquidCrystal_I2C.h>
#include "max6675.h"
#include <Wire.h>

#define CS0pin 15 //D8
#define CS1pin 0  //D3
#define CLKpin 14 //D5
#define SOpin 12  //D6
#define zero_detection 7 //SD0
#define Triac 2 //D4
#define Button1 9 //SD2
#define Button2 10 //SD3
MAX6675 tc1(CLKpin, CS0pin, SOpin);
MAX6675 tc2(CLKpin, CS1pin, SOpin);
LiquidCrystal_I2C lcdScreen(0x27, 16, 2); //Uses pin 5(D1)SCL and 4(D2)SDA
int t = 0;
void ShowCount(){
  Serial.print("Count: ");
  Serial.println(t);
  t=0;
}
int dim = 1000;
TickTwo tickerObject(ShowCount, 1000); 
TickTwo tempTimer(updateTemp, 400);
TickTwo timeTimer(updateTime, 1000);
TickTwo triacTimer(SetTriacHigh, 0, 1, MICROS);

bool optionSelected = false;

bool zero_crossed = true;
int TempSet = 60;
int maximumDimValue = 7400;

int kp = 400;
int ki = 0;
int kd = 0;

float pidP = 0;
float pidI = 0;
float pidD = 0;
float PID = 0;
long prevPidTime = 0;
float PID_lastError = 0;
volatile float temp1 = 0;

void setup() {
  lcdScreen.begin(16, 2);
  lcdScreen.init();
  lcdScreen.backlight();
  lcdScreen.write((byte)0);

  pinMode(Triac, OUTPUT);
  pinMode(zero_detection, INPUT);
  pinMode(Button2, INPUT);
  pinMode(Button1, INPUT);
  tickerObject.start();
  tempTimer.start();
  timeTimer.start();
  Serial.begin(9600);

  setParamScreen();
}

void setOptionScreen(){
  lcdScreen.print("testa");
  lcdScreen.setCursor(4,0);
  lcdScreen.blink();
  lcdScreen.setCursor(5,0);
  lcdScreen.blink();
}

bool tempUpPressed = false;
bool tempDownPressed = false;
void loop() {
  if(optionSelected){
    // lcdScreen.blink();
  }
  else{
    updateTimers();
    zeroDetect();
    checkSetPointsButtons();
    calculatePid();
  }
}

void updateTimers(){
  tickerObject.update();
  tempTimer.update();
  timeTimer.update();
  triacTimer.update();
}

void setParamScreen(){
  lcdScreen.setCursor(0, 0);
  lcdScreen.print("Tu:");
  lcdScreen.setCursor(0, 1);
  lcdScreen.print("Tb:");
  lcdScreen.setCursor(8, 0);
  lcdScreen.print("T:");
  lcdScreen.setCursor(8, 1);
  lcdScreen.print("Ts:");
  lcdScreen.print(TempSet);
}

void zeroDetect(){
  int value = digitalRead(zero_detection);
  if(value == 1 && zero_crossed == false){
    zero_crossed = true;
    t+=1;
    digitalWrite(Triac, LOW);
    triacTimer.interval(maximumDimValue - PID);
    triacTimer.start();
  }
  else if(value == 0 && zero_crossed == true){
    zero_crossed = false;
    t+=1;
  }
}

void checkSetPointsButtons(){
  int tempUpValue = digitalRead(Button2);
  int tempDownValue = digitalRead(Button1);
  if(tempUpValue == 1 && !tempUpPressed){
      tempUpPressed = true;
      TempSet += 5;
      lcdScreen.setCursor(11, 1);
      lcdScreen.print("   ");
      lcdScreen.setCursor(11, 1);
      lcdScreen.print(TempSet);
    }
    else if(tempUpValue == 0){
      tempUpPressed = false;
    }
    if(tempDownValue == 1 && !tempDownPressed){
      tempDownPressed = true;
      TempSet -= 5;
      lcdScreen.setCursor(11, 1);
      lcdScreen.print("   ");
      lcdScreen.setCursor(11, 1);
      lcdScreen.print(TempSet);
    }
    else if(tempDownValue == 0){
      tempDownPressed = false;
    }
}

void calculatePid(){
  float PID_error = TempSet - temp1;
  long Time = millis();
  float elapsedTime = Time - prevPidTime;
  pidP = kp *(PID_error);                         //Calculate the P value
  pidI += ki * PID_error * elapsedTime;               //Calculate the I value
  pidD = kd * ((PID_error - PID_lastError)/elapsedTime);
  PID = pidP + pidI + pidD;
  PID_lastError = PID_error;
  prevPidTime = Time;
  //We define firing delay range between 0 and 7400. Read above why 7400!!!!!!!
  if(PID < 0)
  {      PID = 0;       }
  if(PID > 7400)
  {      PID = 7400;    } 
  Serial.println(PID);
}

void SetTriacHigh(){
  digitalWrite(Triac, HIGH);
  triacTimer.stop();
}
volatile int lastTemp1 = 0;
volatile int lastTemp2 = 0;
void updateTemp(){
//   int status = tc1.read();
  temp1 = tc1.readCelsius();
  float temp2 = tc2.readCelsius();
  lcdScreen.setCursor(4,0);
  lcdScreen.print(int(trunc(temp1)));
  lcdScreen.print(" ");
  lcdScreen.setCursor(4,1);
  lcdScreen.print(int(trunc(temp2)));
  lcdScreen.print(" ");
}

volatile int timePassed = 0;
void updateTime(){
  lcdScreen.setCursor(11,0);
  timePassed += 1;
  lcdScreen.print(timePassed);
  if(timePassed < 10){
    lcdScreen.setCursor(12, 0);
  }
  else if(timePassed < 100){
    lcdScreen.setCursor(13, 0);
  }
  else if(timePassed < 1000){
    lcdScreen.setCursor(14, 0);
  }
  else if(timePassed < 10000){
    lcdScreen.setCursor(15, 0);
  }
  lcdScreen.print("s");
}