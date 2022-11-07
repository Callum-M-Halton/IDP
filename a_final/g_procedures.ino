#include <Arduino.h>

// (For testing recording of motor cmds)
void print_motor_cmds() {
  for (int i = 0; i < state.motor_cmds.size(); i++) {
    motor_cmd_struct cmd = state.motor_cmds.get(i);
    Serial.println("=========================");
    Serial.println(String(cmd.dirs[0]) + " " + String(cmd.dirs[1]));
    Serial.println(String(cmd.speeds[0]) + " " + String(cmd.speeds[1]));
    Serial.println(String(cmd.time_stamp));
    Serial.println("=========================");
  }
}

// Runs the stored movements in reverse to go back to original position
void reverse_run(bool ignore_sensors=true) {
  Serial.println("Task: Reverse Running");
  state.recording = false; // don't want to record the reversed process
  state.time_stamp_of_cmd_being_rev_run = millis();
  state.timer_end = millis();
  while (ignore_sensors || !any_front_line_sensors_firing()) {
    if (millis() >= state.timer_end) {
      if (state.motor_cmds.size() == 0) {
        break;
      }

      // gets the last movement from stack and reverses it
      motor_cmd_struct last_cmd = state.motor_cmds.pop();
      set_motor_dir(false, 3 - last_cmd.dirs[0]);
      set_motor_dir(true, 3 - last_cmd.dirs[1]);
      set_motor_speed(false, last_cmd.speeds[0], true);
      set_motor_speed(true, last_cmd.speeds[1], true);
      
      // reversed for same length it was executed originally
      unsigned long timer_length = state.time_stamp_of_cmd_being_rev_run - last_cmd.time_stamp;

      state.timer_end = millis() + timer_length;
      state.time_stamp_of_cmd_being_rev_run = last_cmd.time_stamp;
    } else {
      my_delay(1);
    }
  }
  // release motors once back in position
  set_motor_speeds(0);
}

// turning back into start box once finished
void go_home() {
  Serial.println("Task: Going Home");
  // turn right at junction
  turn_on_spot(true);
  my_delay(1200); ///////
  // go forwards into home box
  set_motor_dirs(FORWARD);
  my_delay(1300);
  // stop
  set_motor_speeds(0);
  while (1);
}


// manually turning and going into start box from the red box
/*
void go_home_from_red_box() {
  turn_on_spot(true);
  my_delay(1025); //TUNE
  set_motor_dirs(FORWARD);
  my_delay(2000); //TUNE
  while (!any_front_line_sensors_firing()) {
    my_milli_delay();
  }
  set_motor_speed(true, 255);
  set_motor_speed(false, 230);
  set_motor_speeds(0);
  //myservo.write(45);
  turn_on_spot(false);
  my_delay(250);
  set_motor_dirs(FORWARD);
  my_delay(1175);
  set_motor_speeds(0);
  while(1);
}
*/

/////////////
// going home from green box
void turn_around_and_go_home() {
  // Start 180 degree turn
  turn_on_spot(true);
  my_delay(2000);
  
  // Continue turning until back on line facing towards the home junction
  while (!any_front_line_sensors_firing()) {
    my_milli_delay();
  }
  // Stop
  set_motor_dirs(0);
  // Start a timer for the time taken to get from the current position to just before the home junction
  // Where we will start listening to the junction sensor for the turn to the home box
  start_super_timer(ST_lengths.just_after_green_junct_to_just_before_home_junct);
  state.approaching = approachables.just_before_home_junct;
}

// Once at the red junction, turn around and prepare to turn back around after the home junction
void start_going_home_from_red_box() {
  // Turn until off the line facing towards the central divider
  turn_on_spot(false);
  my_delay(2000);
  // Continue turning until back on the line facing towards the home junction
  while (!any_front_line_sensors_firing()){
    my_milli_delay();
  }
  // Now approaching point just before the green junction when we should turn again
  // Start correspongind super timer
  state.approaching = approachables.just_before_green_junct;
  start_super_timer(ST_lengths.red_junct_to_just_before_green_junct);
}

// incrementing the junction the buggy is near as needed to change approach
void next_approaching_after_junct() {
  Serial.println("Task: Setting the junction now being approached");
  switch (state.approaching) {
    case approachables.green_junct: state.approaching = approachables.home_junct; break;
    case approachables.home_junct: state.approaching = approachables.red_junct; break;
    // Never used as for going via ramp case
    case approachables.red_junct:
      start_super_timer(ST_lengths.red_junct_to_straight_before_ramp);
      state.approaching = approachables.straight_before_ramp;
    break;
  }
}

// drops off the box and returns to the line
void deposit_block() {
  Serial.println("Task: Depositing Block");
  // start recording moves and turn at junct
  state.recording = true;
  turn_on_spot(true);
  my_delay(1150);
  // move into box
  set_motor_dirs(FORWARD);
  set_motor_speeds(255);
  my_delay(1200);
  // stop and raise grabber
  set_motor_speeds(0);
  raise_grabber();
  // return to initial position and stop
  reverse_run();
  // update state of block
  state.block_type = block_types.none;

  // === escape box ===
  // If we're in the red box and time is running low we need to dash home
  if (state.approaching == approachables.red_junct) {
    start_going_home_from_red_box();
  } else {
    // drive past junction
    set_motor_dirs(FORWARD);
    set_motor_speeds(255);
    my_delay(500);
    // return to line
    refind_line();
    next_approaching_after_junct();
  }

}

