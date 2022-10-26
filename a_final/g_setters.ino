#include <Arduino.h>
/*
void set_sector(int sector_code) {
  state.sector_code = sector_code;
  sector_struct sector = sectors[sector_code];
  Serial.println("Sector set to: " + sector.name);
  state.approaching = sector.approaching;
  switch (sector_code) {
    case 3: state.speed_coeff *= 1.0; break; // tunnel
    case 8: state.speed_coeff *= 1.0; break; // ramp
  }
}

void next_sector() {
  set_sector((state.sector_code + 1) % num_of_sectors);
}
*/