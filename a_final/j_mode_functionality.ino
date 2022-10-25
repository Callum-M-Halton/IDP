#include <Arduino.h>

void follow_line_step() {
  int suggested_timer = correct_trajectory();
  bool approaching_EOL = false;//(state.approaching == approachables.tunnel);
  if (suggested_timer != suggested_timers_by_line_end_likelihoods.none) {
    // if there's an existing timer
    if (suggested_timer == suggested_timers_by_line_end_likelihoods.as_before) {
      // if the timer is valid (not set to the null state of 0)...
      // ... and it has run out, assume buggy off line
      Serial.println(state.timer_end - millis());
      if (state.timer_end != 0 && millis() >= state.timer_end) {
        if (approaching_EOL) {
          next_on_loop_sector();
        } else {
          set_mode(modes.refinding_line);
        }
      }
    // Otherwise if expecting an end of line...
    } else if (approaching_EOL 
        // ... or gone from [0, 1, 0] to [0, 0, 0]
        || suggested_timer == suggested_timers_by_line_end_likelihoods.high) {
      Serial.println("aaaaaaaaaaaaaaaaaaaaaaaa");
      // set timer
      state.timer_end = millis() + suggested_timer;
    // Otherwise not expecting end of line and not [0, 1, 0] to [0, 0, 0]...
    } else {
      // ... so set a timer_end to the null state 0, which will never run out
      state.timer_end = 0;
    }
  }
}

void refind_line_step() {
  if (any_front_line_sensors_firing()) {
    bool perturb_right = true; // set via sector!!!!!!!!
    int dirs[2] = {FORWARD, BACKWARD}
    set_motor_dir(false, dirs[int(!perturb_right)], false);
    set_motor_dir(true, dirs[int(perturb_right)], false);
    set_motor_speeds(speeds.high, false)
    while (any_front_line_sensors_firing()) {
      delayMicroseconds(1);
    }
    set_motor_dir(false, dirs[int(perturb_right)], false);
    set_motor_dir(true, dirs[int(!perturb_right)], false);
    while (!any_front_line_sensors_firing()) {
      delayMicroseconds(1);
    }
    digitalWrite(ERROR_LED_PIN, LOW);
    set_mode(modes.following_line);
  } else if (!state.timer_set || millis() >= state.timer_end) {
      if (!state.timer_set) { state.time_stamp_of_cmd_being_rev_run = millis(); }
      motor_cmd_struct last_cmd = state.motor_cmds.pop();

      set_motor_dir(false, last_cmd.dirs[0], false);
      set_motor_dir(true, last_cmd.dirs[1], false);
      set_motor_speed(false, last_cmd.speeds[0], false);
      set_motor_speed(true, last_cmd.speeds[1], false);

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

void traverse_tunnel() {
  set_motor_speeds(speeds.tunnel);
  while (!any_front_line_sensors_firing()) {
    delayMicroseconds(1);
  }
  next_on_loop_sector();
}
