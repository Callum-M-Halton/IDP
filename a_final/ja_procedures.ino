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

void reverse_run(bool ignore_sensors=false) {
  Serial.println("Task: Reverse Running");
  state.recording = false;
  state.time_stamp_of_cmd_being_rev_run = millis();
  state.timer_end = millis();
  while (ignore_sensors || !any_front_line_sensors_firing()) {
    if (millis() >= state.timer_end) {
      if (state.motor_cmds.size() == 0) {
        break;
      }
      motor_cmd_struct last_cmd = state.motor_cmds.pop();
      set_motor_dir(false, 3 - last_cmd.dirs[0]);
      set_motor_dir(true, 3 - last_cmd.dirs[1]);
      set_motor_speed(false, last_cmd.speeds[0], true);
      set_motor_speed(true, last_cmd.speeds[1], true);

      unsigned long timer_length = state.time_stamp_of_cmd_being_rev_run - last_cmd.time_stamp;

      state.timer_end = millis() + timer_length;
      state.time_stamp_of_cmd_being_rev_run = last_cmd.time_stamp;
    } else {
      my_delay(1);
    }
  }
  set_motor_speeds(0);
}

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

void turn_around_and_go_home() {
  turn_on_spot(true);
  my_delay(2000);
  while (!any_front_line_sensors_firing()){
    my_milli_delay();
  }
  set_motor_dirs(0);
  start_super_timer(ST_lengths.just_after_green_junct_to_just_before_home_junct);
  state.approaching = approachables.just_before_home_junct;
}

void go_home_from_red_box() {
  state.block_type = block_types.none;
  // return to initial position and stop
  reverse_run(true);

  turn_on_spot(false);
  my_delay(2000);
  while (!any_front_line_sensors_firing()){
    my_milli_delay();
  }
  state.approaching = approachables.just_before_green_junct;
  start_super_timer(ST_lengths.red_junct_to_just_before_green_junct);
}

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

  // === escape box ===
  // If we're in the red box and time is running low we need to dash home
  if (state.approaching == approachables.red_junct) {
    go_home_from_red_box();
  } else {
    // update state of block collection
    state.block_type = block_types.none;
    // return to initial position and stop
    reverse_run(true);
    // drive past junction
    set_motor_dirs(FORWARD);
    set_motor_speeds(255);
    my_delay(500);
    // return to line
    refind_line();
    next_approaching_after_junct();
  }

}

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

void aquire_block() {
  Serial.println("Task: Acquiring Block");
  set_motor_speed(false, 220);
  set_motor_speed(true, 255);
  while(get_ultrasonic_distance(true) > 3) {
    my_milli_delay();
  }
  set_motor_speeds(0);
  
  if (test_if_magnetic()) {
    state.block_type = block_types.mag;
  } else {
    state.block_type = block_types.non_mag;
  }
  lower_grabber();

  if (GO_VIA_RAMP) {
    start_super_timer(ST_lengths.block_aquired_to_straight_before_tunnel);
  } else {
    turn_on_spot(true);
    my_delay(2000);
    while (!any_front_line_sensors_firing()){
      my_milli_delay();
    }
    start_super_timer(ST_lengths.turned_180_to_straight_before_tunnel);
  }
  state.approaching = approachables.straight_before_tunnel;
  
}

void traverse_tunnel() {
  Serial.println("Task: Traversing Tunnel");
  set_motor_dirs(FORWARD);
  set_motor_speeds(255);
  my_delay(500);

  unsigned long timer_end = millis() + 3500;
  while (!any_front_line_sensors_firing() && millis() < timer_end) {
    int dist_to_wall = get_ultrasonic_distance(false);
    if (dist_to_wall < 6) {
      set_motor_speed(false, 255);
      set_motor_speed(true, 220);
    } else if (dist_to_wall > 6) {
      set_motor_speed(true, 255);
      set_motor_speed(false, 220);
    } else {
      set_motor_speeds(255);
    }
  }
  set_motor_speeds(0);

  refind_line();
  if (state.block_type == block_types.none) {
    state.approaching = approachables.straight_before_block;
    start_super_timer(ST_lengths.tunnel_end_to_straight_before_block);
  } else {
    state.approaching = approachables.straight_before_juncts;
    start_super_timer(ST_lengths.tunnel_end_to_straight_before_juncts);
  }
}

