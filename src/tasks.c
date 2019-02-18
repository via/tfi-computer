#include "platform.h"
#include "config.h"
#include "decoder.h"
#include "util.h"
#include "tasks.h"

void handle_fuel_pump() {
  static timeval_t last_valid = 0;

  /* If engine is turning, keep pump on */
  if ((config.decoder.state == DECODER_RPM) ||
      (config.decoder.state == DECODER_SYNC)) {
    last_valid = current_time();
    set_gpio(config.fueling.fuel_pump_pin, 1);
    return;
  }

  /* Allow 4 seconds of fueling */
  if (time_diff(current_time(), last_valid) < time_from_us(4000000)) {
    set_gpio(config.fueling.fuel_pump_pin, 1);
  } else {
    set_gpio(config.fueling.fuel_pump_pin, 0);
    /* Keep last valid 5 seconds behind to prevent rollover bugs */
    last_valid = current_time() - time_from_us(5000000);
  }
}

void handle_boost_control() {
}

void handle_idle_control() {
}

#ifdef UNITTEST
#include <check.h>
#include "check_platform.h"

START_TEST(check_tasks_handle_fuel_pump) {

  /* Initial conditions */
  config.fueling.fuel_pump_pin = 1;
  ck_assert_int_eq(get_gpio(1), 0);

  set_current_time(time_from_us(500));
  handle_fuel_pump();  
  ck_assert_int_eq(get_gpio(1), 1);

  /* Wait 4 seconds, should turn off */
  set_current_time(time_from_us(4000000));
  handle_fuel_pump();  
  ck_assert_int_eq(get_gpio(1), 0);

  /* Wait 4 more seconds, should still be off */
  set_current_time(time_from_us(8000000));
  handle_fuel_pump();  
  ck_assert_int_eq(get_gpio(1), 0);

  /* Get RPM sync */
  config.decoder.state = DECODER_RPM;
  handle_fuel_pump();  
  ck_assert_int_eq(get_gpio(1), 1);

  /* Wait 10 more seconds, should still be on */
  set_current_time(time_from_us(18000000));
  handle_fuel_pump();  
  ck_assert_int_eq(get_gpio(1), 1);

  /* Lose RPM sync */
  config.decoder.state = DECODER_NOSYNC;
  set_current_time(time_from_us(19000000));
  handle_fuel_pump();  
  ck_assert_int_eq(get_gpio(1), 1);

  /* Keep waiting, should shut off */
  set_current_time(time_from_us(25000000));
  handle_fuel_pump();  
  ck_assert_int_eq(get_gpio(1), 0);

  /* Keep waiting, should stay shut off */
  set_current_time(time_from_us(40000000));
  handle_fuel_pump();  
  ck_assert_int_eq(get_gpio(1), 0);

} END_TEST

TCase *setup_tasks_tests() {
  TCase *tasks_tests = tcase_create("tasks");
  tcase_add_test(tasks_tests, check_tasks_handle_fuel_pump);
  return tasks_tests;
}

#endif

