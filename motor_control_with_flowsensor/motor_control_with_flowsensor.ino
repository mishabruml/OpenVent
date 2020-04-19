#include <Wire.h>
#include <sfm3000wedo.h>
#include "ArduinoMotorShieldR3.h"
#include <TimerOne.h>

ArduinoMotorShieldR3 md;

// motor pins
int DIR_A = 12;
int PWM_A = 3;
int current = 0;     // log filtered current reading

// UI inputs
int a1 = analogRead(A1);      // right POT variable
int maxPressurePOT = analogRead(A2);      // left POT variable
int maxPressurePOTconstrianed = 0;
int bpm = 20; // breaths per min

const int freqUpButtonPin = 2;
const int freqDownButtonPin = 6;
const int redLEDPin = A4;
const int blueLEDPin = A5;
unsigned long blueLEDonTime = 0;          // timer to turn on blue LED
unsigned long redLEDonTime = 0;          // timer to turn on blue LED
unsigned long POTblueLEDonTime = 0;       // timer for blue LED flashing with POT
int redLEDState = HIGH;         // the current state of the output pin
int blueLEDState = HIGH;         // the current state of the output pin
int buttonState2 = 0;             // the current reading from the input pin
int lastButtonState2 = LOW;   // the previous reading from the input pin
unsigned long lastDebounceTime2 = 0;  // the last time the output pin was toggled
int buttonState6 = 0;             // the current reading from the input pin
int lastButtonState6 = LOW;   // the previous reading from the input pin
long lastDebounceTime6 = 0;  // the last time the output pin was toggled
long debounceDelay = 50;    // the debounce time; increase if the output flickers

// LEDs
int redLED = A4;
int blueLED = A5;

unsigned long pauseTimer = millis();      // stop watch for pauses
unsigned long accelTime = millis();
int mSpeed = 0;

int revTimeSetting = 400;       // time it takes to reverse in ms
int revSpeed = 400;             // max speed of reverse stroke
int maxFwdSpeed = -400;         // max speed of motor/acceloration limit
int currentLimit = 1200;        // current limit

int postInhaleDwell = 0;
unsigned long postExhaleDwell = 0;
unsigned long lastBreathTime = 0;
unsigned long breathPeriod = 0;

void setup()
{
  md.init();
  Wire.begin();

  // set up motor pins
  pinMode(DIR_A, OUTPUT);
  pinMode(PWM_A, OUTPUT);
  
  Serial.begin(115200);
  // Serial.println("OpenVent Bristol");
  // Serial.println("stage\tms\tcurrent\tpressure(pa)\tflow(ml/s)");
}

void loop()
{
  // checkUI();
  // lastBreathTime = millis();
  inhale();
  // postInhalePause();
  // exhale();
  // postExhalePause();
}
