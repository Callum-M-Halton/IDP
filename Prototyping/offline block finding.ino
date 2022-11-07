#include <Arduino.h>

// detect the block
if (state.approaching == approachables.block_to_left) {
  int dist = get_ultrasonic_distance(false);
  state.last_side_dist != -1;
  if (state.last_side_dist != -1) {
    if (state.last_side_dist - dist > 10) { //TUNE/measure
      find_block();
    }
  }
  state.last_side_dist = dist;
}

// aquire the block
void find_block() {
  Serial.println("Task: Finding Block");
  state.recording = true;
  turn_on_spot(false);
  delay(1000); // TUNE
  set_motor_speeds(0);

  int dist = get_ultrasonic_distance(true);
  int prev_dist;
  while (1) {
    prev_dist = dist;
    set_motor_dirs(FORWARD);
    set_motor_speeds(255);
    while (abs(dist - prev_dist) < DELTA_DIST_FOR_LOST_BLOCK) { // TUNE
      my_milli_delay();
      prev_dist = dist;
      dist = get_ultrasonic_distance(true);
      if (dist <= 3) {
        set_motor_speeds(0);
        break;
      }
    }
    turn_on_spot(true);
    while (abs(dist - prev_dist) >= DELTA_DIST_FOR_LOST_BLOCK) {
      my_milli_delay();
      dist = get_ultrasonic_distance(true);
    }
  }

  if (test_if_magnetic()) {
    state.block_type = block_types.mag;
  } else {
    state.block_type = block_types.non_mag;
  }
  lower_grabber();

  reverse_run();
  refind_line();
  // Now returned to start before find_block() called
}