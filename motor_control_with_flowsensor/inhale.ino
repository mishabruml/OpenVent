void inhale ()
{
  unsigned long inhaleStartTime = micros();
  unsigned long inhaleVolumeDelivered = 0;
  unsigned long elapsedInhaleTime = 0;
  
  unsigned long samplingPeriod = 1000; // microseconds

  accelTime = millis();               // log time in ms
  mSpeed = 0;                     // clear motor speed variable
  
  setMotor1Speed(mSpeed);         // drive motor
  
  while(elapsedInhaleTime < 3000000)
  // while (current < currentLimit)          // drive until current setting
  {
    if (mSpeed > maxFwdSpeed)            // acceloration
    {
      if (millis() - accelTime > 3)     // accelorate the motor by 1 every 3ms
      {
        mSpeed--;                 // - is forward direction
        accelTime = millis();
      }
    }

    // inhaleVolumeDelivered = getFlowRateMls() * ( ( millis() - inhaleStartTime ) / 1000);
    // Serial.print('volume delivered\t');
    // Serial.println(inhaleVolumeDelivered);

    // Serial.print("flow rate\t");
    // Serial.print(getFlowRateMls());
    // Serial.print("elapsed\t");
    elapsedInhaleTime += micros() - inhaleStartTime;

    // if ((micros() - inhaleStartTime) % samplingPeriod == 0) { 
      Serial.println(elapsedInhaleTime); 
    // }

    // setMotor1Speed(mSpeed);
    // checkUI();

    // printCycleInfo("inhale",current);
    // Serial.println(getFlowRateMls());
  }
  // unsigned long inhaleDuration = millis() - inhaleStartTime;
  // Serial.println("inhale");
  // Serial.print("speed\t");
  // Serial.println(mSpeed);
  // Serial.print("current\t");
  // Serial.println(current);
  // Serial.print("duration\t");
  // Serial.println(inhaleDuration);
  // Serial.println();
}