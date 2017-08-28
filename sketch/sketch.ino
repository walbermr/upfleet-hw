String dataReceived;

void setup() {
  Serial.begin(115200); // use the same baud-rate as the python side
}
void loop() {
  if (Serial.available()) {
    dataReceived = Serial.readString();
    
    Serial.print(dataReceived);
    
  }
}
