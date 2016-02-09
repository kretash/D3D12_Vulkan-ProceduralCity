#pragma once

#include "math_func.hh"
#include <ostream>

struct float3 {
  float x, y, z;

  static float3 normal( const float3& f );
  static float dot( const float3& d, const float3& f );
  static float3 cross( const float3& d, const float3& f );
  static float lenght( const float3& f );

  float3() { x = 0, y = 0, z = 0; }
  float3( float x, float y, float z ) {
    this->x = x;
    this->y = y;
    this->z = z;
  }
  void normalize() {
    float m = asm_sqrt( x * x + y * y + z * z );
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
};

std::ostream& operator<<( std::ostream& os, const float3& f );

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