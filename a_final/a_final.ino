#include <Adafruit_MotorShield.h>
#include <LinkedList.h>
#include <Servo.h>

unsigned long start_millis;

// Initialise Motors and Servo
Adafruit_MotorShield AFMS = Adafruit_MotorShield();
Adafruit_DCMotor *L_motor = AFMS.getMotor(4);
Adafruit_DCMotor *R_motor = AFMS.getMotor(3);
Servo myservo;
