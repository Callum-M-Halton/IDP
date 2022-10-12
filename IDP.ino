#include <Adafruit_MotorShield.h>

// Create the motor shield object with the default I2C address
Adafruit_MotorShield AFMS = Adafruit_MotorShield();
Adafruit_DCMotor *L_motor = AFMS.getMotor(1);
Adafruit_DCMotor *R_motor = AFMS.getMotor(2);
int sensorPin = A2;   
int sensorValue = 0;

struct speeds_struct {
  int tiny;
  int low;
  int med;
  int high;
};
speeds_struct speeds = {100, 150, 200, 250};


int* get_front_line_sensors() {
  int sensor_vals[3] = {analogRead(A0), analogRead(A1), analogRead(A2)};
  return sensor_vals;
}


/*
void set_motor_speed(is_right, speed) {
	if is_right:
		if motor_speeds[1] != speed:
			set_right(speed)
			motor_speeds[1] == speed
	else:
		// ...
}
*/

void setup() {
  Serial.begin(9600);
}

void loop() {

  Serial.println(get_front_line_sensors()[0]);
  //Serial.print(" ");
  delay(20);
}
