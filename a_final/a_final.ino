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
struct US_pins_struct {int echo; int trig;};
int get_ultrasonic_distance(US_pins_struct US_pins) {
  // Clears the TRIG_PIN condition
  digitalWrite(US_pins.trig, LOW);
  delayMicroseconds(2);
  // Sets the TRIG_PIN HIGH (ACTIVE) for 10 microseconds
  digitalWrite(US_pins.trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(US_pins.trig, LOW);
  // Reads the ECHO_PIN, returns the sound wave travel time in microseconds
  long duration = pulseIn(US_pins.echo, HIGH);
  // Calculating the distance
  int distance = duration * 0.034 / 2; // Speed of sound wave divided by 2 (go and back)
  // Displays the distance on the Serial Monitor
  Serial.println("Distance in cm: " + String(distance));
  return distance;
}


// ====================== JUNK ======================
/*
// ORDER MATTERS!!!!!
struct on_loop_sector_codes_struct {
  // int straight_after_start_junct;
  // int straight_afer_red_junct;
  // int corner_before_ramp;
  // int ramp_straight;
  // int corner_after_ramp;
  // int straight_before_cross;
  // int straight_after_cross;
  // int corner_before_tunnel;
  // int section_before_tunnel;
  // int tunnel;
  // int section_after_tunnel;
  // int corner_after_tunnel;
  // int straight_before_green_junct;
  // int straight_after_green_junct;
};
on_loop_sector_codes_struct OL_SCs = {0,1,2,3,4,5,6,7,8,9,10,11,12,13}; */

/*
  pinMode(LEDpin_1, OUTPUT);
  pinMode(LEDpin_2, OUTPUT);
  pinMode(LEDpin_3, OUTPUT);
  //pinMode(LEDpin_mag, OUTPUT);
  //pinMode(LEDpin_nonmag, OUTPUT);*/

/*
unsigned long time_now = 0;
// to turn 90 degrees in one way for either leaving box or when object is located

*/
