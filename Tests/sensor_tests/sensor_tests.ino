int sensorPin = A0;   
int sensorValue = 0;
int sample_length = 100;
int vals[sample_length];
int i = 0;

void setup() {
  pinMode(sensorPin, INPUT);
  Serial.begin(9600);
}

void loop() {
  sensorValue = analogRead(sensorPin);
  if (i < sample_length) {
    vals[i] = sensorValue
    i++
  } else {
    for (j=0; j < sample_length-1, j++) {
      
    }
    vals[sample_length - 1] = sensorValue
  }
  vals[i] = sensorValue
  Serial.println(sensorValue);
  delay(20);
}
