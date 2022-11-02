#include <Arduino.h>
/*
void handle_EOL() {
  switch (state.approaching) {
    case approachables.ramp: traverse_ramp(); break;
    case approachables.tunnel: traverse_tunnel(); break;
  }
}*/

void follow_line_step() {
  int line_end_likelihood = correct_trajectory();
  if (line_end_likelihood != line_end_likelihoods.none) {
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
      state.timer_end = millis() + 500;
    // Otherwise not [0, 1, 0] to [0, 0, 0]...
    } else {
      // ... so set a timer_end to the null state 0, which will never run out
      state.timer_end = 0;
    }
  }
}
