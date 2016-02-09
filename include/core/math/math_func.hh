#pragma once

#define pi 3.1415926535897932384626433832795028841971693993f
#define to_rad(x) (x*pi)/180.0f

#include <cmath>

#if 0
// 1.5x faster than normal one
float inline __declspec ( naked ) __fastcall asm_sqrt( double n ) {
  _asm fld qword ptr[esp + 4]
  //_asm fld n
  _asm fsqrt
  //_asm fst n
  _asm ret 8
}
#else
#define asm_sqrt sqrtf
#endif

// 1.23x faster than normal one
// Greg Walsh - Quake 3
float inline FastInvSqrt( float x ) {
  float xhalf = 0.5f * x;
  int i = *( int* ) &x;
  i = 0x5f3759df - ( i >> 1 );
  x = *( float* ) &i;
  x = x*( 1.5f - ( xhalf*x*x ) );
  return x;
}