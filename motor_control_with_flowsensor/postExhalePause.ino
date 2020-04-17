void postExhalePause ()
{
  while (millis() - lastBreathTime < breathPeriod)                     // dwell
  {
    setMotor1Speed(0);
    //digitalWrite(9, HIGH);          // apply brake DON'T USE
    checkUI();
    // printCycleInfo("pePause",current);
    Serial.println(getFlowRateMls());
  }
}