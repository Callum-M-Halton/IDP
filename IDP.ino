#include <Adafruit_MotorShield.h>
#include <StackArray.h>
#include <LinkedList.h>
#include <Servo.h>


// Initialise Motors and Servo
Adafruit_MotorShield AFMS = Adafruit_MotorShield();
Adafruit_DCMotor *L_motor = AFMS.getMotor(4);
Adafruit_DCMotor *R_motor = AFMS.getMotor(3);
Servo myservo;

// Define constants
// pins (not settled)
const int JUNCT_SENSOR_PIN = 3;
struct front_sensor_pins_struct {int left; int mid; int right;};
front_sensor_pins_struct front_sensor_pins  = {0,1,2};
const int ERROR_LED_PIN = 4;
const int SERVO_PIN = 5;
struct US_pins_struct {int echo; int trig;};
US_pins_struct front_US_pins = {6, 7}; // for HC-SR04
US_pins_struct side_US_pins = {8, 9}; // for HC-SR04
// other
const int MOTOR_CMDS_SIZE = 100;
const int TURN_DISPARITIES_SAMPLE_LENGTH = 100; //TUNE
const int CURVING_LEFT_THRESHOLD = 10000; //TUNE
const int GOING_STRAIGHT_THRESHOLD = 50; //TUNE
const unsigned long TIME_TO_DRIVE_FORWARD_FOR_AT_START = 3000; //TUNE
const int DROP_GRABBER_VALUE = 90; //TUNE
const int RAISE_GRABBER_VALUE = 0; //TUNE
struct speeds_struct {int tiny; int low; int med; int high; int tunnel;};
speeds_struct speeds = {0, 50, 150, 255, 100};
struct stblil_struct {int none; int as_before; int high; int med; int low; };
stblil_struct suggested_timers_by_line_end_likelihoods = { -2, -1, 1000, 10000, 10000 };

// ============== MODES ==============
struct modes_struct {
  const int following_line; // must = 0!!!!!!
  const int finding_line_at_start;
  const int turning_at_symmetric_junction;
	const int making_right_turn;
	const int testing_block;
	const int lowering_grabber;
	const int traversing_tunnel;
	const int approaching_box;
	const int raising_grabber;
	const int doing_a_180;
	const int refinding_line;
  const int refinding_line_after_tunnel;
};
constexpr modes_struct modes = {0,1,2,3,4,5,6,7,8,9,10,11};
String mode_strings[12] = {
"following_line",
"finding_line_at_start",
"turning_at_symmetric_junction",
"making_right_turn",
"testing_block",
"lowering_grabber",
"traversing_tunnel",
"approaching_box",
"raising_grabber",
"doing_a_180",
"refinding_line",
"refinding_line_after_tunnel"
};

// ++++++++++++++++++

// ======= SECTORS =========
struct approachables_struct {
  const int tunnel; const int straight; int nothing;
  const int junct_on_right; const int corner;
};
constexpr approachables_struct approachables = {0,1,2,3,4};

struct on_loop_sector_struct {
  String name;
  // -1 indicates no off loop sector option at next junction
  int off_loop_sector_code;
  int approaching;
  int associated_mode;
  float speed_coeff;
};

const int num_of_on_loop_sectors = 13;
on_loop_sector_struct on_loop_sectors[num_of_on_loop_sectors] = {
  {"straight_after_start_junct", -1, approachables.junct_on_right, 0, 1.0},
  {"straight_afer_red_junct", -1, approachables.corner, 0, 1.0},
  {"corner_before_ramp", -1, approachables.straight, 0, 1.0},
  {"ramp_straight", -1, approachables.corner, 0, 1.0},

  {"corner_after_ramp", -1, approachables.straight, 0, 1.0},
  {"block_straight", -1, approachables.corner, 0, 1.0}, // sector 5
  {"corner_before_tunnel", -1, approachables.straight, 0, 1.0},

  {"section_before_tunnel", -1, approachables.tunnel, 0, 0.7},
  {"tunnel", -1, approachables.nothing, modes.traversing_tunnel, 1.0},
  {"section_after_tunnel", -1, approachables.corner, 0, 1.0},
  {"corner_after_tunnel", -1, approachables.straight, 0, 1.0},

  {"straight_before_green_junct", -1, approachables.junct_on_right, 0, 1.0},
  {"straight_after_green_junct", -1, approachables.junct_on_right, 0, 1.0}
};
// ++++++++++++++++

// ==== OTHER ENUMS ====
struct offset_dirs_struct {int none; int left; int right; int unknown;};
offset_dirs_struct offset_dirs = {0,1,2,3};

struct offset_exts_struct {
  const int none; const int little; const int mid; const int far;
};
constexpr offset_exts_struct offset_exts = {0,1,2,3};
// ++++++++++++++++++++

// ===== STATE ========
struct motor_cmd_struct {
  bool is_right; int speed; unsigned long time_stamp;
};

