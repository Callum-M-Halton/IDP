// Sensor data receive on digital pin 0
int sensorPin = 0;   
int sensorValue = 0;
int avg = 0;
// number of samples for the rolling avg
const int sample_length = 200;
// create array to store the last sample_length values for calculating our rolling avg
int vals[sample_length];
// i will count up each loop until sample_length is reached at which point avg can begin being calculated
int i = 0;

void setup() {
  pinMode(sensorPin, INPUT);
  Serial.begin(9600);
}

void loop() {
  // Read sensor value
  sensorValue = analogRead(sensorPin);

  // if complete sample not produced yet
  if (i < sample_length) {
    // add to end of sample 'list'
    vals[i] = sensorValue;
    i++;
  } else {
    // new sensor value will form part of new avg
    int tot = sensorValue;
    // from second to final element
    for (int j=1; j < sample_length; j++) {
      // add sample value to total and shift element back one index
      tot += vals[j];
      vals[j-1] = vals[j];
    }
    // add new sample to newly free spot at end of sample array and find avg
    vals[sample_length - 1] = sensorValue;
    avg = tot / sample_length;
  }

  // output avg
  Serial.println(avg);
  delay(20);
}
