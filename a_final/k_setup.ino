#include <Arduino.h>

void setup() {
  Serial.begin(9600);
  // Connecting to motors
  if (!AFMS.begin(/*default frequency 1.6KHz*/)) {
    Serial.println("Could not find Motor Shield. Check wiring.");
    while (1);
  }
  Serial.println("Motor Shield found.");
  // Make sure we don't start moving prematurely
  set_motor_speeds(0);

  // attach Servo and configure ultrasonic pins
  myservo.attach(SERVO_PIN);
  // Set trig pins as outputs and echo pins as inputs
  pinMode(front_US_pins.trig, OUTPUT);  pinMode(side_US_pins.trig, OUTPUT);
  pinMode(front_US_pins.echo, INPUT);  pinMode(side_US_pins.echo, INPUT);

  // Setting the IR sensor pins as inputs
  pinMode(front_sensor_pins.left, INPUT);
  pinMode(front_sensor_pins.mid, INPUT);
  pinMode(front_sensor_pins.right, INPUT);
  
  // Setting LEDs
  pinMode(ERROR_LED_PIN, OUTPUT);
  pinMode(AMBER_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(HALL_SENSOR_PIN, INPUT);

  if (false) {
    turn_on_spot(false);
    delay(500);
    set_motor_dirs(FORWARD);
    set_motor_speeds(255);
    delay(1000);
    turn_on_spot(true);
    delay(500);
    set_motor_dirs(FORWARD);
    set_motor_speeds(255);
    delay(1000);
  }

  leave_start();
}
