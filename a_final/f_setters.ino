#include <Arduino.h>

void set_sector(int sector_code) {
  state.sector_code = sector_code;
  on_loop_sector_struct sector = on_loop_sectors[sector_code];
  Serial.println("Sector set to: " + sector.name);
  state.approaching = sector.approaching;
  state.speed_coeff = sector.speed_coeff;
  set_mode(sector.associated_mode);
}

void next_on_loop_sector() {
  set_sector((state.sector_code + 1) % num_of_on_loop_sectors);
}

void set_mode(int mode) {
  switch(mode) {
    case modes.following_line:
      state.offset_dir = offset_dirs.none;
      state.offset_ext = offset_exts.none;
      set_motor_speed(false, speeds.high);
      set_motor_speed(true, speeds.high);
      set_motor_dirs(FORWARD);
    break;
    case modes.refinding_line:
      digitalWrite(ERROR_LED_PIN, HIGH);
      set_motor_dirs(BACKWARD);
      Serial.println("lossssst liiiiiine!!!!!!!!");
    break;
  }
  Serial.println("Mode set to: " + mode_strings[mode]);
  state.mode = mode;
}