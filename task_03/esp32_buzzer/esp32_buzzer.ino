// We can not use analogWrite easily because its not supported by ESP32...
// So we can't use the "tone" function as well..

const uint8_t BUZZ = 12;

void setup() {
  
  Serial.begin(115200);
  Serial.println("Started.");

  pinMode(BUZZ, OUTPUT);
}

void loop() {

  // active buzzer frequency output
  playTone(100, 1, 1);
  delay(2000);

  // lower pitch
  playTone(100, 5, 2);
  delay(2000);

  // high pitch
  playTone(100, 2, 5);
  delay(2000);

  // lower pitch but louder
  playTone(50, 10, 1);
  delay(2000);

  // high pitch and low volume
  playTone(50, 1, 10);
  delay(2000);

  delay(3000);
  
}

void playTone(int toneLength, int delay1, int delay2) {
  for (int i = 0; i < toneLength; i++) {
    digitalWrite(BUZZ, HIGH);
    delay(delay1);
    digitalWrite(BUZZ, LOW);
    delay(delay2);
  }
}
