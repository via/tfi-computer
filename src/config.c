#include "config.h"
#include "sensors.h"

struct table enrich_vs_temp_and_map __attribute__((section(".configdata"))) = {
  .title = "temp_enrich", .num_axis = 2,
  .axis = { { 
      .name = "TEMP", .num = 6,
      .values = {-20, 0, 20, 40, 102, 120},
    },
    { .name = "MAP", .num = 4,
      .values = {20, 60, 80, 100},
    },
  },
  .data = {
    .two = {
      {1.3, 1.2, 1.2, 1.0, 1.0, 1.2},
      {1.3, 1.2, 1.2, 1.0, 1.0, 1.2},
      {1.3, 1.1, 1.1, 1.0, 1.0, 1.2},
      {1.2, 1.1, 1.0, 1.0, 1.0, 1.2},
    },
  },
};

/* Thousandths of a cm^3 */
struct table tipin_vs_tpsrate_and_tps __attribute__((section(".configdata"))) = {
  .title = "tipin_enrich_amount", .num_axis = 2,
  .axis = { { 
      .name = "TPSRATE", .num = 5,
      .values = {-1000, 0, 100, 500, 1000},
    },
    { .name = "TPS", .num = 5,
      .values = {0, 10, 20, 50, 80},
    },
  },
  .data = {
    .two = {
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 20, 30},
      {0, 0, 0, 15, 20},
      {0, 0, 0, 10, 10},
    },
  },
};

struct table tipin_duration_vs_rpm __attribute__((section(".configdata"))) = {
  .title = "tipin_enrich_duration", .num_axis = 1,
  .axis = { { 
      .name = "RPM", .num = 4,
      .values = {0, 1000, 2000, 4000},
    },
  },
  .data = {
    .one = {300.0, 150.0, 80.0, 40.0}
  },
};

