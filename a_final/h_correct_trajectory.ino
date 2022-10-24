#include <Arduino.h>

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