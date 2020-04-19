// ****** drive motors ******
void setMotor1Speed(int M1speed)
{
  if (M1speed < 0) {
    M1speed = -M1speed;  // Make speed a positive quantity
    digitalWrite(DIR_A, LOW);
  }
  else {
    digitalWrite(DIR_A, HIGH);
  }
  if (M1speed > 400) M1speed = 400;  // Max PWM dutycycle
  analogWrite(PWM_A, M1speed * 51 / 80); // default to using analogWrite, mapping 400 to 255
}