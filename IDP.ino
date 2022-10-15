#include <Adafruit_MotorShield.h>

// Create the motor shield object with the default I2C address
Adafruit_MotorShield AFMS = Adafruit_MotorShield();
Adafruit_DCMotor *L_motor = AFMS.getMotor(1);
Adafruit_DCMotor *R_motor = AFMS.getMotor(2);
int LEDpin_error = 3;
struct front_sensor_pins_struct {int left; int mid; int right;};
front_sensor_pins_struct front_sensor_pins  = {0, 1, 2};

struct modes_struct{
  int start;
	int approaching_symmetric_junct;
	int making_right_turn;
	int basic; // (basic line following)
	int approaching_block_on_line;
	int testing_block;
	int lowering_grabber;
	int approaching_tunnel;
	int in_tunnel;
	int approaching_right_turn_to_take;
	int approaching_box;
	int raising_grabber;
	int doing_a_180;
	int lost_line;
  }
modes_struct modes = {1,2,3,4,5,6,7,8,9,10,11,12,13,14};

struct speeds_struct {int tiny; int low; int med; int high;};
speeds_struct speeds = {100, 150, 200, 250};

struct offset_dirs_struct {int none; int left; int right; int unknown;};
offset_dirs_struct offset_dirs = {0,1,2,3};

struct offset_exts_struct {int none; int little; int mid; int far;};
offset_exts_struct offset_exts = {0,1,2,3};

struct line_end_likelihoods_struct {int none; int low; int med; int high; int as_before;};
line_end_likelihoods_struct line_end_likelihoods = {0,1,2,3};

struct state_struct {
  int motor_speeds[2]; int offset_dir;
  int offset_ext;
};
state_struct state = {
  {0, 0}, offset_dirs.none,
  offset_exts.none
};

void set_L_motor_speed(int speed) {
	if (state.motor_speeds[0] != speed) {
    state.motor_speeds[0] = speed;
    L_motor -> setSpeed(speed);
	}
}
void set_R_motor_speed(int speed) {
	if (state.motor_speeds[1] != speed) {
    state.motor_speeds[1] = speed;
    R_motor -> setSpeed(speed);
	}
}

int correct_trajectory() {
  int sensors[3] = {digitalRead(front_sensor_pins.left),
    digitalRead(front_sensor_pins.mid), digitalRead(front_sensor_pins.right)};
  int line_end_likelihood = line_end_likelihoods.none;
  if (sensors[0]) {
    if (sensors[1]) {
      if (sensors[2]) {
        // case [1, 1, 1]
        // ERROR
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
        // ERROR
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
        // Don't do anything if not off the line in the last correction
        if (state.offset_ext == offset_exts.far) {
          line_end_likelihood = line_end_likelihoods.as_before;
        } else {
          // decide how to change motor speeds
          if (state.offset_dir == offset_dirs.left) {
            set_R_motor_speed(speeds.tiny);
          } else if (state.offset_dir == offset_dirs.right) {
            set_L_motor_speed(speeds.tiny);
          }
          // decide what the line_end_likelihood is 
          /*
          switch (state.offset_ext) {
            case offset_exts.none:
              line_end_likelihood = line_end_likelihoods.high;
              break;
            case offset_exts.little:
              line_end_likelihood = line_end_likelihoods.med;
              break;
            case offset_exts.mid:
              line_end_likelihood = line_end_likelihoods.low;
              break;
          }
          */
          // set new offset extent
          state.offset_ext = offset_exts.far;
        }
      }
    }
  }
  return line_end_likelihood;
}

void setup() {
  Serial.begin(9600);

  //setting the IR sensor pins as inputs
  pinMode(front_sensor_pins.left, INPUT);
  pinMode(front_sensor_pins.mid, INPUT);
  pinMode(front_sensor_pins.right, INPUT);
  
  //setting LEDs for Errors, movement
  pinMode(LEDpin_error, OUTPUT);/*
  pinMode(LEDpin_1, OUTPUT);
  pinMode(LEDpin_2, OUTPUT);
  pinMode(LEDpin_3, OUTPUT);
  //pinMode(LEDpin_mag, OUTPUT);
  //pinMode(LEDpin_nonmag, OUTPUT);*/

  if (!AFMS.begin(/*default frequency 1.6KHz*/)) {
    Serial.println("Could not find Motor Shield. Check wiring.");
    while (1);
  }
  Serial.println("Motor Shield found.");

  L_motor->setSpeed(speeds.high);
  R_motor->setSpeed(speeds.high);
  L_motor->run(BACKWARD);
  R_motor->run(BACKWARD);

  

}

void loop() {
  //correct_trajectory();
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