void refind_line() {
  Serial.println("Task: Refinding Line");
  set_motor_speeds(0);
  unsigned long timer_end = millis() + 600;
  turn_on_spot(true);
  while(millis() < timer_end && !any_front_line_sensors_firing()) {
    my_milli_delay();
  }
  set_motor_speeds(0);
  if (any_front_line_sensors_firing()) { return; }

  timer_end = millis() + 1000;
  turn_on_spot(false);
  while(millis() < timer_end && !any_front_line_sensors_firing()) {
    my_milli_delay();
  }
  set_motor_speeds(0);
}

void leave_start() {
  Serial.println("Task: Leaving Start Box");
  if (GO_VIA_RAMP) {
    start_super_timer(ST_lengths.start_to_straight_before_ramp); // TUNE
    state.approaching = approachables.straight_before_ramp;
  } else {
    start_super_timer(ST_lengths.start_to_straight_before_tunnel);
    state.approaching = approachables.straight_before_tunnel;
  }
  //rotate slightly to hit line at angle
  turn_on_spot(GO_VIA_RAMP);
  my_delay(500);

  // go to the line and skip the box
  set_motor_dirs(FORWARD);
  my_delay(1200);

  while (!any_front_line_sensors_firing()) {
    my_milli_delay();
  }
}

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

/*
void leave_start() {
  Serial.println("Task: Leaving Start");
  set_motor_dirs(FORWARD);
  set_motor_speeds(255);
  my_delay(TIME_TO_DRIVE_FORWARD_FOR_AT_START);

  // reverse until the side sensor detects the line
  set_motor_dirs(BACKWARD);
  set_motor_speeds(255);
  bool not_yet = true
  while (!digitalRead(JUNCT_SENSOR_PIN)) {
    my_delay(Microseconds(1);
  }
  
  // turns right until line detected
  // set direction to turn right
  set_motor_dir(false, FORWARD);
  set_motor_dir(true, BACKWARD);

  set_motor_speeds(255);
  // while no front sensors are firing
  while (!any_front_line_sensors_firing()){
    my_delay(Microseconds(1);
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
    my_delay(Microseconds(1);
  }

  // perturb right if sector is ramp_straight otherwise perturb left
  bool perturb_right = state.sector_code == 3; // ramp straight code
  int dirs[2] = {FORWARD, BACKWARD};
  set_motor_dir(false, dirs[int(!perturb_right)], false);
  set_motor_dir(true, dirs[int(perturb_right)], false);
  set_motor_speeds(255, false);
  while (any_front_line_sensors_firing()) {
    my_delay(Microseconds(1);
  }
  set_motor_dir(false, dirs[int(perturb_right)], false);
  set_motor_dir(true, dirs[int(!perturb_right)], false);
  while (!any_front_line_sensors_firing()) {
    my_delay(Microseconds(1);
  }
  set_motor_speeds(0, false);
  digitalWrite(ERROR_LED_PIN, LOW);
}*/


/* ======= bits ======
  // adds a flag to the motor cmds history so we can reverse up until this point later
  add_motor_cmd(true // true indicates to add a flag command);

  reverse_run(true);
  set_motor_dirs(BACKWARD);
  set_motor_speeds(255);
  my_delay(200);
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
/*
      turn_on_spot(false);
    my_delay(1500);
    set_motor_dirs(FORWARD);
    set_motor_speeds(255);
    my_delay(1200);
    //
    set_motor_speeds(0);
    while(1)
    // rejoin line
    while (!any_front_line_sensors_firing()) {
      my_milli_delay();
    }
    */
