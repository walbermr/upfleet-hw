char dataReceived[1000];
int incomingByte = 0;
int i = 0;
int time1 = 0;

void setup() {
  Serial.begin(115200); // use the same baud-rate as the python side
}
void loop() {
  time1 = millis();
  if(Serial.available() > 0){
    incomingByte = Serial.read();
    dataReceived[i] = incomingByte;
    i++;
    if(incomingByte == '\n'){
      dataReceived[i] = '\0';
      Serial.println(dataReceived);
      i = 0;
      delay(millis()-time1);
    }
  }
}
