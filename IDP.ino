#include <Adafruit_MotorShield.h>

// Create the motor shield object with the default I2C address
Adafruit_MotorShield AFMS = Adafruit_MotorShield();
Adafruit_DCMotor *L_motor = AFMS.getMotor(2);
Adafruit_DCMotor *R_motor = AFMS.getMotor(1);
int LEDpin_error = 3;
struct front_sensor_pins_struct {int left; int mid; int right;};
front_sensor_pins_struct front_sensor_pins  = {2, 1, 0};

struct modes_struct {
  const int start;
	const int approaching_symmetric_junct;
	const int making_right_turn;
	const int basic; // (basic line following)
	const int approaching_block_on_line;
	const int testing_block;
	const int lowering_grabber;
	const int approaching_tunnel;
	const int in_tunnel;
	const int approaching_right_turn_to_take;
	const int approaching_box;
	const int raising_grabber;
	const int doing_a_180;
	const int lost_line;
};
constexpr modes_struct modes = {1,2,3,4,5,6,7,8,9,10,11,12,13,14};

// debug
void print_mode(int mode) {
  switch (mode) {
    case modes.start: Serial.println("start"); break;
    case modes.approaching_symmetric_junct: Serial.println("approaching_symmetric_junct"); break;
    case modes.making_right_turn: Serial.println("making_right_turn"); break;
    case modes.basic: Serial.println("basic"); break;
    case modes.approaching_block_on_line: Serial.println("approaching_block_on_line"); break;
    case modes.testing_block: Serial.println("testing_block"); break;
    case modes.lowering_grabber: Serial.println("lowering_grabber"); break;
    case modes.approaching_tunnel: Serial.println("approaching_tunnel"); break;
    case modes.in_tunnel: Serial.println("in_tunnel"); break;
    case modes.approaching_right_turn_to_take: Serial.println("approaching_right_turn_to_take"); break;
    case modes.approaching_box: Serial.println("approaching_box"); break;
    case modes.raising_grabber: Serial.println("raising_grabber"); break;
    case modes.doing_a_180: Serial.println("doing_a_180"); break;
    case modes.lost_line: Serial.println("lost_line"); break;
  }
}

struct speeds_struct {int tiny; int low; int med; int high;};
speeds_struct speeds = {0, 100, 125, 250};

struct offset_dirs_struct {int none; int left; int right; int unknown;};
offset_dirs_struct offset_dirs = {0,1,2,3};

struct offset_exts_struct {
  const int none; const int little; const int mid; const int far;};
constexpr offset_exts_struct offset_exts = {0,1,2,3};

struct stblil_struct {int none; int as_before; int high; int med; int low; };
stblil_struct suggested_timers_by_line_end_likelihoods = {
  -2, -1, 50, 200, 300
};

struct state_struct {
  int motor_speeds[2]; int offset_dir;
  int offset_ext; int mode;
  bool following_line; int count_down;
  int prev_mode;
};
state_struct state = {
  {0, 0}, offset_dirs.none,
  offset_exts.none, -1, 
  true, -1,
  -1
};

void set_L_motor_speed(int speed) {
  speed *= 0.7;
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

void set_mode(int mode) {
  switch(mode) {
    case modes.start:
      state.following_line = false;
      break;
    case modes.approaching_symmetric_junct:
      state.following_line = true;
      break;
    case modes.making_right_turn:
      state.following_line = false;
      break;
    case modes.basic:
      state.following_line = true;
      break;
    case modes.approaching_block_on_line:
      state.following_line = true;
      break;
    case modes.testing_block:
      state.following_line = false;
      break;
    case modes.lowering_grabber:
      state.following_line = false;
      break;
    case modes.approaching_tunnel:
      state.following_line = true;
      break;
    case modes.in_tunnel:
      state.following_line = false;
      break;
    case modes.approaching_right_turn_to_take:
      state.following_line = true;
      break;
    case modes.approaching_box:
      state.following_line = true;
      break;
    case modes.raising_grabber:
      state.following_line = false;
      break;
    case modes.doing_a_180:
      state.following_line = false;
      break;
    case modes.lost_line:
      state.following_line = false;
      break;
  }
  if (mode = modes.lost_line) {
    digitalWrite(LEDpin_error, HIGH);
  } else {
    digitalWrite(LEDpin_error, LOW);
  }
  state.mode = mode;
}

int correct_trajectory() {
  int sensors[3] = {digitalRead(front_sensor_pins.left),
    digitalRead(front_sensor_pins.mid), digitalRead(front_sensor_pins.right)};
  int suggested_timer = suggested_timers_by_line_end_likelihoods.none;
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
          suggested_timer =
            suggested_timers_by_line_end_likelihoods.as_before;
        } else {
          // decide how to change motor speeds
          if (state.offset_dir == offset_dirs.left) {
            set_R_motor_speed(speeds.tiny);
          } else if (state.offset_dir == offset_dirs.right) {
            set_L_motor_speed(speeds.tiny);
          }
          // decide what the line_end_likelihood is 
          switch (state.offset_ext) {
            case offset_exts.none:
              suggested_timer = suggested_timers_by_line_end_likelihoods.high;
              break;
            case offset_exts.little:
              suggested_timer = suggested_timers_by_line_end_likelihoods.med;
              break;
            case offset_exts.mid:
              suggested_timer = suggested_timers_by_line_end_likelihoods.low;
              break;
          }
          // set new offset extent
          state.offset_ext = offset_exts.far;
        }
      }
    }
  }
  return suggested_timer;
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

  
  set_L_motor_speed(speeds.high);
  set_R_motor_speed(speeds.high);
  L_motor->run(FORWARD);
  R_motor->run(FORWARD);
  set_mode(modes.basic);

}

void print_sensor_vals() {
  Serial.print(digitalRead(front_sensor_pins.left));
  Serial.print(digitalRead(front_sensor_pins.mid));
  Serial.println(digitalRead(front_sensor_pins.right));
}

void loop() {
  correct_trajectory();
  // Line Follow if in a line_following mode
  if (state.following_line) {
    int suggested_timer = correct_trajectory();
    bool approaching_EOL = (state.mode == modes.approaching_symmetric_junct
      || state.mode == modes.approaching_tunnel
      || state.mode == modes.approaching_box);
    
    if (suggested_timer != suggested_timers_by_line_end_likelihoods.none) {
      // if there's an existing timer
      if (suggested_timer == suggested_timers_by_line_end_likelihoods.as_before) {
        state.count_down -= 1;
        // if the timer has run out assume buggy off line
        if (state.count_down == 0) {
          if (approaching_EOL) {
            // update sector
          } else {
            state.prev_mode = state.mode;
            set_mode(modes.lost_line);
            Serial.println("uuuuuh oohhhhhhh");
          }
        }
      // Otherwise initialise countdown with suggested timer
      } else if (approaching_EOL 
          // i.e. if gone from [0, 1, 0] to [0, 0, 0]
          || suggested_timer == suggested_timers_by_line_end_likelihoods.high) {
        state.count_down = suggested_timer;
      }
    }
  }

  // Recover line if in lost_line mode
  if (state.mode == modes.lost_line) {
    set_L_motor_speed(0);
    set_R_motor_speed(0);
  }

  //print_sensor_vals();
  //print_mode(state.mode);
}
