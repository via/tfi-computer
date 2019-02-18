#include "config.h"
#include "calculations.h"
#include "stats.h"
struct calculated_values calculated_values;

int ignition_cut() {
  if (config.decoder.rpm >= config.rpm_stop) {
    calculated_values.rpm_limit_cut = 1;
  }
  if (config.decoder.rpm < config.rpm_start) {
    calculated_values.rpm_limit_cut = 0;
  }
  return calculated_values.rpm_limit_cut;
}

int fuel_cut() {
  return ignition_cut();
}

void calculate_ignition() {
  calculated_values.timing_advance = 
    interpolate_table_twoaxis(config.timing, config.decoder.rpm, 
        config.sensors[SENSOR_MAP].processed_value);
  switch (config.ignition.dwell) {
    case DWELL_FIXED_DUTY:
      calculated_values.dwell_us = 
        time_from_rpm_diff(config.decoder.rpm, 45) / (TICKRATE / 1000000);
      break;
    case DWELL_FIXED_TIME:
      calculated_values.dwell_us = config.ignition.dwell_us;
      break;
  }
}

static float air_density(float iat_celsius, float atmos_kpa) {
  const float kelvin_offset = 273.15;
  float temp_factor =  kelvin_offset / (iat_celsius + kelvin_offset);
  return (atmos_kpa / 100) * config.fueling.density_of_air_stp * temp_factor;
}

static float fuel_density(float fuel_celsius) {
  const float beta = 950.0; /* Gasoline 10^-6/K */
  float delta_temp = fuel_celsius - 15.0;
  return config.fueling.density_of_fuel - (delta_temp * beta / 1000000.0);
}

void calculate_fueling() {
  stats_start_timing(STATS_FUELCALC_TIME);
  float ve;
  float lambda;
  float idt;
  float atmos_kpa = config.sensors[SENSOR_AAP].processed_value;
  float fueltemp = config.sensors[SENSOR_FRT].processed_value;
  float iat = config.sensors[SENSOR_IAT].processed_value;
  float map = config.sensors[SENSOR_MAP].processed_value;
  float brv = config.sensors[SENSOR_BRV].processed_value;

  if (config.ve) {
    ve = interpolate_table_twoaxis(config.ve, config.decoder.rpm, map);
  } else {
    ve = 100.0;
  }

  if (config.commanded_lambda) {
    lambda = interpolate_table_twoaxis(config.commanded_lambda, 
      config.decoder.rpm, map);
  } else {
    lambda = 1.0;
  }

  if (config.injector_pw_compensation) {
    idt = interpolate_table_oneaxis(config.injector_pw_compensation, brv);
  } else {
    idt = 1000;
  }

  float injested_volume_per_cycle = (ve / 100.0) * 
    (map / atmos_kpa) * 
    config.fueling.cylinder_cc;

  float injested_mass_per_cycle = injested_volume_per_cycle *
    air_density(iat, atmos_kpa);
  config.fueling.airmass_per_cycle = injested_mass_per_cycle;

  float required_fuelvolume_per_cycle = injested_mass_per_cycle / 
    config.fueling.fuel_stoich_ratio / fuel_density(fueltemp) / lambda;
  config.fueling.fuelvol_per_cycle = required_fuelvolume_per_cycle;

  float raw_pw_us = required_fuelvolume_per_cycle / 
    config.fueling.injector_cc_per_minute * 60000000 / /* uS per minute */
    config.fueling.injections_per_cycle; /* This many pulses */
  
  config.fueling.injector_dead_time = idt;

  calculated_values.fueling_us = raw_pw_us + idt;
  stats_finish_timing(STATS_FUELCALC_TIME);
}

#ifdef UNITTEST
#include <check.h>

#include "check_platform.h"

START_TEST(check_air_density) {

  /* Confirm STP */
  ck_assert_float_eq_tol(air_density(0.0, 100), 1.2754e-3, 0.000001);

  ck_assert_float_eq_tol(air_density(20, 101.325), 1.2041e-3, 0.000001);
} END_TEST

START_TEST(check_fuel_density) {
  ck_assert_float_eq_tol(fuel_density(15), 0.755, 0.001);
  ck_assert_float_eq_tol(fuel_density(50), 0.721, 0.001);
} END_TEST

TCase *setup_calculations_tests() {
  TCase *tc = tcase_create("calculations");
  tcase_add_test(tc, check_air_density);
  tcase_add_test(tc, check_fuel_density);

  return tc;
}
#endif

