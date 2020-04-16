/*
    OpenVent-Bristol geared DC motor BVM ventilator V2 Arduino software
    http://darrenlewismechatronics.co.uk/
*/

#include <LiquidCrystal.h>
#include "ArduinoMotorShieldR3.h"
ArduinoMotorShieldR3 md;

#define DIR_A 12
#define PWM_A 3

// select the pins used on the Velleman LCD button Arduino shield
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
#define buttonsLCDpin A3        // Analogue input for buttons on LCD shield
#define btnRIGHT 0
#define btnUP 1
#define btnDOWN 2
#define btnLEFT 3
#define btnSELECT 4
#define btnNONE 5

// for selecting ventilation modes
#define ventMode 0
#define BIPAP 1

#define pressureSensorPin A2          // A0 & A1 are motor current sense pins
#define currentLimit 1000;            // current limit for sensing end stop YOU MAY NEED TO CHANGE THIS IF USING A DIFFERENT MOTOR

#define maxPressure 35                // set in NHS spec
#define minPressure 10                // 10 is set higher than the lowest PEEP for failsafe

unsigned long lastBreathTime = 0;     // Sorry Sam

void setup()
{
  //md.init();
  // set up motor pins
  pinMode(DIR_A, OUTPUT);
  pinMode(PWM_A, OUTPUT);

  Serial.begin(115200);

  // set up LCD library
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("cmH2O I:E BPM Mode");    // headings to LCD
  lcd.setCursor(6, 1);
  lcd.print("1:");                    // print the static part of the I:E ratio
}


