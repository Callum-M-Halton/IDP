const byte red_pin = 6;
const byte green_pin = 5;
bool magnetic = 0;
int hallsensor_threshold = 161;
const int HallSensorPin = A1;
int sensorValue = 0;
int avg = 0;
const int sample_length = 5;
int vals[sample_length];
int i = 0;

void setup() {
  pinMode(red_pin, OUTPUT);
  pinMode(green_pin, OUTPUT);
  pinMode(HallSensorPin, INPUT);
  Serial.begin(9600);
}

void loop() {
    sensorValue = analogRead(HallSensorPin);
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
  
  
  
  
  
  if(avg > hallsensor_threshold){
      //box is magnetic
      magnetic = 1; 
              
      //turn on red light for 5 sec if magnetic
      digitalWrite(red_pin, HIGH);
      digitalWrite(green_pin, LOW);
      //delay(5000);
      //digitalWrite(red_pin, LOW);
  } else {
      //box is not magnetic
      magnetic = 0;

      //turn on green light for 5 sec if not magnetic
      digitalWrite(red_pin, LOW);
      digitalWrite(green_pin, HIGH);
      //delay(5000);
      //digitalWrite(green_pin, LOW);
  }
  //Serial.println(magnetic);
  Serial.println(avg);
  //return (magnetic);
  
  
  
  delay(20);
}
