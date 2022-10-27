#include <Arduino.h>

void follow_line_step() {
  int suggested_timer = correct_trajectory();
  bool approaching_EOL = ( state.approaching == approachables.tunnel
    || state.approaching == approachables.home_box
    || state.approaching == approachables.deposit_box
  );
  if (suggested_timer != suggested_timers_by_line_end_likelihoods.none) {
    if (approaching_EOL) {
      if (state.approaching == approachables.tunnel) {
        traverse_tunnel();
      // === Otherwise approaching box ===
      } else if (state.approaching == approachables.home_box) {
        go_home();
      // Otherwise we're depositing a block
      } else {
        deposit_block();
      }
    } else if (suggested_timer == suggested_timers_by_line_end_likelihoods.as_before) {
      // if the timer is valid (not set to the null state of 0)...
      // ... and it has run out, assume buggy off line
      if (state.timer_end != 0 && millis() >= state.timer_end) {
        refind_line();
      }
    // Gone from [0, 1, 0] to [0, 0, 0]
    } else if (suggested_timer == suggested_timers_by_line_end_likelihoods.high) {
      Serial.println("Starting left line timer");
      // set timer
      state.timer_end = millis() + suggested_timer;
    // Otherwise not [0, 1, 0] to [0, 0, 0]...
    } else {
      // ... so set a timer_end to the null state 0, which will never run out
      state.timer_end = 0;
    }
  }
}

void refind_line() {
  Serial.println("Task: Refinding Line");
  unsigned long timer_end = millis() + 300;
  turn_on_spot(true);
  while(millis() < timer_end && !any_front_line_sensors_firing()) {
    delayMicroseconds(1);
  }
  set_motor_speeds(0);
  if (any_front_line_sensors_firing()) { return; }

  timer_end = millis() + 600;
  turn_on_spot(false);
  while(millis() < timer_end && !any_front_line_sensors_firing()) {
    delayMicroseconds(1);
  }
  set_motor_speeds(0);
  if (any_front_line_sensors_firing()) { return; }
  
  /*
  bool seq[4] = {true, false, false, true};
  for (int t = 50; t < 1000; t += 50) {
    for (int s = 0; s < 5; s++) {
      timer_end = millis() + t;
      turn_on_spot(seq[s]);
      while(millis() < timer_end && !any_front_line_sensors_firing()) {
        delayMicroseconds(1);
      }
      if (any_front_line_sensors_firing()) { return; }
    }
  }
  */
}

void traverse_tunnel() {
  Serial.println("Task: Traversing Tunnel");
  set_motor_dirs(FORWARD);
  set_motor_speeds(255);
  delay(3500);
  refind_line();
  if (state.has_block) {
    state.approaching = approachables.straight_before_juncts;
    state.super_timer_end = millis() + 5000; // TUUUUUUNE
  } else {
    state.approaching = approachables.straight_before_block;
    state.super_timer_end = millis() + 7100; // TUUUUUUNE
  }
}

void make_right_turn() {
  Serial.println("Task: Making Right Turn");

  turn_on_spot(true);
  delay(200);
  while (!any_front_line_sensors_firing()) {
    delayMicroseconds(1);
  }
  if (state.approaching == approachables.deposit_junct) {
    state.approaching = approachables.deposit_box;
  } else {
    state.approaching = approachables.home_box;
  }
}

void go_home() {
  Serial.println("Task: Going Home");
  set_motor_dirs(FORWARD);
  set_motor_speeds(255);
  delay(1000);
  set_motor_speeds(0);
  while (1);
}

void deposit_block() {
  Serial.println("Task: Depositing Block");
  // drop off block
  set_motor_dirs(FORWARD);
  set_motor_speeds(255);
  delay(500);
  raise_grabber();
  set_motor_dirs(BACKWARD);
  set_motor_speeds(255);
  delay(500);
  // escape box
  turn_on_spot(false);
  delay(1500);
  set_motor_dirs(FORWARD);
  set_motor_speeds(255);
  delay(1200);
  // rejoin line
  while (!any_front_line_sensors_firing()){
    delayMicroseconds(1);
  }
  state.approaching = approachables.home_junct;
}

void aquire_block() {
  Serial.println("Task: Acquiring Block");
  set_motor_speeds(200);
  while(get_ultrasonic_distance() > 3) {
    delayMicroseconds(1);
  }
  set_motor_speeds(0);
  lower_grabber();
  state.has_block = true;
  state.speed_coeff = 1.0;
  turn_on_spot(true);
  delay(200);
  while (!any_front_line_sensors_firing()){
    delayMicroseconds(1);
  }
  state.approaching = approachables.straight_before_tunnel;
  state.super_timer_end = millis() + 7000;
}

