#include <Arduino.h>


// RAMP NOT USED so has been commented out
/*
void handle_EOL() {
  switch (state.approaching) {
    case approachables.ramp: traverse_ramp(); break;
    case approachables.tunnel: traverse_tunnel(); break;
  }
}*/

// Follows line unless the approachable indicates it shouldn't
void follow_line_step() {
  ///// of what?! also "estimates" implies we're going to get a probability float, make clear it's an enum
  // estimates the liklihood from the sensor readings in correct trajectory
  int line_end_likelihood = correct_trajectory();

  // if it is likely the line has ended consider each case in turn
  if (line_end_likelihood != line_end_likelihoods.none) {
    // if we're expecting the tunnel then line ending is even more likely so immediately switch to tunnel traversal
    if (state.approaching == approachables.tunnel) {
      traverse_tunnel();
    } else if (line_end_likelihood == line_end_likelihoods.as_before) {
      // if the timer is valid (not set to the null state of 0)...
      // ... and it has run out, assume buggy off line
      if (state.timer_end != 0 && millis() >= state.timer_end) {
        refind_line();
      }
    // Gone from [0, 1, 0] to [0, 0, 0]
    } else if (line_end_likelihood == line_end_likelihoods.high) {
      Serial.println("Starting left line timer");
      // set timer
      state.timer_end = millis() + LOST_LINE_TIMER_LENGTH;
    // Otherwise not [0, 1, 0] to [0, 0, 0]...
    } else {
      // ... so set a timer_end to the null state 0, which will never run out
      state.timer_end = 0;
    }
  }
}
