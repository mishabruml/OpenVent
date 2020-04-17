void inhale ()
{
  accelTime = millis();               // log time in ms
  mSpeed = 0;                     // clear motor speed variable
  setMotor1Speed(mSpeed);         // drive motor
  while (current < currentLimit)          // drive until current setting
  {
    if (mSpeed > maxFwdSpeed)            // acceloration
    {
      if (millis() - accelTime > 3)     // accelorate the motor by 1 every 3ms
      {
        mSpeed--;                 // - is forward direction
        accelTime = millis();
      }
    }
    setMotor1Speed(mSpeed);
    checkUI();
    printCycleInfo("inhale",current);
  }
}