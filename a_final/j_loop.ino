#include <Arduino.h>

// the continuously run loop
void loop() {
  // if the previous super timer has run out of time, the approachable needs to be updated to the new one
  if (millis() >= state.super_timer_end) {
     // depending on the previous approaching, set the new one
    switch (state.approaching) {
      case approachables.just_before_home_junct:
        state.approaching = approachables.home_junct; break;
      case approachables.just_before_green_junct:
        // We're just before the green junction facing left and are on our way
        // home so now need to turn to face right to turn off at the home junction when we reach it.
        // we have to turn off with the home junction on our right as that is the side with the 
        // junction line sensor
        turn_around_and_go_home(); break;
      case approachables.straight_before_ramp:
        state.approaching = approachables.ramp; break;
      case approachables.straight_before_tunnel:
        state.approaching = approachables.tunnel; break;
      case approachables.straight_before_block:
        state.approaching = approachables.block; break;
      case approachables.straight_before_juncts:
// CRITICAL ERROR:  set_motor_speeds(0);
// CRITICAL ERROR:  while(1);                 
        state.approaching = approachables.green_junct; break;
    }
  }
  // if expecting junction, look for junction
  if (is_approaching_junct()) {
    int junct_sensor_val = digitalRead(JUNCT_SENSOR_PIN);
    // if falling edge of junct sensor
    if (state.junct_sensor_val && !junct_sensor_val) {
      handle_junct();
    }
    state.junct_sensor_val = junct_sensor_val;
  } else if (state.approaching == approachables.block) {
    int dist_to_block = get_ultrasonic_distance(true);
    // once under 10cm, acquire block is initiated so that the rover starts adjusting course and slowing slightly
    if (dist_to_block <= 10) {
      aquire_block();
    }
  }

  // line following
  follow_line_step();

  // update amber LED
  flash_amber();

  // === Useful tests used throughout ===
  // Serial.println(digitalRead(BUTTON_PIN));
  // print_sensor_vals();
  // Serial.println(digitalRead(JUNCT_SENSOR_PIN));
}