// looks at the state of the block type and approachable to decide what to do at each junction
void handle_junct() {
  Serial.println("Task: Handling Junction");
  if (state.block_type == block_types.none && state.approaching == approachables.home_junct) {
    go_home();
  } else if (
    (state.block_type == block_types.non_mag && state.approaching == approachables.green_junct)
    || (state.block_type == block_types.mag && state.approaching == approachables.red_junct)
  ) {
    deposit_block();
  } else {
    next_approaching_after_junct();
  }
}

// slows down once the block is within range to then capture block
void aquire_block() {
  Serial.println("Task: Acquiring Block");
  set_motor_speed(false, 220);
  set_motor_speed(true, 255);
  while(get_ultrasonic_distance(true) > 3) { // will approach block until it is within 3cm
    my_milli_delay();
  }
  set_motor_speeds(0);

  // Test if block is magnetic and assigns it
  if (test_if_magnetic()) {
    state.block_type = block_types.mag;
  } else {
    state.block_type = block_types.non_mag;
  }
  //capture block in grabber
  lower_grabber();

  // not used as tunnel was used both times
  // would go up ramp ignoring anomolous sensor readings
  if (GO_VIA_RAMP) {
    start_super_timer(ST_lengths.block_aquired_to_straight_before_tunnel);
  } else {
    // would need to return via tunnel if not using ramp so must turn around
    turn_on_spot(true);
    my_delay(2000);
    while (!any_front_line_sensors_firing()){
      my_milli_delay();
    }
    start_super_timer(ST_lengths.turned_180_to_straight_before_tunnel);
  }
  state.approaching = approachables.straight_before_tunnel;
  
}

// Uses side US to control distance to tunnel wall whilst not detecting the line
void traverse_tunnel() {
  Serial.println("Task: Traversing Tunnel");
  set_motor_dirs(FORWARD);
  set_motor_speeds(255);
  my_delay(500);

  unsigned long timer_end = millis() + 3500;

  // whilst still in tunnel (tuned)
  while (!any_front_line_sensors_firing() && millis() < timer_end) {
    int dist_to_wall = get_ultrasonic_distance(false);
    
    // if too close to wall move away
    if (dist_to_wall < 6) {
      set_motor_speed(false, 255);
      set_motor_speed(true, 220);
    // if too far from wall move closer
    } else if (dist_to_wall > 6) {
      set_motor_speed(true, 255);
      set_motor_speed(false, 220);
    } else {
    // else continue straight
      set_motor_speeds(255);
    }
  }
  // stop regulating speed in this function once reached end of tunnel
  set_motor_speeds(0);

  // find the line at the end of the tunnel
  refind_line();

  // decide where to go depending on block type
  if (state.block_type == block_types.none) {
    state.approaching = approachables.straight_before_block;
    start_super_timer(ST_lengths.tunnel_end_to_straight_before_block);
  } else {
    state.approaching = approachables.straight_before_juncts;
    start_super_timer(ST_lengths.tunnel_end_to_straight_before_juncts);
  }
}

// rotate to scan if line is within the arc of the rover
void refind_line() {
  Serial.println("Task: Refinding Line");
  set_motor_speeds(0);
  unsigned long timer_end = millis() + 600;
  
  //arcs to the right first
  turn_on_spot(true);
  while(millis() < timer_end && !any_front_line_sensors_firing()) {
    my_milli_delay();
  }
  set_motor_speeds(0);
  if (any_front_line_sensors_firing()) { return; }

  timer_end = millis() + 1000;

  // if no line found on right, look left
  turn_on_spot(false);
  while(millis() < timer_end && !any_front_line_sensors_firing()) {
    my_milli_delay();
  }
  set_motor_speeds(0);
}


// Join the main line circuit from the start box
void leave_start() {
  Serial.println("Task: Leaving Start Box");
  // if ramp is chosen then turn right
  if (GO_VIA_RAMP) {
    start_super_timer(ST_lengths.start_to_straight_before_ramp); // TUNE
    state.approaching = approachables.straight_before_ramp;
  // if tunnel is chosen turn left
  } else {
    start_super_timer(ST_lengths.start_to_straight_before_tunnel);
    state.approaching = approachables.straight_before_tunnel;
  }
  //rotate slightly to hit line at angle
  turn_on_spot(GO_VIA_RAMP); // true is turning right
  my_delay(500);

  // go to the line and skip the box
  set_motor_dirs(FORWARD);
  my_delay(1200);

  // carry on until line is found
  while (!any_front_line_sensors_firing()) {
    my_milli_delay();
  }
}

// Ramp code not used as tunnel was chosen:

/*
void traverse_ramp() {
  Serial.println("Task: Traversing Ramp");
  set_motor_speed(false, 255);
  set_motor_speed(true, 255);
  delay(2000);
  //
  myservo.write(45);
  refind_line();
  if (state.blocks_collected == 0) {
    start_super_timer(ST_lengths.7000; //TUNE
    state.approaching = approachables.straight_before_block;
  } else {
    start_super_timer(ST_lengths.15000; //TUNE
    state.approaching = approachables.straight_before_tunnel;
  }
}
*/

/*
void find_block() {
  Serial.println("Task: Finding Block");
  unsigned long time_spent_on_block_straight = millis() - state.time_at_start_of_block_straight;
  state.recording = true;
  turn_on_spot(false);
  delay(1000);
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
      dist = get_ultrasonic_distance(false);
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
  Serial.println("a");
  state.approaching = approachables.straight_before_tunnel;
  start_super_timer(ST_lengths.(TIME_FOR_BLOCK_STRAIGHT - time_spent_on_block_straight); // TUNE
}
*/

