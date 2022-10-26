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
      int junct_sensor_val = digitalRead(JUNCT_SENSOR_PIN);
      // if falling edge of junct sensor
      if (state.junct_sensor_val && !junct_sensor_val) {
        if (state.sector_code == state.sector_code_to_turn_off_after) {
          make_right_turn();
        } else {
          next_sector();
        }
      }
      state.junct_sensor_val = junct_sensor_val;
    } else if (state.approaching == approachables.block) {
      // Slow down when distance is 10cm
      int dist_to_block = get_ultrasonic_distance(front_US_pins);
      if (dist_to_block <= 3) {
        aquire_block();
      } else if (dist_to_block < 10) {
        state.speed_coeff = BOX_APPROACH_COEFF;
      }     
    }

    // line following 
    follow_line_step();

    // time checking
    if (state.sector_code_to_turn_off_after == -1 
        && millis() > TIME_BEFORE_HEADING_BACK) {
      // head home at the next opportunity, sector_code 12 is "straight_before_start_junct"
      state.sector_code_to_turn_off_after = 12;
    }
  }
  //print_sensor_vals();
  //Serial.println(digitalRead(JUNCT_SENSOR_PIN));
  //print_mode(state.mode);
}
