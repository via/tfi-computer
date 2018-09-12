#ifndef _DECODER_H
#define _DECODER_H

#include "platform.h"
#define MAX_DECODER_TIMES 8

struct decoder;
typedef void (*decoder_func)(struct decoder *);

typedef enum {
  FORD_TFI,
  TOYOTA_24_1_CAS,
} trigger_type;

typedef enum {
  DECODER_NOSYNC,
  DECODER_RPM,
  DECODER_SYNC,
} decoder_state;

typedef enum {
  DECODER_VARIATION,
  DECODER_TRIGGERCOUNT_HIGH,
  DECODER_TRIGGERCOUNT_LOW,
  DECODER_EXPIRED,
} decoder_loss_reason;

struct decoder {
  /* Unsafe interrupt-written vars */
  timeval_t last_t0;
  timeval_t last_t1;
  char needs_decoding_t0;
  char needs_decoding_t1;

  /* Safe, only handled in main loop */
  decoder_func decode;
  unsigned char valid;
  unsigned int rpm;
  timeval_t last_trigger_time;
  degrees_t last_trigger_angle;
  timeval_t expiration;

  /* Configuration */
  trigger_type type;
  degrees_t offset;
  float trigger_max_rpm_change;
  float trigger_cur_rpm_change;
  unsigned int trigger_min_rpm;
  unsigned int required_triggers_rpm;
  unsigned int num_triggers;
  degrees_t degrees_per_trigger;
  unsigned int rpm_window_size;

  /* Debug */
  unsigned int t0_count;
  unsigned int t1_count;
  decoder_loss_reason loss;

  /* Internal state */
  decoder_state state;
  unsigned int current_triggers_rpm;
  unsigned int triggers_since_last_sync;
  timeval_t times[MAX_DECODER_TIMES];
};

void decoder_init(struct decoder *);
void decoder_update_scheduling(int trigger, timeval_t time);
void enable_test_trigger(trigger_type t, unsigned int rpm);

#ifdef UNITTEST
void check_handle_decoder_expire(void *_d);
#endif

#endif