struct table ve_vs_rpm_and_map __attribute__((section(".configdata"))) = {
  .title = "ve", .num_axis = 2,
  .axis = { { 
      .name = "RPM", .num = 17,
      .values = {250, 500, 900, 1200, 1600, 2000, 2400, 2800, 3200, 3600, 4000, 4500, 5000, 5500, 6000, 6500, 7000},
    },
    { .name = "MAP", .num = 16,
      .values = {23, 30, 40, 50, 60, 70, 80, 90, 100, 120, 140, 160, 180, 200, 220, 240},
    },
  },
  .data = {
    .two = {
   /*  250,   500,   900,  1200,  1600,   2000,  2400,  2800, 3200,  3600,  4000,   4500,  5000,  5500,  6000,  6500,  7000}, */
      {75.0,  50.0,  50.0,  50.0,  60.0,  61.0,  64.0,  62.0,  62.0,  60.0,  58.0,  59.0,  65.0,  65.0,  65.0,  65.0,  65.0}, /* 23 */
      {75.0,  55.0,  52.0,  57.0,  51.0,  50.0,  56.0,  55.0,  54.0,  59.0,  59.0,  60.0,  60.0,  70.0,  70.0,  70.0,  70.0}, /* 30 */
      {75.0,  60.0,  59.0,  66.0,  63.0,  60.0,  66.0,  65.0,  67.0,  63.0,  62.0,  70.0,  70.0,  70.0,  70.0,  70.0,  70.0}, /* 40 */
      {75.0,  55.0,  65.0,  70.0,  65.0,  68.0,  72.0,  69.0,  69.0,  69.0,  58.0,  70.0,  70.0,  70.0,  70.0,  70.0,  70.0}, /* 50 */
      {75.0,  55.0,  67.0,  75.0,  71.0,  72.0,  78.0,  75.0,  78.0,  76.0,  71.0,  71.0,  70.0,  70.0,  70.0,  70.0,  70.0}, /* 60 */
      {75.0,  55.0,  76.0,  88.0,  83.0,  77.0,  83.0,  82.0,  79.0,  82.0,  77.0,  77.0,  76.0,  77.0,  77.0,  77.0,  77.0}, /* 70 */
      {75.0,  55.0,  90.0,  84.0,  90.0,  81.0,  86.0,  77.0,  87.0,  84.0,  94.0,  80.0,  84.0,  80.0,  78.0,  78.0,  78.0}, /* 80 */
      {75.0,  55.0,  88.0,  92.0,  88.0,  89.0,  90.0,  99.0,  96.0,  93.0, 109.0, 100.0,  90.0,  80.0,  80.0,  80.0,  82.0}, /* 90 */
      {75.0,  55.0,  90.0, 103.0,  95.0,  90.0,  94.0,  91.0,  93.0,  87.0, 104.0, 100.0,  90.0,  87.0,  87.0,  88.0,  88.0}, /* 100 */
      {75.0,  55.0,  82.0,  88.0,  93.0,  93.0,  95.0,  90.0,  98.0,  96.0, 104.0,  98.0,  92.0,  90.0,  90.0,  90.0,  90.0}, /* 120 */
      {75.0,  55.0,  82.0,  88.0,  93.0,  93.0,  95.0,  93.0,  98.0,  94.0, 106.0,  98.0,  97.0,  97.0,  97.0,  97.0,  97.0}, /* 140 */
      {75.0,  55.0,  82.0,  88.0,  93.0,  93.0,  95.0,  95.0,  95.0,  93.0, 101.0,  98.0,  97.0,  97.0,  96.0,  96.0,  98.0}, /* 160 */
      {75.0,  55.0,  82.0,  88.0,  93.0,  93.0,  95.0,  95.0,  95.0,  95.0, 102.0,  98.0,  97.0,  97.0,  96.0,  96.0,  98.0}, /* 180 */
      {75.0,  55.0,  82.0,  88.0,  93.0,  93.0,  95.0,  95.0,  95.0,  95.0, 100.0,  98.0,  97.0,  97.0,  96.0,  96.0,  98.0}, /* 200 */
      {75.0,  55.0,  82.0,  88.0,  93.0,  93.0,  95.0,  95.0,  95.0,  95.0,  98.0,  98.0,  97.0,  97.0,  96.0,  96.0,  98.0}, /* 220 */
      {75.0,  55.0,  82.0,  88.0,  93.0,  93.0,  95.0,  95.0,  95.0,  95.0,  98.0,  98.0,  97.0,  97.0,  96.0,  96.0,  98.0}, /* 240 */
    },
  },
};

