//void setup(){
  //Serial.begin(9600);
//}

//void loop(){
  //Serial.println(digitalRead(4));
  //delay(10);
//}



int IR_sensor_pin = 4;  // LED connected to digital pin 13
int LEDpin = 3;    // pushbutton connected to digital pin 7
int val = 0;      // variable to store the read value

void setup() {
  pinMode(IR_sensor_pin, INPUT);  // sets the digital pin 13 as output
  pinMode(LEDpin, OUTPUT);    // sets the digital pin 7 as input
}

void loop() {
  val = digitalRead(IR_sensor_pin);   // read the input pin
  digitalWrite(LEDpin, val);  // sets the LED to the IR's value
}