void loop()
{
  // ********* UI stuff **********
  static int cursorPos;               // position variable of select cursor
  static unsigned long buttonTimer;   // lof time for how long the button has been pressed for for debounce
  static int buttonState;
  static int lastButtonState;
  const int debounceTime = 150;       // time period for debouncing the buttons
  static int screenLatPos;
  static int screenLatPosTarget;

  // location of numbers on LCD display below their corrisponding headings
  const int cmH2O_cell = 0;
  const int IE_cell = 8;
  const int BPM_cell = 10;
  const int Mode_cell = 14;

  // measured readings variables
  static int target_cmH2O;
  static int target_IE;
  static int target_BPM;
  static bool target_Mode;

  int lcd_key = read_LCD_buttons(); // read the buttons
  if (lastButtonState != lcd_key) buttonTimer = millis();    // reset the debouncing timer if state change
  if (millis() - buttonTimer > debounceTime)                 // if button pressed for longer than delay
  {
    if (lcd_key != buttonState) buttonState = lcd_key;       // if there is a new button reading
    switch (buttonState)                                     //
    {
      case btnRIGHT:        // change cursor position
        {
          screenLatPosTarget++;
          if (cursorPos == cmH2O_cell) cursorPos = IE_cell;
          else if (cursorPos == IE_cell) cursorPos = BPM_cell;
          else if (cursorPos == BPM_cell) cursorPos = Mode_cell;
          cursorPos = constrain(cursorPos, 0, 14);
          lcd.setCursor(cursorPos, 1);         // format (cell, line)
          break;
        }
      case btnLEFT:        // change cursor position
        {
          screenLatPosTarget--;
          if (cursorPos == Mode_cell) cursorPos = BPM_cell;
          else if (cursorPos == BPM_cell) cursorPos = IE_cell;
          else if (cursorPos == IE_cell) cursorPos = cmH2O_cell;
          cursorPos = constrain(cursorPos, 0, 14);
          lcd.setCursor(cursorPos, 1);         // format (cell, line)
          break;
        }
      case btnUP:        // change target value
        {
          if (cursorPos == cmH2O_cell) target_cmH2O++;
          else if (cursorPos == IE_cell) target_IE++;
          else if (cursorPos == BPM_cell) target_BPM = target_BPM + 2;     // NHS spec asked for increments of 2
          else if (cursorPos == Mode_cell) target_Mode = ventMode;
          break;
        }
      case btnDOWN:        // change target value
        {
          if (cursorPos == cmH2O_cell) target_cmH2O--;
          else if (cursorPos == IE_cell) target_IE--;
          else if (cursorPos == BPM_cell) target_BPM = target_BPM - 2;     // NHS spec asked for increments of 2
          else if (cursorPos == Mode_cell) target_Mode = BIPAP;
          break;
        }
    }
    buttonTimer = millis();       // to make sure it doesn't register multiple presses from one finger press

    // constrain targets
    target_cmH2O = constrain(target_cmH2O, minPressure, maxPressure);
    target_IE = constrain(target_IE, 1, 3);         // as of NHS spec
    target_BPM = constrain(target_BPM, 10, 30);     // as of NHS spec
    target_Mode = constrain(target_Mode, 0, 1);

    // write targets to LCD
    lcd.setCursor(cmH2O_cell, 1);    // move cursor below cmH2O
    lcd.print(target_cmH2O);
    lcd.setCursor(IE_cell, 1);    // move cursor below I:E
    lcd.print(target_IE);
    lcd.setCursor(BPM_cell, 1);    // move cursor below BPM
    lcd.print(target_BPM);
    lcd.setCursor(Mode_cell, 1);    // move cursor below Mode
    if (target_Mode == ventMode) lcd.print("Vent ");
    if (target_Mode == BIPAP) lcd.print("BIPAP");
  }
  lastButtonState = lcd_key;
  lcd.setCursor(cursorPos, 1);
  lcd.blink();                    // blink in current cusor position

  if (screenLatPos != screenLatPosTarget)     // shift characters along if needed
  {
    screenLatPosTarget = constrain(screenLatPosTarget, 0, 3);
    if (screenLatPosTarget > screenLatPos) lcd.scrollDisplayLeft();
    if (screenLatPosTarget < screenLatPos) lcd.scrollDisplayRight();
    screenLatPos = screenLatPosTarget;
  }


  // ***************** motor driving ****************
  unsigned long breathPeriod = (60 / target_BPM) * 1000;      // calc time needed per breath in ms based on UI setting
  unsigned long inhalePeriod = breathPeriod / (target_IE + 1);  // calc period for inhale. breathPeriod/total number of IE units
  int revTime = map(target_cmH2O, minPressure, maxPressure, 220, 350);      // set reversing time period
  revTime = constrain(revTime, 220, 350);         // constrain reverse time limits
  static int oldSpeed;                            // used to pass previous speed value tp motor speed functions
  static bool firstRunExhale = 1;
  static unsigned long revTimer;

  // inhale period according to breath frequency & I:E ratio
  Serial.print(millis());
  Serial.print(" , ");
  Serial.println(pressureSensorReading());
  if (millis() - lastBreathTime < inhalePeriod)               // inhale
  {
    static int inhaleSpeedVal = 0;
    oldSpeed = inhaleSpeedVal;                                // record old motor speed
    inhaleSpeedVal = inhaleSpeed(oldSpeed, target_cmH2O);     // calc new motor forewards speed
    setMotor1Speed(inhaleSpeedVal);
    //Serial.print("inhaleSpeedVal ");
    //Serial.println(inhaleSpeedVal);

  }
  // if inhale time is up
  else if (millis() - lastBreathTime < breathPeriod)          // exhale
  {
    if (firstRunExhale == 1)
    {
      revTimer = millis();
      firstRunExhale = 0;
      Serial.println("firstRunExhale");
    }
    //unsigned long exhalePeriod = (breathPeriod / (target_IE + 1)) * IEval;   // calc period for exhale, not needed
    // motor reverse for a set time period
    if (millis() - revTimer < revTime)
    {
      static int exhaleSpeedVal = 0;
      oldSpeed = exhaleSpeedVal;                              // record old motor speed
      exhaleSpeedVal = exhaleSpeed(oldSpeed);                 // calc new motor reverse speed
      setMotor1Speed(exhaleSpeedVal);                                      // motor reverse
      //Serial.print("exhaleSpeedVal ");
      //Serial.println(exhaleSpeedVal);
    }
    // pause motor until breathPeriod is up
    else
    {
      setMotor1Speed(0);              // stop motor
      //Serial.println("paused");
    }
  }
  else if (millis() >= breathPeriod)
  {
    lastBreathTime = millis();              // reset & record time at begining of breath
    firstRunExhale = 1;                     // reset condition
  }
  // **************** start new breath ***************
}


