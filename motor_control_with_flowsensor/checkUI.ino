// ***** check buttons & POTs and flash LEDs *****
void checkUI()          // check buttons and POTs
{
  current = getCurrentM1();
  a1 = analogRead(A1);
  if (analogRead(A2) > maxPressurePOT + 1 || analogRead(A2) < maxPressurePOT - 1) // if values are not equal within a tollerance
  {
    maxPressurePOT = analogRead(A2);                              // read POT
    if (maxPressurePOT > 950 || maxPressurePOT < 50) redLEDonTime = millis();
    else POTblueLEDonTime = millis();                                // flash LEDs for feedback
    maxPressurePOTconstrianed = constrain(maxPressurePOT, 50, 950);          // constrain POT values to account for tollerance
  }

  currentLimit = map(maxPressurePOTconstrianed, 50, 950, 800, 1200);       // set current limit
  currentLimit = constrain(currentLimit, 800, 1200);            // constrain current limits
  revTimeSetting = map(currentLimit, 800, 1200, 650, 850);      // set reversing time period
  revTimeSetting = constrain(revTimeSetting, 650, 850);         // constrain reverse time limits

  // frequency up button reading
  int reading2 = digitalRead(freqUpButtonPin);
  // debounce
  if (reading2 != lastButtonState2) lastDebounceTime2 = millis();    // reset the debouncing timer if state change
  if ((millis() - lastDebounceTime2) > debounceDelay) {              // if button pressed for longer than delay
    if (reading2 != buttonState2)
    {
      buttonState2 = reading2;
      if (buttonState2 == LOW)
      {
        bpm++;         // add 1 to breath per min setting
        if (bpm <= 30) blueLEDonTime = millis();
        else redLEDonTime = millis();
      }
    }
  }
  lastButtonState2 = reading2;

  // frequency down button reading
  int reading6 = digitalRead(freqDownButtonPin);
  // debounce
  if (reading6 != lastButtonState6) lastDebounceTime6 = millis();    // reset the debouncing timer if state change
  if ((millis() - lastDebounceTime6) > debounceDelay) {              // if button pressed for longer than delay
    if (reading6 != buttonState6)
    {
      buttonState6 = reading6;
      if (buttonState6 == LOW)
      {
        bpm--;         // take away 1 from breath per min setting
        if (bpm >= 10) blueLEDonTime = millis();
        else redLEDonTime = millis();
      }
    }
  }
  lastButtonState6 = reading6;

  bpm = constrain(bpm, 10, 30);       // constrain variable to 10-30 b/m
  breathPeriod = (60 / bpm) * 1000;                  // calc time needed per breath in ms based on UI setting
  // Serial.println(bpm);

  if (millis() - blueLEDonTime < 200) digitalWrite(blueLEDPin, HIGH);   // flash blue LED for button press feedback
  else digitalWrite(blueLEDPin, LOW);
  if (millis() - redLEDonTime < 500) digitalWrite(redLEDPin, HIGH);     // flash red LED if b/m limit reached
  else digitalWrite(redLEDPin, LOW);
  if (millis() - POTblueLEDonTime < 10) digitalWrite(blueLEDPin, HIGH);   // flash blue LED for button press feedback
  else digitalWrite(blueLEDPin, LOW);
}