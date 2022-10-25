#include <Arduino.h>

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
  //L_motor->run(FORWARD);
  //R_motor->run(BACKWARD);
  set_motor_dir(false, FORWARD)
  set_motor_dir(true, BACKWARDS)

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
  set_motor_speeds(speeds.high)
  delay(800)

  while (!any_front_line_sensors_firing()){
    delayMicroseconds(1);
  }
  // We're now on the main loop at the first on-loop sector
  set_sector(0);
}
*/

void traverse_tunnel() {
  set_motor_speeds(speeds.tunnel);
  while (!any_front_line_sensors_firing()) {
    delayMicroseconds(1);
  }
  next_on_loop_sector();
}