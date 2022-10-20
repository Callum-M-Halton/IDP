#include <Adafruit_MotorShield.h>
#include <StackArray.h>
#include <LinkedList.h>

// Initialise Motors
Adafruit_MotorShield AFMS = Adafruit_MotorShield();
Adafruit_DCMotor *L_motor = AFMS.getMotor(2);
Adafruit_DCMotor *R_motor = AFMS.getMotor(1);

// Define lone constants
const int ERROR_LED_PIN = 3;
const int MOTOR_CMDS_SIZE = 100;
const int TURN_DISPARITIES_SAMPLE_LENGTH = 100; //TUNE
const int CURVING_LEFT_THRESHOLD = 100; //TUNE

struct front_sensor_pins_struct {int left; int mid; int right;};
front_sensor_pins_struct front_sensor_pins  = {2, 1, 0};

// ============== MODES ==============
struct modes_struct {
  const int finding_line_at_start;
  const int turning_at_symmetric_junction;
	const int making_right_turn;
	const int following_line;
	const int testing_block;
	const int lowering_grabber;
	const int in_tunnel;
	const int approaching_box;
	const int raising_grabber;
	const int doing_a_180;
	const int refinding_line;
};
constexpr modes_struct modes = {0,1,2,3,4,5,6,7,8,9,10};
String modes_strings[11] = [
  "finding_line_at_start",
  "turning_at_symmetric_junction",
	"making_right_turn",
	"following_line",
	"testing_block",
	"lowering_grabber",
	"in_tunnel",
	"approaching_box",
	"raising_grabber",
	"doing_a_180",
	"refinding_line",
];
void set_mode(int mode) {
  switch(mode) {
    case modes.refinding_line:
      digitalWrite(ERROR_LED_PIN, HIGH);
      L_motor->run(BACKWARD);
      R_motor->run(BACKWARD);
      Serial.println("lossssst liiiiiine!!!!!!!!");
      state.prev_modes.push(state.mode);
      break;
  }
  Serial.println(mode_strings[state.mode] + "=> " + mode_strings[mode]);
  state.mode = mode;
}

struct approachables_struct {
  int nothing; int symmetric_junct; int block_on_line; int tunnel;
  int right_turn_to_take; int cross; int corner;
};
things_to_approach_struct approachables = {1,2,3,4,5,6,7};

struct on_loop_sector_struct {
  String name;
  int off_loop_sector_code;
  int approaching;

};

// ORDER MATTERS!!!!!
// ONLY MAIN LOOP ENUMERATED SO FAR
struct on_loop_sector_codes_struct {
  int start_junct_to_red_junct; int red_junct_to_corner_before_ramp;
  int corner_before_ramp; int ramp_section;

  int corner_after_ramp; int corner_after_ramp_to_cross;
  int cross_to_corner_before_tunnel; int corner_before_tunnel;

  int section_before_tunnel; int tunnel;
  int section_after_tunnel; int corner_after_tunnel;
  
  int corner_after_tunnel_to_green_junct; int green_junct_to_start_junct;
};
sector_codes_struct SCs = {0,1,2,3,4,5,6,7,8,9,10,11,12};

sector_struct sectors_array[13] = {
  {"start_junct_to_red_junct", SCs.red_junct_to_corner_before_ramp, 0},
  {"red_junct_to_corner_before_ramp", SCs.corner_before_ramp, 0},
  {"corner_before_ramp", SCs.ramp_section, 0},
  {"ramp_section", SCs.corner_after_ramp, 0},

  {"corner_after_ramp", SCs.corner_after_ramp_to_cross, 0},
  {"corner_after_ramp_to_cross", SCs.cross_to_corner_before_tunnel, 0},
  {"cross_to_corner_before_tunnel", SCs.corner_before_tunnel, 0},
  {"corner_before_tunnel", SCs.section_before_tunnel, 0},

  {"section_before_tunnel", SCs.tunnel, 0},
  {"tunnel", SCs.section_after_tunnel, 0},
  {"section_after_tunnel", SCs.corner_after_tunnel, 0},
  {"corner_after_tunnel", SCs.corner_after_tunnel_to_green_junct, 0},

  {"corner_after_tunnel_to_green_junct", SCs.green_junct_to_start_junct, 0},
  {"green_junct_to_start_junct", SCs.start_junct_to_red_junct, 0},

  {"red_junct_to_red"}
};

