int sensorPin = A0;   
int sensorValue = 0;
int avg = 0;
const int sample_length = 200;
int vals[sample_length];
int i = 0;

void setup() {
  pinMode(sensorPin, INPUT);
  Serial.begin(9600);
}

void loop() {
  sensorValue = analogRead(sensorPin);
 
  if (i < sample_length) {
    vals[i] = sensorValue;
    i++;
  } else {
    int tot = sensorValue;
    for (int j=1; j < sample_length; j++) {
      tot += vals[j];
      vals[j-1] = vals[j];
    }
    vals[sample_length - 1] = sensorValue;
    avg = tot / sample_length;
  }

  Serial.println(avg);
  delay(20);
}
