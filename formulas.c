#include <stdint.h>
#include <math.h>
//#include "formulas.h"


float deg2rad(float deg) {
  return (deg * (M_PI/180.0));
}

int32_t min(int32_t x, int32_t y) {
  int32_t ans = (x > y) ? y : x;
}

int16_t unitsToGrid(float units) {
  return (uint16_t)(floor(units/64));
}
