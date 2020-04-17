void printCycleInfo(String cycleStageName, int current)
{
  if(millis() % 100 == 0)
  {
    Serial.print(cycleStageName);
    Serial.print("\t");
    Serial.print(millis());
    Serial.print("\t");
    Serial.print(current);
    Serial.print("\t");
    Serial.print(getPressureReadingPa());
    Serial.print("\t");
    Serial.print(getFlowRateMls());
    Serial.println();
  }
}