void setup() {

  Serial.begin(115200);
  Serial.println("Starting");

  delay(50);

  Serial.println("ToDo: Create Server and stuff...");
}

void loop() {

  int val = 0;
  val = hallRead();

  Serial.print("Hall sensor value: ");
  Serial.println(val);

  delay(1000);
}