struct state_struct {
  /*StackArray<int> prev_modes;*/
  int motor_speeds[2]; int offset_dir;
  int offset_ext; int mode;
  bool timer_set; float avg_turns_disparity;
  // may revert this to prev_mode if stack not needed
  int approaching; unsigned long timer_end;
  LinkedList<motor_cmd_struct> motor_cmds; unsigned long time_stamp_of_cmd_being_rev_run;
  LinkedList<int> disparities_sample; int sector_code;
  //
  float speed_coeff; int blocks_collected;
};
state_struct state = {
  /*{},*/
  {0, 0}, offset_dirs.none,
  offset_exts.none, -1, 
  false, 0,
  //
  approachables.nothing, 0,
  LinkedList<motor_cmd_struct>(), 0,
  LinkedList<int>(), -1,
  //
  1.0, 1 /////////// 1 for testing, 0 in production!!!!!
};
// ++++++++++++++++++++++

// ======== SETTERS ========
void set_sector(int sector_code) {
  state.sector_code = sector_code;
  on_loop_sector_struct sector = on_loop_sectors[sector_code];
  Serial.println("Sector set to: " + sector.name);
  state.approaching = sector.approaching;
  state.speed_coeff = sector.speed_coeff;
  set_mode(sector.associated_mode);
}

void next_on_loop_sector() {
  set_sector((state.sector_code + 1) % num_of_on_loop_sectors);
}

void set_mode(int mode) {
  switch(mode) {
    case modes.following_line:
      state.offset_dir = offset_dirs.none;
      state.offset_ext = offset_exts.none;
      set_motor_speed(false, speeds.high);
      set_motor_speed(true, speeds.high);
      set_motor_dirs(FORWARD);
    break;
    case modes.refinding_line:
      digitalWrite(ERROR_LED_PIN, HIGH);
      set_motor_dirs(BACKWARD);
      Serial.println("lossssst liiiiiine!!!!!!!!");
    break;
  }
  Serial.println("Mode set to: " + mode_strings[mode]);
  state.mode = mode;
}
// ++++++++++++++++++++++

// ===== OUR FUNCTIONS =======
bool any_front_line_sensors_firing() {
  return digitalRead(front_sensor_pins.left) 
    || digitalRead(front_sensor_pins.mid)
    || digitalRead(front_sensor_pins.right);
}

void set_motor_dirs(int dir) {
  L_motor->run(dir);
  R_motor->run(dir);
}

