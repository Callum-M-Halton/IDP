#include <Arduino.h>

bool any_front_line_sensors_firing() {
  return digitalRead(front_sensor_pins.left) 
    || digitalRead(front_sensor_pins.mid)
    || digitalRead(front_sensor_pins.right);
}

void set_motor_dirs(int dir) {
  L_motor->run(dir);
  R_motor->run(dir);
}

void set_motor_speed(bool is_right, int speed) {
  speed *= state.speed_coeff;
  // underdrives right motor to match left motor
  if (is_right) {
    speed *= 0.9;
  }
	if (state.motor_speeds[int(is_right)] != speed) {
    state.motor_speeds[int(is_right)] = speed;
    if (is_right) {
      R_motor -> setSpeed(speed);
    } else {
      L_motor -> setSpeed(speed);
    }
    motor_cmd_struct motor_cmd = {is_right, speed, millis()};
    state.motor_cmds.add(motor_cmd);
    if (state.motor_cmds.size() > MOTOR_CMDS_SIZE) {
      state.motor_cmds.shift();
    }
	}
  if (speed > 0){
    digitalWrite(AMBER_LED_PIN,HIGH)
  }
}

void set_motor_speeds(int speed) {
  set_motor_speed(false, speed);
  set_motor_speed(true, speed);
}

void print_sensor_vals() {
  Serial.print(digitalRead(front_sensor_pins.left));
  Serial.print(digitalRead(front_sensor_pins.mid));
  Serial.println(digitalRead(front_sensor_pins.right));
}

void lowering_grabber(){
    myservo.write(DROP_GRABBER_VALUE);
}

void raising_grabber(){
    myservo.write(RAISE_GRABBER_VALUE);
}