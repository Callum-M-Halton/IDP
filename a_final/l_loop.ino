#include <Arduino.h>

void loop() {
  if (millis() >= state.super_timer_end) {
    switch (state.approaching) {
      case approachables.just_before_home_junct:
        state.approaching = approachables.home_junct; break;
      case approachables.just_before_green_junct:
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
  if (is_approaching_junct()) {
    int junct_sensor_val = digitalRead(JUNCT_SENSOR_PIN);
    // if falling edge of junct sensor
    if (state.junct_sensor_val && !junct_sensor_val) {
      handle_junct();
    }
    state.junct_sensor_val = junct_sensor_val;
  } else if (state.approaching == approachables.block) {
    // Slow down when distance is 10cm
    int dist_to_block = get_ultrasonic_distance(true);
    if (dist_to_block <= 10) {
      aquire_block();
    }
  }

  // line following
  follow_line_step();

  // update amber LED
  flash_amber();

  // Useful Tests
  //Serial.println(digitalRead(BUTTON_PIN));
  //print_sensor_vals();
  //Serial.println(digitalRead(JUNCT_SENSOR_PIN));
}

/*
        case approachables.block_straight:
          if (millis() > state.start_time + 240000) {
            start_super_timer(ST_lengths.TIME_FOR_BLOCK_STRAIGHT;
            state.approaching = approachables.straight_before_tunnel; 
          } else {
            state.time_at_start_of_block_straight = millis();
            state.approaching = approachables.block_to_left;           
          }
        break;
*/

/*else if (state.approaching == approachables.block_to_left) {
      int dist = get_ultrasonic_distance(false);
      if (state.last_side_dist != -1) {
        if (state.last_side_dist - dist > 10) { //TUNE/measure
          state.last_side_dist != -1;
          //
          set_motor_speeds(0);
          while(1);
          find_block();
        }
      }
      state.last_side_dist = dist;
    }*/

    /*
    state.cycle_num += 1;
    // Sensor checking based on what we're approaching
    if (
      (state.approaching == approachables.corner || state.approaching == approachables.straight)
      && (state.cycle_num % DISPARITY_SAMPLE_PERIOD == 0)
    ) {
      update_turns_disparity_and_sector(
        
        state.approaching == approachables.corner
      );
    } else if (state.approaching == approachables.junct_on_right) {
      int junct_sensor_val = digitalRead(JUNCT_SENSOR_PIN);
      // if falling edge of junct sensor
      if (state.junct_sensor_val && !junct_sensor_val) {
        Serial.println("Aaaaaaaaa");
        *
        if (state.sector_code == state.sector_code_to_turn_off_after) {
          make_right_turn();
        } else {
          next_sector();
        }
        *
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
    */

   /*
    // time checking
    if (state.sector_code_to_turn_off_after == -1 
        && millis() > TIME_BEFORE_HEADING_BACK) {
      // head home at the next opportunity, sector_code 12 is "straight_before_start_junct"
      state.sector_code_to_turn_off_after = 12;
    }
  }
*/
