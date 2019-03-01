#ifndef UTIL_H
#define UTIL_H

#include "platform.h"

timeval_t time_diff(timeval_t t1, timeval_t t2);
int time_before(timeval_t n, timeval_t x);
timeval_t time_from_us(unsigned int us);
rpm_t rpm_from_time_diff(timeval_t t1, degrees_t degrees);
timeval_t time_from_rpm_diff(rpm_t rpm, degrees_t degrees);
bool time_in_range(timeval_t val, timeval_t t1, timeval_t t2);
degrees_t clamp_angle(int, degrees_t);

#ifdef UNITTEST
#include <check.h>
TCase *setup_util_tests();
#endif

#endif

