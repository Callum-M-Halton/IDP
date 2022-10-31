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

  // attach Servo and configure ultrasonic pins
  myservo.attach(SERVO_PIN);
  // Set trig pins as outputs and echo pins as inputs for ultrasonics
  pinMode(FRONT_TRIG_PIN, OUTPUT); pinMode(SIDE_TRIG_PIN, OUTPUT);
  pinMode(FRONT_ECHO_PIN, INPUT); pinMode(SIDE_ECHO_PIN, INPUT);
  // Setup hall sensor
  pinMode(HALL_SENSOR_PIN, OUTPUT);

  // Setting the IR sensor pins as inputs
  pinMode(front_sensor_pins.left, INPUT);
  pinMode(front_sensor_pins.mid, INPUT);
  pinMode(front_sensor_pins.right, INPUT);
  pinMode(JUNCT_SENSOR_PIN, INPUT);
  
  // Setting LEDs
  pinMode(AMBER_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
 
  // hardware config
  set_motor_speeds(0); /////////
  raise_grabber();
  my_delay(10000);

  //state.approaching = approachables.block;
  leave_start();
}

  /*if (false) {
    turn_on_spot(false);
    my_delay(500);
    set_motor_dirs(FORWARD);
    set_motor_speeds(255);
    my_delay(1000);
    turn_on_spot(true);
    my_delay(500);
    set_motor_dirs(FORWARD);
    set_motor_speeds(255);
    my_delay(1000);
  }*/

/*
pinMode(RED_LED_PIN, OUTPUT);
pinMode(GREEN_LED_PIN, OUTPUT);
pinMode(HALL_SENSOR_PIN, INPUT);
pinMode(ERROR_LED_PIN, OUTPUT);
*/
