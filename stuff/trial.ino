// Line Follow if in a line_following mode
	if state.mode in [approaching_symmetric_junct, basic, approaching_block_on_line, approaching_tunnel, 
			approaching_right_turn_to_take, approaching_box]:
		line_end_likelihood = correct_trajectory()
		if line_end_likelihood = line_end_likelihood.none:
			// a value of -1 indicates no countdown as we're still on the line
			state.count_down = -1
		// if as_before, a count_down will already have been set so...
		else if line_end_likelihood == line_end_likelihoods.as_before:
			// decrement it towards 0 (where it runs out)
			state.count_down -= 1
			// if the timer has run out assume end of line reached and take appropriate action
			if state.count_down = 0:
				if state.mode in [approaching_box, approaching_symmetric_junct, approaching_tunnel]:
					// update sector
				// Otherwise we're not expecting an end of line so we've actually LOST THE LINE!!!
				else:
					// we have lost the line
					state.prev_mode = state.mode
					state.mode = modes.lost_line
		// We have just moved off of the line and need to set a timer to wait to determine if end of line reached
		else:
			// The higher the likelihood the lower the count_down set 
			// e.g. state.count_down = 1 for line_end_likelihoods.high
			state.count_down = count_downs_by_line_end_likelihood[line_end_likelihood]
		
			
	// FIGURE OUT IF WE'RE CURVING LEFT
	update_curving_left()