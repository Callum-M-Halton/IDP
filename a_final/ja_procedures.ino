#include <Arduino.h>


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

void reverse_run(bool ignore_sensors=false) {
  state.time_stamp_of_cmd_being_rev_run = millis();
  state.timer_end = millis();
  //start_time = millis();
  while (ignore_sensors || !any_front_line_sensors_firing()) {
    Serial.println("rev");
    if (millis() >= state.timer_end) {
      if (state.motor_cmds.size() == 0) {
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
      my_delay(1);
    }
  }
  set_motor_speeds(0);
}

void go_home() {
  Serial.println("Task: Going Home");
  set_motor_dirs(FORWARD);
  set_motor_speeds(255);
  my_delay(1000);
  set_motor_speeds(0);
  while (1);
}

void find_block() {
  unsigned long time_spent_on_block_straight = millis() - state.time_at_start_of_block_straight;
  add_motor_cmd(true);
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

  if (true/*test_if_magnetic()*/) {
    state.block = block_types.mag;
  } else {
    state.block = block_types.non_mag;
  }
  lower_grabber();

  reverse_run();
  refind_line();
  state.approaching = approachables.straight_before_tunnel;
  state.super_timer_end = millis() + (10000 - time_spent_on_block_straight); // TUNE
}

void next_junct() {
  switch (state.approaching) {
    case approachables.green_junct: state.approaching = approachables.home_junct; break;
    case approachables.home_junct: state.approaching = approachables.red_junct; break;
    case approachables.red_junct: state.approaching = approachables.straight_before_ramp; break;      
  }
}

void deposit_block() {
  Serial.println("Task: Depositing Block");
  // drop off block
  set_motor_dirs(FORWARD);
  set_motor_speeds(255);
  my_delay(500);
  raise_grabber();
  state.block = block_types.none;
  state.blocks_collected++;
  set_motor_dirs(BACKWARD);
  my_delay(500);
  // escape box
  turn_on_spot(false);
  my_delay(1500);
  set_motor_dirs(FORWARD);
  set_motor_speeds(255);
  my_delay(1200);
  // rejoin line
  while (!any_front_line_sensors_firing()) {
    my_milli_delay();
  }
  next_junct();
}

void make_right_turn() {
  Serial.println("Task: Making Right Turn");
  turn_on_spot(true);
  my_delay(1000); // TUNE
  /*
  switch (state.approaching) {
    case approachables.red_junct: state.approaching = approachables.red_box; break;
    case approachables.green_junct: state.approaching = approachables.green_box; break;
    case approachables.home_junct: state.approaching = approachables.home_box; break;
  }
  */
  if (state.approaching == approachables.home_junct) {
    go_home();
  } else {
    deposit_block();
  }
}

void handle_junct() {
  if (
       (state.block = block_types.mag && state.approaching == approachables.green_junct)
    || (state.block = block_types.non_mag && state.approaching == approachables.red_junct)
    || (state.block = block_types.none && state.approaching == approachables.home_junct 
        /* && millis() > state.start_time + 240000 */)
  ) {
    make_right_turn();
  } else {
    next_junct();
  }
}

void aquire_block() {
  Serial.println("Task: Acquiring Block");
  set_motor_speed(false, 200);
  set_motor_speed(true, 255);
  while(get_ultrasonic_distance(true) > 3) {
    my_milli_delay();
  }
  set_motor_speeds(0);
  // state.speed_coeff = 1.0;
  
  if (true/*test_if_magnetic()*/) {
    state.block = block_types.mag;
  } else {
    state.block = block_types.non_mag;
  }
  lower_grabber();

  //
  while(1);

  if (GO_VIA_RAMP) {
    state.super_timer_end = millis() + 7000; // TUNE
  } else {
    turn_on_spot(true);
    my_delay(200);
    while (!any_front_line_sensors_firing()){
      my_milli_delay();
    }
    state.super_timer_end = millis() + 7000; // TUNE
  }
  state.approaching = approachables.straight_before_tunnel;
  
}

void traverse_ramp() {
  set_motor_speed(false, 255);
  set_motor_speed(true, 150);
  delay(2000);
  //
  myservo.write(45);
  refind_line();
  if (state.blocks_collected == 0) {
    state.super_timer_end = millis() + 7000; //TUNE
    state.approaching = approachables.straight_before_block;
  } else {
    state.super_timer_end = millis() + 7000; //TUNE
    state.approaching = approachables.block_straight;
  }
  
}

void traverse_tunnel() {
  Serial.println("Task: Traversing Tunnel");
  set_motor_dirs(FORWARD);
  /*
  unsigned long timer_end = millis() + 3500;
  while (!any_front_line_sensors_firing() && millis < timer_end) {
    int dist_to_wall = get_ultrasonic_distance(side_US_pins);
    if (dist_to_wall < 9) {
      set_motor_speed(false, 255);
      set_motor_speed(true, 200);
    } else (dist_to_wall > 11) {
      set_motor_speed(true, 255);
      set_motor_speed(false, 200);
    } else {
      set_motor_speeds(255);
    }
  }
  */
  set_motor_speeds(255);//
  my_delay(3500);//
  refind_line();
  if (state.block == block_types.none) {
    state.approaching = approachables.straight_before_block;
    state.super_timer_end = millis() + 7100;
  } else {
    state.approaching = approachables.straight_before_juncts;
    state.super_timer_end = millis() + 5000; // TUUUUUUNE
  }
}

void refind_line() {
  Serial.println("Task: Refinding Line");
  unsigned long timer_end = millis() + 300;
  turn_on_spot(true);
  while(millis() < timer_end && !any_front_line_sensors_firing()) {
    my_milli_delay();
  }
  set_motor_speeds(0);
  if (any_front_line_sensors_firing()) { return; }

  timer_end = millis() + 600;
  turn_on_spot(false);
  while(millis() < timer_end && !any_front_line_sensors_firing()) {
    my_milli_delay();
  }
  set_motor_speeds(0);
}

void leave_start() {
  Serial.println("Task: Leaving Start Box");
  if (GO_VIA_RAMP) {
    state.super_timer_end = millis() + 9300; // TUNE
    state.approaching = approachables.straight_before_ramp;
  } else {
    state.super_timer_end = millis() + 9300;
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
