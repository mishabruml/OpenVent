void postInhalePause ()
{
  pauseTimer = millis();
  while (millis() - pauseTimer < postInhaleDwell)                     // dwell
  {
    setMotor1Speed(0);
    //digitalWrite(9, HIGH);          // apply brake DON'T USE
    checkUI();
    // printCycleInfo("piPause",current);
    // Serial.println(getFlowRateMls());
  }
  // Serial.println("post inhale pause");
  // Serial.print("speed\t");
  // Serial.println(mSpeed);
  // Serial.print("current\t");
  // Serial.println(current);
  // Serial.println();
}