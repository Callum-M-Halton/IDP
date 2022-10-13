#include <Adafruit_MotorShield.h>

// Create the motor shield object with the default I2C address
Adafruit_MotorShield AFMS = Adafruit_MotorShield();
Adafruit_DCMotor *L_motor = AFMS.getMotor(1);
Adafruit_DCMotor *R_motor = AFMS.getMotor(2);
int sensorPin = A2;   
int sensorValue = 0;

struct speeds_struct {int tiny; int low; int med; int high;};
speeds_struct speeds = {100, 150, 200, 250};

struct front_sensor_pins_struct {int left; int mid; int right;};
front_sensor_pins_struct front_sensor_pins  = {A0, A1, A2};

struct offset_dirs_struct {int none; int left; int right; int unknown;};
offset_dirs_struct offset_dirs = {0, 1, 2, 3};

struct offset_exts_struct {int none; int little; int mid; int far;};
offset_exts_struct offset_exts = {0, 1, 2, 3};

struct state_struct {
  int motor_speeds[2]; offset_dirs_struct offset_dir;
  offset_exts_struct offset_ext;
};
state_struct state = {
  [0, 0], offset_dirs.none,
  offset_exts.none
}

void set_L_motor_speed(speed) {
		if (motor_speeds[0] != speed) {
      motor_speeds[0] = speed
      L_motor -> setSpeed(speed)
}
void set_R_motor_speed(speed) {
		if (motor_speeds[1] != speed) {
      motor_speeds[1] = speed
      R_motor -> setSpeed(speed)
}




void correct_trajectory() {
  int[3] sensors = {digitalRead(front_sensor_pins.left),
    digitalRead(front_sensor_pins.mid), digitalRead(front_sensor_pins.right)};

  if (sensors[0]) {
    if (sensors[1]) {
      if (sensors[2]) {
        // case [1, 1, 1]
        digitalWrite(LEDpin_error, 1);
      } else {
        // case [1, 1, 0]
        state.offset_dir = offset_dirs.right;
        state.offset_ext = offset_exts.little;
			  set_L_motor_speed(speeds.med);
      }
    } else {
      if (sensors[2]) {
        // case [1, 0, 1]
        digitalWrite(LEDpin_error, 1);
      } else {
        // case [1, 0, 0]
        state.offset_dir = offset_dirs.right;
        state.offset_ext = offset_exts.mid;
			  set_L_motor_speed(speeds.low);
      }
    }
  } else {
    if (sensors[1]) {
      if (sensors[2]) {
        // case [0, 1, 1]
        state.offset_dir = offset_dirs.left;
        state.offset_ext = offset_exts.little;
			  set_R_motor_speed(speeds.med);
      } else {
        // case [0, 1, 0]
        state.offset_dir = offset_dirs.none;
        state.offset_ext = offset_exts.none;
			  set_L_motor_speed(speeds.high);
        set_R_motor_speed(speeds.high);
      }
    } else {
      if (sensors[2]) {
        // case [0, 0, 1]
        state.offset_dir = offset_dirs.left;
        state.offset_ext = offset_exts.mid;
			  set_R_motor_speed(speeds.low);
      } else {
        // case [0, 0, 0]
        if (state.offset_dir == offset_dirs.left) {
					set_R_motor_speed(speeds.tiny);
        } else if (state.offset_dir == offset_dirs.right) {
					set_L_motor_speed(speeds.tiny)
        } else {
          // ERROR
      }
    }
  }
}

void setup() {
  Serial.begin(9600);

  //setting the IR sensor pins as inputs
  pinMode(IR_sensor_pin_1, INPUT);
  pinMode(IR_sensor_pin_2, INPUT);
  pinMode(IR_sensor_pin_3, INPUT);
  
  //setting LEDs for Errors, movement
  pinMode(LEDpin_error, OUTPUT);
  pinMode(LEDpin_1, OUTPUT);
  pinMode(LEDpin_2, OUTPUT);
  pinMode(LEDpin_3, OUTPUT);
  //pinMode(LEDpin_mag, OUTPUT);
  //pinMode(LEDpin_nonmag, OUTPUT);

}

void loop() {

  

  Serial.println(" ");
  delay(20);
}

/*
Bool same_a3(int a[3], int b[3]) {
  for (int i = 0; i < 3; i++) {
    if (a[i] !== b[i]) {
      return false
    }
  }
  return true
}
*/