int inhaleSpeed(int currentSpeedI, int targetPressure)
{
  int newPressureVal = pressureSensorReading();
  currentSpeedI = constrain(currentSpeedI, -400, 400);                              // cap readings for safety
  int pressureDifference = targetPressure - newPressureVal;        // the difference in pressure that needs making up
  int maxFwdSpeed = map(pressureDifference, 0, 20, -90, -400); // set motor speed proportionally to pressure difference
  maxFwdSpeed = constrain(maxFwdSpeed, -400, 0);
  if (newPressureVal > targetPressure) maxFwdSpeed = 0;

  static unsigned long fwdAccelTimer = millis();                            // only declare once
  if (currentSpeedI > maxFwdSpeed)                                           // acceloration
  {
    if (millis() - fwdAccelTimer > 1)                                       // accelorate the motor by 1 every Xms
    {
      currentSpeedI--;                                                       // - is forward direction
      fwdAccelTimer = millis();                                             // log time in ms
    }
  }
  else if (currentSpeedI <= maxFwdSpeed)                      // acceloration
  {
    if (millis() - fwdAccelTimer > 1)                   // accelorate the motor by 1 every 3ms
    {
      currentSpeedI++;                                  // - is forward direction
      fwdAccelTimer = millis();                         // log time in ms
    }
  }
  return currentSpeedI;
}


int exhaleSpeed(int currentSpeedE)
{
  currentSpeedE = constrain(currentSpeedE, -400, 400);  // cap readings for safety

  static const int maxRevSpeed = 400;                         // set max exhale motor speed
  static unsigned long revAccelTimer = micros();        // only called once
  if (currentSpeedE < maxRevSpeed)                      // acceloration
  {
    if (micros() - revAccelTimer > 100)                 // accelorate the motor
    {
      currentSpeedE = currentSpeedE + 10;                                  // - is forward direction
      revAccelTimer = micros();                         // log time in ms
    }
  }
  return currentSpeedE;
}


// ****** drive motors ******
void setMotor1Speed(int M1speed)
{
  if (M1speed < 0) {
    M1speed = -M1speed;                         // Make speed a positive quantity
    digitalWrite(DIR_A, LOW);                   // direction pin setting
  }
  else {
    digitalWrite(DIR_A, HIGH);                  // direction pin setting
  }
  if (M1speed > 400) M1speed = 400;             // Max PWM dutycycle
  analogWrite(PWM_A, M1speed * 51 / 80);        // default to using analogWrite, mapping 400 to 255
}


float pressureSensorReading()
{
  static const float ADC_mV = 4.8828125;       // convesion multiplier from Arduino ADC value to voltage in mV
  static const float SensorOffset = 200.0;     // in mV taken from datasheet
  static const float sensitivity = 4.413;      // in mV/mmH2O taken from datasheet
  static const float mmh2O_cmH2O = 10;         // divide by this figure to convert mmH2O to cmH2O
  static const float mmh2O_kpa = 0.00981;      // convesion multiplier from mmH2O to kPa

  float sensorValue = (analogRead(pressureSensorPin) * ADC_mV - SensorOffset) / sensitivity / mmh2O_cmH2O;    // result in cmH2O
  int intSensorValue = sensorValue;            // convert to integer
  return intSensorValue;
}


// read the buttons
int read_LCD_buttons()      // debounce these
{
  int adc_key_in = analogRead(buttonsLCDpin);             // read the value from the sensor
  // buttons are centered at these values: 0, 102, 258, 412, 641 (example code vals: 0, 144, 329, 504, 741)
  if (adc_key_in > 1000) return btnNONE; // We make this the 1st option for speed reasons since it will be  the most likely result
  // For V1.1 us this threshold
  if (adc_key_in < 50) return btnRIGHT;
  if (adc_key_in < 175) return btnUP;
  if (adc_key_in < 350) return btnDOWN;
  if (adc_key_in < 525) return btnLEFT;
  if (adc_key_in < 900) return btnSELECT;
  return btnNONE; // when all others fail, return this

  // IMPORTANT: Calibration routine needed to account for resistors out of tollerance
}