void leave_start(){
  Serial.println("Task: Leaving Start Box");
  state.super_timer_end = millis() + 9300;
  state.approaching = approachables.straight_before_tunnel;
  //rotate right slightly to hit line at angle
  turn_on_spot(false);
  delay(500);

  // go to the line and skip the box
  set_motor_dirs(FORWARD);
  delay(1200);

  while (!any_front_line_sensors_firing()){
    delayMicroseconds(1);
  }
}


/*
//unsigned long start_time;
void reverse_run(bool ignore_sensors) {
  state.time_stamp_of_cmd_being_rev_run = millis();
  state.timer_end = millis();
  //start_time = millis();
  while (ignore_sensors || !any_front_line_sensors_firing()) {
    Serial.println("rev");
    if (millis() >= state.timer_end) {
      if (state.motor_cmds.size() < 1) {
        return;
      }
      motor_cmd_struct last_cmd = state.motor_cmds.pop();
      if (last_cmd.is_flag) { break; }
      set_motor_dir(false, 3 - last_cmd.dirs[0], false);
      set_motor_dir(true, 3 - last_cmd.dirs[1], false);
      set_motor_speed(false, last_cmd.speeds[0], false);
      set_motor_speed(true, last_cmd.speeds[1], false);

      unsigned long timer_length = state.time_stamp_of_cmd_being_rev_run - last_cmd.time_stamp;
      state.timer_end = millis() + timer_length;
      state.time_stamp_of_cmd_being_rev_run = last_cmd.time_stamp;
    } else {
      delay(1);
    }
  }
  set_motor_speeds(0);
}
*/

/*
void leave_start() {
  Serial.println("Task: Leaving Start");
  set_motor_dirs(FORWARD);
  set_motor_speeds(255);
  delay(TIME_TO_DRIVE_FORWARD_FOR_AT_START);

  // reverse until the side sensor detects the line
  set_motor_dirs(BACKWARD);
  set_motor_speeds(255);
  bool not_yet = true
  while (!digitalRead(JUNCT_SENSOR_PIN)) {
    delayMicroseconds(1);
  }
  
  // turns right until line detected
  // set direction to turn right
  set_motor_dir(false, FORWARD);
  set_motor_dir(true, BACKWARD);

  set_motor_speeds(255);
  // while no front sensors are firing
  while (!any_front_line_sensors_firing()){
    delayMicroseconds(1);
  }
  set_motor_speeds(0);
  // We're now on the main loop at the first on-loop sector
  set_sector(0);
}*/

/*void refind_line() {
  //print_motor_cmds();
  Serial.println("Task: Refinding Line");
  set_motor_dirs(FORWARD);
  set_motor_speed(false, 255);
  set_motor_speed(true, 150);
  while(!any_front_line_sensors_firing()) {
    delayMicroseconds(1);
  }

  // perturb right if sector is ramp_straight otherwise perturb left
  bool perturb_right = state.sector_code == 3; // ramp straight code
  int dirs[2] = {FORWARD, BACKWARD};
  set_motor_dir(false, dirs[int(!perturb_right)], false);
  set_motor_dir(true, dirs[int(perturb_right)], false);
  set_motor_speeds(255, false);
  while (any_front_line_sensors_firing()) {
    delayMicroseconds(1);
  }
  set_motor_dir(false, dirs[int(perturb_right)], false);
  set_motor_dir(true, dirs[int(!perturb_right)], false);
  while (!any_front_line_sensors_firing()) {
    delayMicroseconds(1);
  }
  set_motor_speeds(0, false);
  digitalWrite(ERROR_LED_PIN, LOW);
}*/

/*
void print_motor_cmds() {
  for (int i = 0; i < state.motor_cmds.size(); i++) {
    motor_cmd_struct cmd = state.motor_cmds.get(i);
    Serial.println("=========================");
    Serial.println(String(cmd.dirs[0]) + " " + String(cmd.dirs[1]));
    Serial.println(String(cmd.speeds[0]) + " " + String(cmd.speeds[1]));
    Serial.println(String(cmd.time_stamp) + " " + String(cmd.is_flag));
    Serial.println("=========================");
  }
}
*/

/* ======= bits ======
  // adds a flag to the motor cmds history so we can reverse up until this point later
  add_motor_cmd(true // true indicates to add a flag command);

  reverse_run(true);
  set_motor_dirs(BACKWARD);
  set_motor_speeds(255);
  delay(200);
  set_motor_speeds(0);
  state.approaching = approachables.junct_on_right;
  state.sector_code_to_turn_off_after = -1;

  if (test_if_magnetic()) {
    // 11 => "straight_before_green_junct"
    state.sector_code_to_turn_off_after = 11;
  } else {
    // 0 => "straight_after_start_junct" i.e. straight before red junct
    state.sector_code_to_turn_off_after = 0;
  }
*/
