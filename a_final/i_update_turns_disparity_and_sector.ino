#include <Arduino.h>

void update_turns_disparity_and_sector(bool approaching_corner) {
  // Calculate and add the new turns disparity to the list
  int newest_disparity = state.motor_speeds[1] - state.motor_speeds[0];
	state.disparities_sample.add(newest_disparity);
	// Only possible when SAMPLE_LENGTH first reached so avg must be computed from scratch
	if (state.disparities_sample.size() == TURN_DISPARITIES_SAMPLE_LENGTH) {
    int tot = 0;
    for (int i = 0; i < state.disparities_sample.size(); i++) {
      tot += state.disparities_sample.get(i);
    }
		state.avg_turns_disparity = tot / TURN_DISPARITIES_SAMPLE_LENGTH;
  } else if (state.disparities_sample.size() > TURN_DISPARITIES_SAMPLE_LENGTH) {
		// Get and remove oldest turns disparity
    int oldest_disparity = state.disparities_sample.pop();
		// Remove contribution of oldest turns disparity from avg
		state.avg_turns_disparity -= oldest_disparity / TURN_DISPARITIES_SAMPLE_LENGTH;
		// Add contribution of newest turns disparity
		state.avg_turns_disparity += newest_disparity / TURN_DISPARITIES_SAMPLE_LENGTH;

    // If turns_disparity high enough,
    // right wheel overdriven enough to be curving left
    if (
        // From straight to curving
        (state.avg_turns_disparity > CURVING_LEFT_THRESHOLD
        && approaching_corner) ||
        // From curving to straight
        (state.avg_turns_disparity < GOING_STRAIGHT_THRESHOLD
        && !approaching_corner)
    ) {
      next_sector();
    }
  }
}