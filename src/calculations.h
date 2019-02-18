#ifndef _CALCULATIONS_H
#define _CALCULATIONS_H

struct fueling_config {
  float injector_cc_per_minute;
  float cylinder_cc;
  float fuel_stoich_ratio; 
  unsigned int injections_per_cycle; /* fuel quantity per shot divisor */
  unsigned int fuel_pump_pin;

  /* Constants */
  float density_of_air_stp; /* g/cc at 0C 100 kpa */
  float density_of_fuel; /* At ? C */

  /* Intermediate values for debugging below */
  float airmass_per_cycle;
  float fuelvol_per_cycle;
  float injector_dead_time;
};

typedef enum {
  DWELL_FIXED_DUTY,
  DWELL_FIXED_TIME,
} dwell_type;

struct ignition_config {
  dwell_type dwell;
  float dwell_duty;
  float dwell_us;
  int min_fire_time_us;
};

struct calculated_values {
  float timing_advance;
  unsigned int fueling_us;
  unsigned int dwell_us;
  int rpm_limit_cut;
};

extern struct calculated_values calculated_values;

void calculate_ignition();
void calculate_fueling();
int ignition_cut();
int fuel_cut();

#endif
