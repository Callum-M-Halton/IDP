#include <Arduino.h>

void loop() {
  if (false) {
    // Sensor checking based on what we're approaching
    if (state.approaching == approachables.corner 
        || state.approaching == approachables.straight) {
      update_turns_disparity_and_sector(
        state.approaching == approachables.corner
      );
    } else if (state.approaching == approachables.junct_on_right) {
      if (digitalRead(JUNCT_SENSOR_PIN)) {
        if (state.sector_code == state.sector_code_to_turn_off_after) {
          make_right_turn();
        }
      }
    }

    // line following 
    follow_line_step();

    // time checking
    if (state.sector_code_to_turn_off_after == -1 
        && millis() > TIME_BEFORE_HEADING_BACK) {
      // head home at the next opportunity, sector_code 12 is "straight_before_start_junct"
      state.sector_code_to_turn_off_after = 
    }
  }
  //print_sensor_vals();
  //Serial.println(digitalRead(JUNCT_SENSOR_PIN));
  //print_mode(state.mode);
}