struct speeds_struct {int tiny; int low; int med; int high;};
speeds_struct speeds = {0, 100, 125, 250};

// const int cycles_at_max_disparity_for_full_turn = 100;

struct offset_dirs_struct {int none; int left; int right; int unknown;};
offset_dirs_struct offset_dirs = {0,1,2,3};

struct offset_exts_struct {
  const int none; const int little; const int mid; const int far;};
constexpr offset_exts_struct offset_exts = {0,1,2,3};

struct stblil_struct {int none; int as_before; int high; int med; int low; };
stblil_struct suggested_timers_by_line_end_likelihoods = {
  -2, -1, 250, 500, 750
};

struct motor_cmd_struct {
  bool is_right; int speed; unsigned long time_stamp;
};

struct state_struct {
  int motor_speeds[2]; int offset_dir;
  int offset_ext; int mode;
  bool timer_set; float avg_turns_disparity;
  // may revert this to prev_mode if stack not needed
  StackArray<int> prev_modes; unsigned long timer_end;
  LinkedList<motor_cmd_struct> motor_cmds; unsigned long time_stamp_of_cmd_being_rev_run;
  LinkedList<int> disparities_sample; 
};
state_struct state = {
  {0, 0}, offset_dirs.none,
  offset_exts.none, -1, 
  false, 0
  //
  {}, 0,
  LinkedList<motor_cmd_struct>(), 0,
  LinkedList<int>() 
};

