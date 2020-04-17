void printCycleInfo(String cycleStageName, int current)
{
  if(millis() % 100 == 0)
  {
    Serial.print(cycleStageName);
    Serial.print("\t\t");
    Serial.print(millis());
    Serial.print("\t\t");
    Serial.print(current);
    Serial.println();
  }
}