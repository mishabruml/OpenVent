// Returns pressure reading in Pa
float getPressureReadingPa() 
{
  const float SensorOffsetGP = 43;     // in mV taken from datasheet
  const float ADC_mV = 4.8828125;       // convesion multiplier from Arduino ADC value to voltage in mV
  const float sensitivity = 4.413;      // in mV/mmH2O taken from datasheet
  const float mmh2O_pa = 9.80665;      // convesion multiplier from mmH2O to Pa

  float pressureRaw = analogRead(A5); // raw value from pressure sensor pin
  float pressureAdjustedPa = (((pressureRaw - SensorOffsetGP) * ADC_mV) / sensitivity * mmh2O_pa);
  return pressureAdjustedPa;
}