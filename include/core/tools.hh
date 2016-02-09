/*
----------------------------------------------------------------------------------------------------
------                  _   _____ _  __                     ------------ /_/\  ---------------------
------              |/ |_) |_  | |_|(_ |_|                  ----------- / /\ \  --------------------
------              |\ | \ |__ | | |__)| |                  ---------- / / /\ \  -------------------
------   CARLOS MARTINEZ ROMERO - kretash.wordpress.com     --------- / / /\ \ \  ------------------
------                                                      -------- / /_/__\ \ \  -----------------
------       PROCEDURAL CITY RENDERING WITH THE NEW         ------  /_/______\_\/\  ----------------
------            GENERATION GRAPHICS APIS                  ------- \_\_________\/ -----------------
----------------------------------------------------------------------------------------------------

Licensed under the MIT License (the "License"); you may not use this file except
in compliance with the License. You may obtain a copy of the License at
http://opensource.org/licenses/MIT
*/

#pragma once

#include <iostream>

namespace tools {

  static inline float random( float min, float max ) {
    return min + static_cast < float > ( rand() ) / ( static_cast < float > ( RAND_MAX / ( max - min ) ) );
  }

  static inline int random( int min, int max ) {
    return min + static_cast < int > ( rand() ) / ( static_cast < int > ( RAND_MAX / ( max - min ) ) );
  }

  static inline float lerp( float a, float b, float t ) {
    return a + ( b - a ) * t;
  }

  static inline float3 lerp( float3 a, float3 b, float t ) {
    return float3( lerp( a.x, b.x, t ), lerp( a.y, b.y, t ), lerp( a.z, b.z, t ) );
  }

  template<class T>
  static inline T clamp( T a, T min, T max ) {
    T r = a;

    if( a < min )
      a = min;

    if( a > max )
      a = max;

    return a;
  }

  static inline float round_down( float a, float range ) {
    float r = a;

    if( r < range && r > -range )
      r = 0.0f;

    return r;
  }

  static inline float luminance( float3 v ) {
    return v.x * 0.212f + v.y * 0.716f + v.z * 0.072f;
  }

};