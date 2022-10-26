#include <Arduino.h>

bool any_front_line_sensors_firing() {
  return digitalRead(front_sensor_pins.left) 
    || digitalRead(front_sensor_pins.mid)
    || digitalRead(front_sensor_pins.right);
}

/*
void add_motor_cmd(bool is_flag = false) {
  motor_cmd_struct motor_cmd = {
    {state.motor_dirs[0], state.motor_dirs[1]},
    {state.motor_speeds[0], state.motor_speeds[1]},
    millis(),
    is_flag
  };
  state.motor_cmds.add(motor_cmd);
  if (state.motor_cmds.size() > MOTOR_CMDS_SIZE) {
    state.motor_cmds.shift();
  }
}
*/

void set_motor_dir(bool is_right, int dir) {
  if (state.motor_dirs[int(is_right)] != dir) {
    state.motor_dirs[int(is_right)] = dir;
    if (is_right) {
      R_motor->run(dir);
    } else {
      L_motor->run(dir);
    }
  }
}

void set_motor_dirs(int dir) {
  set_motor_dir(false, dir);
  set_motor_dir(true, dir);
}

void set_motor_speed(bool is_right, int speed, bool record = true) {
  speed *= state.speed_coeff;
  // underdrives right motor to match left motor
  if (state.motor_dirs[0] == BACKWARD && state.motor_dirs[1] == BACKWARD) {
    if (is_right) { speed *= 0.95; }
  } else if (is_right) { speed *= 0.9; }
	if (state.motor_speeds[int(is_right)] != speed) {
    state.motor_speeds[int(is_right)] = speed;
    if (is_right) {
      R_motor -> setSpeed(speed);
    } else {
      L_motor -> setSpeed(speed);
    }
	}
  if (state.motor_speeds[0] == 0 && state.motor_speeds[1] == 0) {
    digitalWrite(AMBER_LED_PIN, HIGH);
  } else {
    // figure out flashing in hardware?
    digitalWrite(AMBER_LED_PIN, HIGH);
  }
}

void set_motor_speeds(int speed) {
  set_motor_speed(false, speed);
  set_motor_speed(true, speed);
}

/*
bool test_if_magnetic() {
  Serial.println("Testing block with hall sensor");
  int tot = 0;
  for(int i = 0; i < HALL_EFFECT_SAMPLE_LENGTH; i++) {
      tot += analogRead(HALL_SENSOR_PIN);
  } 
  int avg = tot / HALL_EFFECT_SAMPLE_LENGTH;
  
  if(avg > HALL_SENSOR_THRESHOLD) {
    //turn on red light for 5 sec if magnetic
    digitalWrite(RED_LED_PIN, HIGH);
    delay(5000);
    digitalWrite(RED_LED_PIN, LOW);
    // box is magnetic
    return true;
  } else {
    //turn on green light for 5 sec if not magnetic
    digitalWrite(GREEN_LED_PIN, LOW);
    delay(5000);
    digitalWrite(GREEN_LED_PIN, LOW);
    //box is not magnetic
    return false;
  }
}
*/

void print_sensor_vals() {
  Serial.print(digitalRead(front_sensor_pins.left));
  Serial.print(digitalRead(front_sensor_pins.mid));
  Serial.println(digitalRead(front_sensor_pins.right));
}

void lower_grabber() {
    myservo.write(DROP_GRABBER_VALUE);
}

void raise_grabber() {
    myservo.write(RAISE_GRABBER_VALUE);
}

void turn_on_spot(bool to_right) {
    if (to_right){
        // set direction to turn right
        set_motor_dir(false, FORWARD);
        set_motor_dir(true, BACKWARD);
    } else {
        // set direction to turn left
        set_motor_dir(true, FORWARD);
        set_motor_dir(false, BACKWARD);
    }
    
    set_motor_speed(true, 255); //right motor
    set_motor_speed(false, 255); //left motor
}

/* ============= bits ==============

, bool record = true
if (record) { add_motor_cmd(); } x 2

*/