#ifndef _CALCULATIONS_H
#define _CALCULATIONS_H

struct fueling_config {
  float injector_cc_per_minute;
  float cylinder_cc;
  float fuel_stoich_ratio; 
  uint8_t injections_per_cycle; /* fuel quantity per shot divisor */
  uint8_t fuel_pump_pin;

  /* Constants */
  float density_of_air_stp; /* g/cc at 0C 100 kpa */
  float density_of_fuel; /* At 15 C */

  /* Intermediate values for debugging below */
};

typedef enum {
  DWELL_FIXED_DUTY,
  DWELL_FIXED_TIME,
} dwell_type;

struct ignition_config {
  dwell_type dwell;
  float dwell_duty;
  float dwell_us;
  uint32_t min_fire_time_us;
};

struct calculated_values {
  /* Ignition */
  float timing_advance;
  unsigned int dwell_us;
  rpm_t rpm_limit_cut;

  /* Fueling */
  uint32_t fueling_us;
  float airmass_per_cycle;
  float fuelvol_per_cycle;
  float idt;
  float lambda;
  float ve;
};

extern struct calculated_values calculated_values;

void calculate_ignition();
void calculate_fueling();
bool ignition_cut();
bool fuel_cut();

#ifdef UNITTEST
#include <check.h>
TCase *setup_calculations_tests();
#endif

#endif
