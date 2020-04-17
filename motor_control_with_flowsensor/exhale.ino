void exhale ()
{
  unsigned long revTimer = millis();   // log time in ms
  //while (current > 1000 && millis() - revTime < revTimeSetting)     // reversing based on current was not repeatable

  accelTime = millis();               // log time in ms for acceloration
  int revAccelSpeed = 0;                     // clear motor speed variable
  while (millis() - revTimer < revTimeSetting)           // reverse back for a set time
  {
    if (revAccelSpeed < revSpeed)            // acceloration
    {
      if (millis() - accelTime > 1)     // accelorate the motor by 2 every 3ms
      {
        revAccelSpeed++;                 // - is forward direction
        accelTime = millis();
      }
    }
    setMotor1Speed(revAccelSpeed);
    checkUI();
    // printCycleInfo("exhale",current);
    Serial.println(getFlowRateMls());
  }
}