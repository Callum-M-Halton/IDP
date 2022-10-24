#include <Arduino.h>

void setup() {
  set_motor_speeds(0);
  delay(10000);
  Serial.begin(9600);

  // attach Servo and configure ultrasonic pins
  myservo.attach(SERVO_PIN);
  // Set trig pins as outputs and echo pins as inputs
  pinMode(front_US_pins.trig, OUTPUT);  pinMode(side_US_pins.trig, OUTPUT);
  pinMode(front_US_pins.echo, INPUT);  pinMode(side_US_pins.echo, INPUT);

  // Setting the IR sensor pins as inputs
  pinMode(front_sensor_pins.left, INPUT);
  pinMode(front_sensor_pins.mid, INPUT);
  pinMode(front_sensor_pins.right, INPUT);
  
  // Setting LEDs for errors
  pinMode(ERROR_LED_PIN, OUTPUT);

  // Connecting to motors
  if (!AFMS.begin(/*default frequency 1.6KHz*/)) {
    Serial.println("Could not find Motor Shield. Check wiring.");
    while (1);
  }
  Serial.println("Motor Shield found.");

  // state configuration
  set_sector(5);
  // set_mode(modes.finding_line_at_start);
}