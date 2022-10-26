#include <Arduino.h>

void follow_line_step() {
  int suggested_timer = correct_trajectory();
  bool approaching_EOL = false;
    //(state.approaching == approachables.tunnel || state.approaching == approachables.box);
  if (suggested_timer != suggested_timers_by_line_end_likelihoods.none) {
    // if there's an existing timer
    if (suggested_timer == suggested_timers_by_line_end_likelihoods.as_before) {
      // if the timer is valid (not set to the null state of 0)...
      // ... and it has run out, assume buggy off line
      Serial.println(state.timer_end - millis());
      if (state.timer_end != 0 && millis() >= state.timer_end) {
        if (approaching_EOL) {
          if (state.approaching == approachables.tunnel) {
            next_sector();
            traverse_tunnel();
          // === Otherwisee assume approaching box ===
          // If the sector_code was straight_before_start_junct then we're going home
          } else if (state.sector_code == 12){
            go_home();
          // Otherwise we're depositing a block
          } else {
            deposit_block();
          }
        } else {
          refind_line();
        }
      }
    // Otherwise if expecting an end of line...
    } else if (approaching_EOL 
        // ... or gone from [0, 1, 0] to [0, 0, 0]
        || suggested_timer == suggested_timers_by_line_end_likelihoods.high) {
      Serial.println("Starting left line timer");
      // set timer
      state.timer_end = millis() + suggested_timer;
    // Otherwise not expecting end of line and not [0, 1, 0] to [0, 0, 0]...
    } else {
      // ... so set a timer_end to the null state 0, which will never run out
      state.timer_end = 0;
    }
  }
}

void refind_line() {
  Serial.println("Task: Refinding Line");
  digitalWrite(ERROR_LED_PIN, HIGH);
  set_motor_dirs(BACKWARD);
  reverse_run(false);

  // perturb right if sector is tunnel otherwise perturb left
  bool perturb_right = state.sector_code == 8; // tunnel sector code
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
}

void leave_start() {
  Serial.println("Task: Leaving Start");
  set_motor_speeds(255);
  delay(TIME_TO_DRIVE_FORWARD_FOR_AT_START);

  // reverse until the side sensor detects the line
  set_motor_dirs(BACKWARD);
  set_motor_speeds(speeds.med);
  while (!digitalRead(JUNCT_SENSOR_PIN)) {
    delayMicroseconds(1);
  }
  
  // turns right until line detected
  // set direction to turn right
  //L_motor->run(FORWARD);
  //R_motor->run(BACKWARD);
  set_motor_dir(false, FORWARD);
  set_motor_dir(true, BACKWARD);

  set_motor_speeds(speeds.med);
  // while no front sensors are firing
  while (!any_front_line_sensors_firing()){
    delayMicroseconds(1);
  }
  // We're now on the main loop at the first on-loop sector
  set_sector(0);
}

/*
void leave_start(){
  //rotate right slightly to hit line at angle
  //L_motor->run(FORWARD);
  //R_motor->run(BACKWARD);
  set_motor_dir(false, FORWARD)
  set_motor_dir(true, BACKWARDS)  
  set_motor_speeds(speeds.med);
  delay(500)

  // go to the line and skip the box
  R_motor->run(FORWARD)
  set_motor_speeds(255)
  delay(800)

  while (!any_front_line_sensors_firing()){
    delayMicroseconds(1);
  }
  // We're now on the main loop at the first on-loop sector
  set_sector(0);
}
*/

void reverse_run(bool ignore_sensors) {
  state.time_stamp_of_cmd_being_rev_run = millis();
  state.timer_end = millis();
  while (ignore_sensors || !any_front_line_sensors_firing()) {
    if (millis() >= state.timer_end) {
      motor_cmd_struct last_cmd = state.motor_cmds.pop();
      if (last_cmd.is_flag) { break; }

      set_motor_dir(false, last_cmd.dirs[0], false);
      set_motor_dir(true, last_cmd.dirs[1], false);
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

void traverse_tunnel() {
  Serial.println("Task: Traversing Tunnel");
  set_motor_speeds(speeds.tunnel);
  while (!any_front_line_sensors_firing()) {
    delayMicroseconds(1);
  }
  next_sector();
}

void make_right_turn() {
  Serial.println("Task: Making Right Turn");
  // adds a flag to the motor cmds history so we can reverse up until this point later
  add_motor_cmd(true /* true indicates to add a flag command*/);
  turn_on_spot(true);
  delay(200);
  while (!any_front_line_sensors_firing()) {
    delayMicroseconds(1);
  }
  set_motor_speeds(0);
  state.approaching = approachables.box;
}

void go_home() {
  Serial.println("Task: Going Home");
  set_motor_dirs(FORWARD);
  set_motor_speeds(255);
  delay(TIME_TO_DRIVE_FORWARD_TO_GO_HOME);
  set_motor_speeds(0);
  while (1);
}

void deposit_block() {
  Serial.println("Task: Depositing Block");
  set_motor_dirs(FORWARD);
  set_motor_speeds(255);
  delay(TIME_TO_DRIVE_FORWARD_TO_DROP_BLOCK);
  raise_grabber();
  reverse_run(true);
  set_motor_dirs(BACKWARD);
  set_motor_speeds(255);
  delay(200);
  set_motor_speeds(0);
  state.approaching = approachables.junct_on_right;
  state.sector_code_to_turn_off_after = -1;
}

void aquire_block() {
  Serial.println("Task: Acquiring Block");
  set_motor_speeds(0);
  if (test_if_magnetic()) {
    // 11 => "straight_before_green_junct"
    state.sector_code_to_turn_off_after = 11;
  } else {
    // 0 => "straight_after_start_junct" i.e. straight before red junct
    state.sector_code_to_turn_off_after = 0;
  }
  lower_grabber();
  state.speed_coeff = 1.0;
  state.approaching = approachables.corner;
}
