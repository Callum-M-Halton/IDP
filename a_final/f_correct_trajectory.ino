#include <Arduino.h>

// Categorising and assinging numeric index to probability of leaving line
struct line_end_likelihoods_struct {int none; int as_before; int high; int low;};
line_end_likelihoods_struct line_end_likelihoods = {0,1,2,3};

// Negative feedback step to continuously adjust the rover back on to the line depending on which sensors are firing
int correct_trajectory() {
  set_motor_dirs(FORWARD);
  int sensors[3] = {digitalRead(front_sensor_pins.left),
    digitalRead(front_sensor_pins.mid), digitalRead(front_sensor_pins.right)}; // creates array of front sensor inputs
  int line_end_likelihood = line_end_likelihoods.none;
  if (sensors[0]) {
    if (sensors[1]) {
      if (sensors[2]) {
        // INVALID CASE [1, 1, 1] - all 3 firing not possible in line following mode
        // Serial.println("Invalid case: [1, 1, 1]");
      } else {
        // case [1, 1, 0] - rover turning slightly right relative to line
        state.offset_dir = offset_dirs.right;
        state.offset_ext = offset_exts.moderate;
		    set_motor_speed(false, speeds.med);
        set_motor_speed(true, speeds.high);
      }
    } else {
      if (sensors[2]) {
        // INVALID CASE [1, 0, 1] - not possible
        // Serial.println("Invalid case: [1, 0, 1]");
      } else {
        // case [1, 0, 0] - rover turning very right
        state.offset_dir = offset_dirs.right;
        state.offset_ext = offset_exts.moderate;
		    set_motor_speed(false, speeds.low);
        set_motor_speed(true, speeds.high);
      }
    }
  } else {
    if (sensors[1]) {
      if (sensors[2]) {
        // case [0, 1, 1] - turning slightly left
        state.offset_dir = offset_dirs.left;
        state.offset_ext = offset_exts.moderate;
		    set_motor_speed(true, speeds.med);
        set_motor_speed(false, speeds.high);
      } else {
        // case [0, 1, 0] - going straight on
        //state.offset_dir = offset_dirs.none; +++important+++
        state.offset_ext = offset_exts.none;
		    set_motor_speed(false, speeds.high);
        set_motor_speed(true, speeds.high);
      }
    } else {
      if (sensors[2]) {
        // case [0, 0, 1] - going very left
        state.offset_dir = offset_dirs.left;
        state.offset_ext = offset_exts.moderate;
		    set_motor_speed(true, speeds.low);
        set_motor_speed(false, speeds.high);
      } else {
        // case [0, 0, 0] - not on line
        // Don't do anything if already off the line in the last correction
        if (state.offset_ext == offset_exts.large) {
          line_end_likelihood = line_end_likelihoods.as_before;
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
          // if now there is no line being detected and the offset is supposed to be none then the line has ended abruptly at the tunnel
          if (state.offset_ext == offset_exts.none) {
            line_end_likelihood = line_end_likelihoods.high;
          // otherwise it is just that the line is far away from the rover and the extent is only large from this moment
          } else {
            line_end_likelihood = line_end_likelihoods.low;
          }
          // set new offset extent
          state.offset_ext = offset_exts.large;
        }
      }
    }
  }
  return line_end_likelihood;
}
