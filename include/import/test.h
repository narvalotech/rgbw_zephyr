#ifndef __TEST_H__
#define __TEST_H__

#include <stdint.h>
#include <stddef.h>
#include <msp430.h>
#include "board.h"
#include "disp.h"
#include "accel.h"

void test_tilt();
void test_graph();
void test_click();
void test_4D();
void test_clock_bcd();
void test_clock_numbers();

#endif
