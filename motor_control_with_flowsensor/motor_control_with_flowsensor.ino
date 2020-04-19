#include <Wire.h>
#include <sfm3000wedo.h>
#include "ArduinoMotorShieldR3.h"
#include <TimerOne.h>

ArduinoMotorShieldR3 md;

// motor pins
int DIR_A = 12;
int PWM_A = 3;
int current = 0;     // log filtered current reading

int currentLimit = 500;        // current limit

volatile int elapsedTime = 0;
volatile int flowReadingMls;
int flowReadingSum = 0;

const int motorUpdateRate = 10; // update motor every 10 ticks
unsigned int motorCount = motorUpdateRate;
volatile int doMotorUpdate = 0;

const int inhaleState = 0;
const int postInhaleDwellState = 1;
const int exhaleState = 2;
const int postExhaleDwellState = 3;

volatile int ventState = inhaleState;

const int inhaleDuration = 1000; // ticks
const int exhaleDuration = 800; // ticks
const int dwell = 100; //ticks

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
      ventState = postInhaleDwellState;
      breathCycle = dwell;
    }
    else if (ventState == postInhaleDwellState)
    {
      ventState = exhaleState;
      breathCycle = exhaleDuration;
    }
    else if (ventState == exhaleState)
    {
      ventState = postExhaleDwellState;
      breathCycle = dwell;
    }
    else if (ventState == postExhaleDwellState)
    {
      ventState = inhaleState;
      breathCycle = inhaleDuration;
    }
  }
}

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
  delay(3000);

  Timer1.initialize(1000); // 1ms
  Timer1.attachInterrupt(systemTick); // systemTick to run every 1ms  
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
    Serial.println(flowReadingCopy);
    // Serial.print("\tvent state\t");
    if(ventStateCopy == inhaleState)
    {
      setMotor1Speed(-150);
    }
    else if(ventStateCopy == postInhaleDwellState)
    {
      setMotor1Speed(0);
    }
    else if(ventStateCopy == exhaleState){
      setMotor1Speed(150);
    }
    else if(ventStateCopy == postExhaleDwellState){
      setMotor1Speed(0);
    }
  }

  // checkUI();
  // lastBreathTime = millis();
  // inhale();
  // postInhalePause();
  // exhale();
  // postExhalePause();
}