struct table lambda_vs_rpm_and_map __attribute__((section(".configdata"))) = {
  .title = "lambda", .num_axis = 2,
  .axis = { { 
      .name = "RPM", .num = 16,
      .values = {250, 500, 900, 1200, 1600, 2000, 2400, 3000, 3600, 4000, 4400, 5200, 5800, 6400, 6800, 7200},
    },
    { .name = "MAP", .num = 16,
      .values = {20, 30, 40, 50, 60, 70, 80, 90, 100, 120, 140, 160, 180, 200, 220, 240},
    },
  },
  .data = {
    .two = {
      {1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00},
      {1.00, 1.00, 0.95, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00},
      {1.00, 1.00, 0.95, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00},
      {1.00, 1.00, 0.90, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00},
      {1.00, 1.00, 0.90, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00},
      {1.00, 1.00, 0.90, 0.95, 0.95, 0.95, 0.95, 0.95, 0.95, 0.95, 0.95, 0.95, 0.95, 0.95, 0.95, 0.95},
      {0.90, 0.90, 0.90, 0.95, 0.95, 0.95, 0.95, 0.95, 0.95, 0.95, 0.95, 0.95, 0.95, 0.95, 0.95, 0.95},
      {0.92, 0.92, 0.90, 0.92, 0.92, 0.92, 0.92, 0.92, 0.92, 0.92, 0.92, 0.92, 0.92, 0.92, 0.92, 0.92},
      {0.90, 0.90, 0.90, 0.90, 0.90, 0.90, 0.90, 0.90, 0.90, 0.90, 0.90, 0.90, 0.90, 0.90, 0.90, 0.90},
      {0.86, 0.86, 0.86, 0.86, 0.86, 0.86, 0.86, 0.86, 0.86, 0.86, 0.86, 0.86, 0.86, 0.86, 0.86, 0.86},
      {0.83, 0.83, 0.83, 0.83, 0.83, 0.83, 0.83, 0.83, 0.83, 0.83, 0.83, 0.83, 0.83, 0.83, 0.83, 0.83},
      {0.83, 0.83, 0.83, 0.83, 0.83, 0.83, 0.83, 0.83, 0.83, 0.83, 0.83, 0.83, 0.83, 0.83, 0.83, 0.83},
      {0.81, 0.81, 0.81, 0.81, 0.81, 0.82, 0.82, 0.82, 0.82, 0.82, 0.82, 0.82, 0.82, 0.82, 0.82, 0.82},
      {0.81, 0.81, 0.81, 0.81, 0.81, 0.82, 0.82, 0.82, 0.82, 0.82, 0.82, 0.82, 0.82, 0.82, 0.82, 0.82},
      {0.75, 0.75, 0.75, 0.76, 0.76, 0.76, 0.76, 0.76, 0.76, 0.76, 0.76, 0.76, 0.76, 0.76, 0.76, 0.76},
      {0.75, 0.75, 0.75, 0.76, 0.76, 0.76, 0.76, 0.76, 0.76, 0.76, 0.76, 0.76, 0.76, 0.76, 0.76, 0.76},
    },
  },
};

struct table timing_vs_rpm_and_map __attribute__((section(".configdata"))) = {
  .title = "Timing", .num_axis = 2,
  .axis = {
    { .name = "RPM", .num = 16,
      .values = {250, 500, 900, 1200, 1600, 2000, 2400, 3000, 3600, 4000, 4400, 5200, 5800, 6400, 6800, 7200},
    },
    {
      .name = "MAP", .num = 16,
      .values = {20, 30, 40, 50, 60, 70, 80, 90, 100, 120, 140, 160, 180, 200, 220, 240},
    },
  },
  .data = {
    .two = {
      {12.0, 15.0, 18.0, 28.0, 32.0, 35.0, 42.0, 42.0, 42.0, 42.0, 42.0, 42.0, 42.0, 42.0, 42.0, 42.0},
      {12.0, 15.0, 22.0, 24.0, 28.0, 32.0, 36.0, 36.0, 38.0, 38.0, 40.0, 40.0, 40.0, 40.0, 40.0, 40.0},
      {12.0, 15.0, 22.0, 24.0, 25.0, 30.0, 32.0, 35.0, 36.0, 36.0, 38.0, 40.0, 40.0, 40.0, 40.0, 40.0},
      {10.0, 15.0, 18.0, 24.0, 24.0, 28.0, 30.0, 32.0, 34.0, 35.0, 35.0, 35.0, 35.0, 35.0, 35.0, 35.0},
      {10.0, 15.0, 15.0, 20.0, 22.0, 24.0, 28.0, 30.0, 34.0, 34.0, 34.0, 34.0, 34.0, 34.0, 34.0, 34.0},
      {10.0, 15.0, 15.0, 17.0, 17.0, 22.0, 26.0, 28.0, 32.0, 32.0, 32.0, 32.0, 32.0, 32.0, 32.0, 32.0},
      {10.0, 10.0, 10.0, 15.0, 18.0, 18.0, 24.0, 26.0, 31.0, 31.0, 31.0, 31.0, 31.0, 31.0, 31.0, 31.0},
      {8.0, 10.0, 10.0, 12.0, 15.0, 15.0, 22.0, 28.0, 30.0, 32.0, 32.0, 32.0, 32.0, 32.0, 32.0, 32.0},

      {8.0, 8.0, 8.0, 8.0, 10.0, 18.0, 24.0, 28.0, 28.0, 30.0, 30.0, 30.0, 30.0, 30.0, 30.0, 30.0},

      {8.0, 10.0, 10.0, 12.0, 13.0, 15.0, 22.0, 25.0, 26.0, 28.0, 28.0, 28.0, 28.0, 28.0, 28.0, 28.0},
      {8.0, 10.0, 10.0, 12.0, 13.0, 15.0, 18.0, 21.0, 23.0, 24.0, 24.0, 24.0, 24.0, 24.0, 24.0, 24.0},
      {8.0, 10.0, 10.0, 12.0, 13.0, 13.0, 15.0, 17.0, 20.0, 20.0, 21.0, 21.0, 21.0, 21.0, 21.0, 21.0},
      {8.0, 10.0, 10.0, 12.0, 13.0, 13.0, 15.0, 16.0, 16.0, 18.0, 18.0, 18.0, 18.0, 18.0, 18.0, 18.0},
      {8.0, 10.0, 10.0, 12.0, 12.0, 13.0, 13.0, 13.0, 15.0, 15.0, 15.0, 15.0, 15.0, 15.0, 15.0, 15.0},
      {8.0, 10.0, 10.0, 10.0, 10.0, 11.0, 13.0, 13.0, 13.0, 13.0, 13.0, 13.0, 13.0, 13.0, 13.0, 13.0},
      {8.0, 10.0, 10.0, 10.0, 10.0, 10.0, 10.0, 10.0, 10.0, 10.0, 10.0, 10.0, 10.0,  8.0,  8.0,  8.0},
    },
  },
};

