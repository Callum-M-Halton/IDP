#include <Arduino.h>

void loop() {
  if (true) {
    // Check if any of the things being approached have been reached
    /*
    if (state.approaching == approachables.corner 
        || state.approaching == approachables.straight) {
      update_turns_disparity_and_sector(
        state.approaching == approachables.corner
      );
    }
    */
    // Execute mode functionality depending on state.mode
    switch (state.mode) {
      case modes.finding_line_at_start:
        leave_start();
      break;
      case modes.following_line:
        follow_line_step();
      break;
      case modes.refinding_line:
        refind_line_step();
      break;
      case modes.traversing_tunnel:
        traverse_tunnel();
      break;
    }
  }
  if (millis() >= start_millis + 5000) {
    set_mode(modes.lowering_grabber);
    turn_on_spot(false);
    set_motor_dirs(FORWARD);
    set_motor_speeds(255);
    delay(1000);
    turn_on_spot(true);
    set_motor_dirs(FORWARD);
    set_motor_speeds(255);
    delay(1000);
    set_mode(modes.refinding_line);
    start_millis = 60000;
  }
  //set_motor_speeds(255);
  //set_motor_dirs(FORWARD);
  //print_sensor_vals();
  //Serial.println(digitalRead(JUNCT_SENSOR_PIN));
  //print_mode(state.mode);
}
