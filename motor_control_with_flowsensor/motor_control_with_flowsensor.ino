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

int currentLimit = 500;        // current limit

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
  Serial.println("OpenVent Bristol");
  
  // Motor initialisation
  while( current < 590 ){
    setMotor1Speed(-100);
    current = getCurrentM1();
  }
  delay(1000);
  setMotor1Speed(100);
  delay(1000);
  setMotor1Speed(0);
  delay(5000);

  Timer1.initialize(1000); // 1ms
  Timer1.attachInterrupt(systemTick); // systemTick to run every 1ms
  
  // Serial.println("stage\tms\tcurrent\tpressure(pa)\tflow(ml/s)");
}

volatile int elapsedTime = 0;

volatile int flowReadingMls;
int flowReadingSum = 0;

const int motorUpdateRate = 10; // update motor every 10 ticks
unsigned int motorCount = motorUpdateRate;
volatile int doMotorUpdate = 0;

const int inhaleState = 1;
const int exhaleState = 0;

volatile int ventState = inhaleState;

const int inhaleDuration = 1000; // ticks
const int dwell = 100;
const int exhaleDuration = 800; // ticks

volatile int breathCycle = inhaleDuration;

void systemTick(void)
{
  elapsedTime = elapsedTime + 1;
  flowReadingMls = getFlowRateMls();
  flowReadingSum = flowReadingSum + flowReadingMls;
  
  motorCount = motorCount - 1;
  if (motorCount == 0)
  {
    doMotorUpdate = 1;
    motorCount = motorUpdateRate;
    flowReadingMls = flowReadingSum/10; // average over 10 samples
    flowReadingSum = 0;
  }

  breathCycle = breathCycle - 1;
  if (breathCycle == 0)
  {
    if (ventState == inhaleState)
    {
      ventState = exhaleState;
      breathCycle = exhaleDuration;
    }
    else
    {
      ventState = inhaleState;
      breathCycle = inhaleDuration;
    }
  }
}

void loop()
{
  unsigned long elapsedTimeCopy;
  unsigned int ventStateCopy;
  int flowReadingCopy;

  if (doMotorUpdate == 1)
  {
    noInterrupts();
    flowReadingCopy = flowReadingMls;
    doMotorUpdate = 0;
    ventStateCopy = ventState;
    elapsedTimeCopy = elapsedTime;
    interrupts();
    // Serial.print("Flow ml/s\t");
    // Serial.println(flowReadingCopy);
    // Serial.print("\tvent state\t");
    if(ventStateCopy == inhaleState)
    {
      Serial.print("inhale\t");
      Serial.println(elapsedTimeCopy);
      setMotor1Speed(-200);
    }
    else if(ventStateCopy == exhaleState){
      Serial.print("exhale\t");
      Serial.println(elapsedTimeCopy);
      setMotor1Speed(200);
    }
  }

  // checkUI();
  // lastBreathTime = millis();
  // inhale();
  // postInhalePause();
  // exhale();
  // postExhalePause();
}
