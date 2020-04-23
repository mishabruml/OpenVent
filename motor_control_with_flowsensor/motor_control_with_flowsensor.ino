#include <Wire.h>
#include <sfm3000wedo.h>
#include <ArduinoMotorShieldR3.h>
#include <TimerOne.h>
#include <PID_v1.h>

ArduinoMotorShieldR3 md;

// motor pins
int DIR_A = 12;
int PWM_A = 3;
int current = 0;     // log filtered current reading
double pidSetpoint, pidInput, pidOutput;

const int currentLimit = 500;        // current limit
const int motorUpdateRate = 10; // update motor every 10 ticks

const unsigned int inhaleState = 0;
const unsigned int exhaleState = 1;
const unsigned int inhaleDuration = 2000; // ticks
const unsigned int exhaleDuration = 1800; // ticks
const unsigned int motorSpeedDelta = 8; // for ramping motor speed
const int maxMotorSpeed = 400;

volatile int flowReadingMls;
volatile int doMotorUpdate = 0;
volatile int ventState = inhaleState;
volatile int breathCycle = inhaleDuration;

unsigned int flowReadingSum = 0;
unsigned int motorCount = motorUpdateRate;


void systemTick(void)
{
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
    else if (ventState == exhaleState)
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

  PID pidController(&pidInput, &pidOutput, &pidSetpoint,2,5,1, DIRECT);

  Serial.begin(115200);
  // Serial.println("OpenVent Bristol");
  
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

unsigned int previousVentState = inhaleState;
int motorSpeed = 0;
double volume = 0;

void loop()
{
  unsigned int ventStateCopy;
  unsigned int flowReadingCopy;

  if (doMotorUpdate == 1)
  {
    noInterrupts();
    flowReadingCopy = flowReadingMls;
    doMotorUpdate = 0;
    ventStateCopy = ventState;
    interrupts();

    if(previousVentState != ventStateCopy){
      previousVentState = ventStateCopy;
      motorSpeed = 0; // set to 0 at state change
      volume = 0; //set to 0 at stage change
    }

    volume = volume + (flowReadingCopy * 10 / 1000);
    // Serial.print("flow ml/s\t");
    Serial.print(flowReadingCopy);
    Serial.print("\t");
    Serial.println(volume);

    if(ventStateCopy == inhaleState)
    {
      if(motorSpeed > -maxMotorSpeed){
        motorSpeed = motorSpeed - motorSpeedDelta;
      }
      setMotor1Speed(motorSpeed);
    }
    else if(ventStateCopy == exhaleState){
      if(motorSpeed < maxMotorSpeed){
        motorSpeed = motorSpeed + motorSpeedDelta;
      }
      setMotor1Speed(motorSpeed);
    }

    // Serial.print("\tmotor speed\t");
    // Serial.println(motorSpeed);
  }
}