void set_motor_speed(bool is_right, int speed) {
  if (!is_right) {
    speed *= 0.7;
  }
	if (state.motor_speeds[int(is_right)] != speed) {
    state.motor_speeds[int(is_right)] = speed;
    if (is_right) {
      R_motor -> setSpeed(speed);
    } else {
      L_motor -> setSpeed(speed);
    }
    motor_cmd_struct motor_cmd = {is_right, speed, millis()};
    state.motor_cmds.add(motor_cmd);
    if (state.motor_cmds.size() > MOTOR_CMDS_SIZE) {
      state.motor_cmds.shift();
    }
	}
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
        Serial.println("error: [1, 1, 1]");
      } else {
        // case [1, 1, 0]
        state.offset_dir = offset_dirs.right;
        state.offset_ext = offset_exts.little;
			  set_motor_speed(false, speeds.med);
      }
    } else {
      if (sensors[2]) {
        // case [1, 0, 1]
        // ERROR
        Serial.println("error: [1, 0, 1]");
      } else {
        // case [1, 0, 0]
        state.offset_dir = offset_dirs.right;
        state.offset_ext = offset_exts.mid;
			  set_motor_speed(false, speeds.low);
      }
    }
  } else {
    if (sensors[1]) {
      if (sensors[2]) {
        // case [0, 1, 1]
        state.offset_dir = offset_dirs.left;
        state.offset_ext = offset_exts.little;
			  set_motor_speed(true, speeds.med);
      } else {
        // case [0, 1, 0]
        state.offset_dir = offset_dirs.none;
        state.offset_ext = offset_exts.none;
			  set_motor_speed(false, speeds.high);
        set_motor_speed(true, speeds.high);
      }
    } else {
      if (sensors[2]) {
        // case [0, 0, 1]
        state.offset_dir = offset_dirs.left;
        state.offset_ext = offset_exts.mid;
			  set_motor_speed(true, speeds.low);
      } else {
        // case [0, 0, 0]
        // Don't do anything if not off the line in the last correction
        if (state.offset_ext == offset_exts.far) {
          suggested_timer =
            suggested_timers_by_line_end_likelihoods.as_before;
        } else {
          // decide how to change motor speeds
          if (state.offset_dir == offset_dirs.left) {
            set_motor_speed(true, speeds.tiny);
          } else if (state.offset_dir == offset_dirs.right) {
            set_motor_speed(false, speeds.tiny);
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

void print_sensor_vals() {
  Serial.print(digitalRead(front_sensor_pins.left));
  Serial.print(digitalRead(front_sensor_pins.mid));
  Serial.println(digitalRead(front_sensor_pins.right));
}

void update_curving_left() {
  // Calculate and add the new turns disparity to the list
  int newest_disparity = state.motor_speeds[1] - state.motor_speeds[0];
	state.disparities_sample.add(newest_disparity);
	// Only possible when SAMPLE_LENGTH first reached so avg must be computed from scratch
	if (state.disparities_sample.size() == TURN_DISPARITIES_SAMPLE_LENGTH) {
    int tot = 0;
    for (int i = 0; i < state.disparities_sample.size(); i++) {
      tot += state.disparities_sample.get(i);
    }
		state.avg_turns_disparity = tot / TURN_DISPARITIES_SAMPLE_LENGTH;
  } else if (state.disparities_sample.size() > TURN_DISPARITIES_SAMPLE_LENGTH) {
		// Get and remove oldest turns disparity
    int oldest_disparity = state.disparities_sample.pop();
		// Remove contribution of oldest turns disparity from avg
		state.avg_turns_disparity -= oldest_disparity / SAMPLE_LENGTH;
		// Add contribution of newest turns disparity
		state.avg_turns_disparity += newest_disparity / SAMPLE_LENGTH;

    // If turns_disparity high enough,
    // right wheel overdriven enough to be curving right
    if (state.avg_turns_disparity > CURVING_LEFT_THRESHOLD) {
      if (!state.curving_left) {
        // From straight to curving
        state.curving_left = true;
        state.sector++;
      }
    } else {
      if (state.curving_left) {
        // From curving to straight
        state.curving_left = false;
        state.sector++;
      }
    }
  }
}

void setup() {
  Serial.begin(9600);

  //setting the IR sensor pins as inputs
  pinMode(front_sensor_pins.left, INPUT);
  pinMode(front_sensor_pins.mid, INPUT);
  pinMode(front_sensor_pins.right, INPUT);
  
  //setting LEDs for Errors, movement
  pinMode(ERROR_LED_PIN, OUTPUT);/*
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

  
  set_motor_speed(false, speeds.high);
  set_motor_speed(true, speeds.high);
  L_motor->run(FORWARD);
  R_motor->run(FORWARD);
  set_mode(modes.following_line);

}

void loop() {
  if (true) {
    // Line Follow if in line following mode
    if (state.mode = modes.following_line) {
      int suggested_timer = correct_trajectory();
      bool approaching_EOL = (state.mode == modes.approaching_symmetric_junct
        || state.mode == modes.approaching_tunnel
        || state.mode == modes.approaching_box);
      
      if (suggested_timer != suggested_timers_by_line_end_likelihoods.none) {
        // if there's an existing timer
        if (suggested_timer == suggested_timers_by_line_end_likelihoods.as_before) {
          // if the timer has run out assume buggy off line
          if (millis() >= state.timer_end) {
            if (approaching_EOL) {
              // update sector
            } else {
              set_mode(modes.refinding_line);
            }
          }
        // Otherwise initialise countdown with suggested timer
        } else if (approaching_EOL 
            // i.e. if gone from [0, 1, 0] to [0, 0, 0]
            || suggested_timer == suggested_timers_by_line_end_likelihoods.high) {
          state.timer_end = millis() + suggested_timer;
        }
      }
    }

    // Recover line if in refinding_line mode
    if (state.mode == modes.refinding_line) {
      if (digitalRead(front_sensor_pins.left) || digitalRead(front_sensor_pins.mid)
          || digitalRead(front_sensor_pins.right)) {
        set_motor_speed(false, 0); set_motor_speed(true, 0);
        L_motor->run(FORWARD);
        R_motor->run(FORWARD);
        set_mode(state.prev_modes.pop());
      } else if (!state.timer_set || millis() >= state.timer_end) {
          if (!state.timer_set) { state.time_stamp_of_cmd_being_rev_run = millis(); }
          motor_cmd_struct last_cmd = state.motor_cmds.pop();
          unsigned long timer_length = state.time_stamp_of_cmd_being_rev_run - last_cmd.time_stamp;
          state.time_stamp_of_cmd_being_rev_run = last_cmd.time_stamp;
          state.timer_end = millis() + timer_length;
          state.timer_set = true;
      }
    }
  }
  //print_sensor_vals();
  //print_mode(state.mode);
}