void set_motor_speed(bool is_right, int speed) {
  speed *= state.speed_coeff;
  // underdrives right motor to match left motor
  if (is_right) {
    speed *= 0.9;
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

void set_motor_speeds(int speed) {
  set_motor_speed(false, speed);
  set_motor_speed(true, speed);
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
        //Serial.println("error: [1, 1, 1]");
      } else {
        // case [1, 1, 0]
        state.offset_dir = offset_dirs.right;
        state.offset_ext = offset_exts.little;
			  set_motor_speed(false, speeds.med);
        set_motor_speed(true, speeds.high);
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
        set_motor_speed(true, speeds.high);
      }
    }
  } else {
    if (sensors[1]) {
      if (sensors[2]) {
        // case [0, 1, 1]
        state.offset_dir = offset_dirs.left;
        state.offset_ext = offset_exts.little;
			  set_motor_speed(true, speeds.med);
        set_motor_speed(false, speeds.high);
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
        set_motor_speed(false, speeds.high);
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
            set_motor_speed(false, speeds.high);
          } else if (state.offset_dir == offset_dirs.right) {
            set_motor_speed(false, speeds.tiny);
            set_motor_speed(true, speeds.high);
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

void update_turns_disparity_and_sector(bool approaching_corner) {
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
		state.avg_turns_disparity -= oldest_disparity / TURN_DISPARITIES_SAMPLE_LENGTH;
		// Add contribution of newest turns disparity
		state.avg_turns_disparity += newest_disparity / TURN_DISPARITIES_SAMPLE_LENGTH;

    // If turns_disparity high enough,
    // right wheel overdriven enough to be curving left
    if (
        // From straight to curving
        (state.avg_turns_disparity > CURVING_LEFT_THRESHOLD
        && approaching_corner) ||
        // From curving to straight
        (state.avg_turns_disparity < GOING_STRAIGHT_THRESHOLD
        && !approaching_corner)
    ) {
      next_on_loop_sector();
    }
  }
}

// update to allow which sensor to be specified
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

void follow_line_step() {
  int suggested_timer = correct_trajectory();
  bool approaching_EOL = false;//(state.approaching == approachables.tunnel);
  if (suggested_timer != suggested_timers_by_line_end_likelihoods.none) {
    // if there's an existing timer
    if (suggested_timer == suggested_timers_by_line_end_likelihoods.as_before) {
      // if the timer is valid (not set to the null state of -1)...
      // ... and it has run out, assume buggy off line
      if (state.timer_end >= 0 && millis() >= state.timer_end) {
        if (approaching_EOL) {
          next_on_loop_sector();
        } else {
          set_mode(modes.refinding_line);
          Serial.println("eeeeeeee");
        }
      }
    // Otherwise if expecting an end of line...
    } else if (approaching_EOL 
        // ... or gone from [0, 1, 0] to [0, 0, 0]
        || suggested_timer == suggested_timers_by_line_end_likelihoods.high) {
      Serial.println("aaaaaaaaaaaaaaa");
      // set timer
      state.timer_end = millis() + suggested_timer;
    // Otherwise not expecting end of line and not [0, 1, 0] to [0, 0, 0]...
    } else {
      // ... so set a timer_end to the null state -1, which will never run out
      state.timer_end = -1;
    }
  }
}

void refind_line_step() {
  if (any_front_line_sensors_firing) {
    digitalWrite(ERROR_LED_PIN, LOW);
    set_mode(modes.following_line);
  } else if (!state.timer_set || millis() >= state.timer_end) {
      if (!state.timer_set) { state.time_stamp_of_cmd_being_rev_run = millis(); }
      motor_cmd_struct last_cmd = state.motor_cmds.pop();
      unsigned long timer_length = state.time_stamp_of_cmd_being_rev_run - last_cmd.time_stamp;
      state.time_stamp_of_cmd_being_rev_run = last_cmd.time_stamp;
      state.timer_end = millis() + timer_length;
      state.timer_set = true;
  }
}

void leave_start() {
  set_motor_speeds(speeds.high);
  delay(TIME_TO_DRIVE_FORWARD_FOR_AT_START);

  // reverse until the side sensor detects the line
  set_motor_dirs(BACKWARD);
  set_motor_speeds(speeds.med);
  while (!digitalRead(JUNCT_SENSOR_PIN)) {
    delayMicroseconds(1);
  }
  
  // turns right until line detected
  // set direction to turn right
  L_motor->run(FORWARD);
  R_motor->run(BACKWARD);  
  set_motor_speeds(speeds.high);
  // while no front sensors are firing
  while (!any_front_line_sensors_firing()){
    delayMicroseconds(1);
  }
  // We're now on the main loop at the first on-loop sector
  set_sector(0);
}

void lowering_grabber(){
    myservo.write(DROP_GRABBER_VALUE);
}

void raising_grabber(){
    myservo.write(RAISE_GRABBER_VALUE);
}

void traverse_tunnel() {
  set_motor_speeds(speeds.tunnel);
  while (!any_front_line_sensors_firing()) {
    delayMicroseconds(1);
  }
  next_on_loop_sector();
}
// ++++++++++++++++++

// ======= BUILT IN FUNCTIONS ========
void setup() {
  set_motor_speeds(0);
  delay(10000);
  Serial.begin(9600);

  // attach Servo and configure ultrasonic pins
  myservo.attach(SERVO_PIN);
  // Set trig pins as outputs and echo pins as inputs
  pinMode(front_US_pins.trig, OUTPUT);  pinMode(side_US_pins.trig, OUTPUT);
  pinMode(front_US_pins.echo, INPUT);  pinMode(side_US_pins.echo, INPUT);

  // Setting the IR sensor pins as inputs
  pinMode(front_sensor_pins.left, INPUT);
  pinMode(front_sensor_pins.mid, INPUT);
  pinMode(front_sensor_pins.right, INPUT);
  
  // Setting LEDs for errors
  pinMode(ERROR_LED_PIN, OUTPUT);

  // Connecting to motors
  if (!AFMS.begin(/*default frequency 1.6KHz*/)) {
    Serial.println("Could not find Motor Shield. Check wiring.");
    while (1);
  }
  Serial.println("Motor Shield found.");

  // state configuration
  set_sector(5);
  // set_mode(modes.finding_line_at_start);
}

void loop() {
  if (true) {
    // Check if any of the things being approached have been reached
    /*
    if (state.approaching == approachables.corner 
        || state.approaching == approachables.straight) {
      update_turns_disparity_and_sector(
        state.approaching == approachables.corner
      );
    }
    */
    // Execute mode functionality depending on state.mode
    switch (state.mode) {
      case modes.finding_line_at_start:
        leave_start();
      break;
      case modes.following_line:
        follow_line_step();
      break;
      case modes.refinding_line:
        refind_line_step();
      break;
      case modes.traversing_tunnel:
        traverse_tunnel();
      break;
    }
  }
  //set_motor_speeds(255);
  //set_motor_dirs(FORWARD);
  //print_sensor_vals();
  //Serial.println(digitalRead(JUNCT_SENSOR_PIN));
  //print_mode(state.mode);
}
// ++++++++++++++++++++

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
void turn_on_spot(to_the_right) {
    if (to_the_right){
        // set direction to turn right
        L_motor->run(FORWARD);
        R_motor->run(BACKWARD);
    } else {
	      // set direction to turn left
        L_motor->run(BACKWARD);
        R_motor->run(FORWARD);
    }
    
    int turn_period = 50 // need to calibrate
    set_motor_speed(true, speeds.high); //right motor
    set_motor_speed(false, speeds.high); //left motor
    delay(turn_period) // not gonna be doing anything apart form turning if turning on spot anyway
}
*/