struct table injector_dead_time __attribute__((section(".configdata"))) = {
  .title = "dead_time", .num_axis = 1,
  .axis = { { .name = "BRV", .num = 5,
    .values = {8.0, 10.0, 12.0, 14.0, 16.0},
    },
  },
  .data = {
    .one = {2.6, 1.675, 1.24, .99, .805},
  },
};

struct table boost_control_pwm __attribute__((section(".configdata"))) = {
  .title = "boost_control", .num_axis = 1,
  .axis = { { .name = "RPM", .num = 6,
      .values = {1000.0, 2000.0, 3000.0, 4000.0, 5000.0, 6000.0},
    },
  },
  .data = {
    .one = {0.6, 0.63, 0.63, 0.68, 0.68, 0.68},
  },
};

struct config config __attribute__((section(".configdata"))) = {
  .num_events = 12,
  .events = {
    {.type=IGNITION_EVENT, .angle=0, .pin=0},
    {.type=IGNITION_EVENT, .angle=120, .pin=1},
    {.type=IGNITION_EVENT, .angle=240, .pin=2},
    {.type=IGNITION_EVENT, .angle=360, .pin=0},
    {.type=IGNITION_EVENT, .angle=480, .pin=1},
    {.type=IGNITION_EVENT, .angle=600, .pin=2},

    {.type=FUEL_EVENT, .angle=0, .pin=8},
    {.type=FUEL_EVENT, .angle=120, .pin=9},
    {.type=FUEL_EVENT, .angle=240, .pin=10},
    {.type=FUEL_EVENT, .angle=360, .pin=8},
    {.type=FUEL_EVENT, .angle=480, .pin=9},
    {.type=FUEL_EVENT, .angle=600, .pin=10},
  },
  .decoder = {
    .type = TOYOTA_24_1_CAS,
    .offset = 50,
    .trigger_max_rpm_change = 0.55, /*Startup sucks with only 90* trigger */
    .trigger_min_rpm = 80,
  },
  .sensors = {
    [SENSOR_BRV] = {.pin=0, .source=SENSOR_ADC, .method=METHOD_LINEAR,
      .params={.range={.min=0, .max=24.5}}, .lag=80,
      .fault_config={.min = 100, .max = 4000, .fault_value = 13.8}},
    [SENSOR_IAT] = {.pin=1, .source=SENSOR_ADC, .method=METHOD_THERM,
      .fault_config={.min = 2, .max = 4095, .fault_value = 10.0},
      .params={.therm={
        .bias=2490,
        .a=0.00146167419060305,
        .b=0.00022887572003919,
        .c=1.64484831669638E-07,
      }}},
    [SENSOR_CLT] = {.pin=2, .source=SENSOR_ADC, .method=METHOD_THERM,
      .fault_config={.min = 2, .max = 4095, .fault_value = 50.0},
      .params={.therm={
        .bias=2490,
        .a=0.00131586818223649,
        .b=0.00025618700140100302,
        .c=0.00000018474199456928,
      }}},
    [SENSOR_EGO] = {.pin=3, .source=SENSOR_ADC, .method=METHOD_LINEAR,
      .params={.range={.min=0.68, .max=1.36}}},
    [SENSOR_MAP] = {.pin=4, .source=SENSOR_ADC, .method=METHOD_LINEAR,
      .params={.range={.min=12, .max=420}}, /* AEM 3.5 bar MAP sensor*/
      .fault_config={.min = 10, .max = 4050, .fault_value = 50.0},
    .lag=10.0},
    [SENSOR_AAP] = {.pin=5, .source=SENSOR_ADC, .method=METHOD_LINEAR,
      .params={.range={.min=10.5, .max=121.6}}, /* AEM 3.5 bar MAP sensor*/
      .fault_config={.min = 10, .max = 4050, .fault_value = 100.0}},
    [SENSOR_TPS] = {.pin=6, .source=SENSOR_ADC, .method=METHOD_LINEAR,
      .params={.range={.min=-15.74, .max=145.47}},
      .fault_config={.min = 200, .max = 3200, .fault_value = 25.0},
      .lag = 10.0},
    [SENSOR_FRT] = {.source=SENSOR_CONST, .params={.fixed_value = 15.0}},
  },
  .timing = &timing_vs_rpm_and_map,
  .injector_pw_compensation = &injector_dead_time,
  .ve = &ve_vs_rpm_and_map,
  .commanded_lambda = &lambda_vs_rpm_and_map,
  .engine_temp_enrich = &enrich_vs_temp_and_map,
  .tipin_enrich_amount = &tipin_vs_tpsrate_and_tps,
  .tipin_enrich_duration = &tipin_duration_vs_rpm,
  .rpm_stop = 6700,
  .rpm_start = 6200,
  .fueling = {
    .injector_cc_per_minute = 1015,
    .cylinder_cc = 500,
    .fuel_stoich_ratio = 14.7,
    .injections_per_cycle = 2, /* All batched */
    .fuel_pump_pin = 0,
    .density_of_fuel = 0.755, /* g/cm^3 at 15C */
    .density_of_air_stp = 1.2754e-3, /* g/cm^3 */
    .crank_enrich_config = {
      .crank_rpm = 400,
      .cutoff_temperature = 20.0,
      .enrich_amt = 2.5,
    },
  },
  .ignition = {
    .dwell = DWELL_FIXED_TIME,
    .dwell_us = 2800,
    .min_fire_time_us = 500,
  },
  .boost_control = {
    .pwm_duty_vs_rpm = &boost_control_pwm,
    .threshhold_kpa = 130.0,
    .pin = 1,
    .overboost = 210.0,
  },
};

int config_valid() {
  if (config.ve && !table_valid(config.ve)) {
    return 0;
  }

  if (config.timing && !table_valid(config.timing)) {
    return 0;
  }

  if (config.injector_pw_compensation &&
      !table_valid(config.injector_pw_compensation)) {
    return 0;
  }

  if (config.commanded_lambda && !table_valid(config.commanded_lambda)) {
    return 0;
  }

  if (config.tipin_enrich_amount && !table_valid(config.tipin_enrich_amount)) {
    return 0;
  }

  if (config.tipin_enrich_duration &&
      !table_valid(config.tipin_enrich_duration)) {
    return 0;
  }

  return 1;
}
