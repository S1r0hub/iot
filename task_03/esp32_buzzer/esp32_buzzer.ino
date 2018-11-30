const uint8_t BUZZ = 12;

void setup() {
  
  Serial.begin(115200);
  Serial.println("Started.");

  pinMode(BUZZ, OUTPUT);
}

void loop() {

  // active buzzer frequency output
  for (int i = 0; i < 80; i++) {
    digitalWrite(BUZZ, HIGH);
    delay(1);
    digitalWrite(BUZZ, LOW);
    delay(1);
  }

  for (int i = 0; i < 100; i++) {
    digitalWrite(BUZZ, HIGH);
    delay(2);
    digitalWrite(BUZZ, LOW);
    delay(2);
  }
  
}
