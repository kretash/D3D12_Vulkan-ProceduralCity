#pragma once

#include "core/math/float3.hh"

float3 float3::normal( const float3& f ) {
  float3 normal = f;
  float m = f.x * f.x + f.y * f.y + f.z * f.z;
  if( m == 0.0f ) return f;
  m = FastInvSqrt( m );
  normal *= m;
  return normal;
}

float float3::dot( const float3& d, const float3& f ) {
  float dot = d.x * f.x + d.y * f.y + d.z * f.z;
  return dot;
}

float3 float3::cross( const float3& d, const float3& f ) {
  float3 r;
  r.x = d.y * f.z - d.z * f.y;
  r.y = d.z * f.x - d.x * f.z;
  r.z = d.x * f.y - d.y * f.x;
  return r;
}

float float3::lenght( const float3& f ) {
  float square_me =
    f.x * f.x +
    f.y * f.y +
    f.z * f.z;
  return sqrtf( square_me );
}

std::ostream& operator<<( std::ostream& os, const float3& f ) {
  return os << "( " << f.x << ", " << f.y << ", " << f.z << " ) ";
}