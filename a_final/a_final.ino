#include <Adafruit_MotorShield.h>
#include <LinkedList.h>
#include <Servo.h>

unsigned long start_millis;

// Initialise Motors and Servo
Adafruit_MotorShield AFMS = Adafruit_MotorShield();
Adafruit_DCMotor *L_motor = AFMS.getMotor(4);
Adafruit_DCMotor *R_motor = AFMS.getMotor(3);
Servo myservo;

// This has to go in here as precompiler doesn't like it for some reason
struct us_pins_struct {int echo; int trig;};
us_pins_struct front_US_pins = {5, 4}; // for HC-SR04
// us_pins_struct side_US_pins = {8, 9}; // for HC-SR04
int get_ultrasonic_distance() {
  // Clears the TRIG_PIN condition
  digitalWrite(4, LOW);
  delayMicroseconds(2);
  // Sets the TRIG_PIN HIGH (ACTIVE) for 10 microseconds
  digitalWrite(4, HIGH);
  delayMicroseconds(10);
  digitalWrite(4, LOW);
  // Reads the ECHO_PIN, returns the sound wave travel time in microseconds
  long duration = pulseIn(5, HIGH);
  // Calculating the distance
  int distance = duration * 0.034 / 2; // Speed of sound wave divided by 2 (go and back)
  // Displays the distance on the Serial Monitor
  Serial.println(String(distance));
  return distance;
}
