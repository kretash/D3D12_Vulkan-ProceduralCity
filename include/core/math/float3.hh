#pragma once

#include <cmath>
#include <ostream>

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

struct float3 {
  float x, y, z;

  float3() { x = 0, y = 0, z = 0; }
  float3( float x, float y, float z ) {
    this->x = x;
    this->y = y;
    this->z = z;
  }
  void normalize() {
    float m = sqrtf( x * x + y * y + z * z );
    x /= m;
    y /= m;
    z /= m;
  }

  float3 operator+( float3 o ) {
    float3 add;
    add.x = this->x + o.x;
    add.y = this->y + o.y;
    add.z = this->z + o.z;
    return add;
  }
  float3 operator-( float3 o ) {
    float3 sub;
    sub.x = this->x - o.x;
    sub.y = this->y - o.y;
    sub.z = this->z - o.z;
    return sub;
  }
  float3 operator-() {
    float3 sub;
    sub.x = -this->x;
    sub.y = -this->y;
    sub.z = -this->z;
    return sub;
  }
  float3 operator*( float o ) {
    float3 mul;
    mul.x = this->x * o;
    mul.y = this->y * o;
    mul.z = this->z * o;
    return mul;
  }
  float3 operator+( float o ) {
    float3 add;
    add.x = this->x + o;
    add.y = this->y + o;
    add.z = this->z + o;
    return add;
  }
  void operator*=( float o ) {
    this->x *= o;
    this->y *= o;
    this->z *= o;
  }
  void operator+=( float o ) {
    this->x += o;
    this->y += o;
    this->z += o;
  }
  void operator+=( float3 o ) {
    this->x += o.x;
    this->y += o.y;
    this->z += o.z;
  }
  void operator-=( float3 o ) {
    this->x -= o.x;
    this->y -= o.y;
    this->z -= o.z;
  }
  float3 operator/( float3 o ) {
    float3 div;
    div.x = this->x / o.x;
    div.y = this->y / o.y;
    div.z = this->z / o.z;
    return div;
  }
  float3 operator/( float o ) {
    float3 div;
    div.x = this->x / o;
    div.y = this->y / o;
    div.z = this->z / o;
    return div;
  }
  bool operator==( float3 o ) {
    bool equal = true;
    equal &= ( o.x == x );
    equal &= ( o.y == y );
    equal &= ( o.z == z );
    return equal;
  }

  static float3 normal( const float3& f ) {
    float3 normal = f;
    float m = f.x * f.x + f.y * f.y + f.z * f.z;
    if( m == 0.0f ) return f;
    m = FastInvSqrt( m );
    normal *= m;
    return normal;
  }

  static float dot( const float3& d, const float3& f ) {
    float dot = d.x * f.x + d.y * f.y + d.z * f.z;
    return dot;
  }

  static float3 cross( const float3& d, const float3& f ) {
    float3 r;
    r.x = d.y * f.z - d.z * f.y;
    r.y = d.z * f.x - d.x * f.z;
    r.z = d.x * f.y - d.y * f.x;
    return r;
  }

  static float lenght( const float3& f ) {
    float square_me =
      f.x * f.x +
      f.y * f.y +
      f.z * f.z;
    return sqrtf( square_me );
  }
};

struct plane {
  float x, y, z, d;

  plane() {
    x = 0.0f; y = 0.0f; z = 0.0f; d = 0.0f;
  }
  plane( float x, float y, float z, float d ) {
    this->x = x;
    this->y = y;
    this->z = z;
    this->d = d;
  }
  void normalize() {
    float scale = 1.0f / ( float3::lenght( float3( x, y, z ) ) );
    x *= scale;
    y *= scale;
    z *= scale;
    d *= scale;
  }
  float3 xyz() {
    return float3( x, y, z );
  }
};