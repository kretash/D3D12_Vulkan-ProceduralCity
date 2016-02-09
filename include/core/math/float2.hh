#pragma once

struct float2 {
  float x, y;

  float2() {
    x = 0.0f; y = 0.0f;
  }
  float2( float x, float y ) {
    this->x = x;
    this->y = y;
  }
  float2 operator+( float2 o ) {
    float2 add;
    add.x = this->x + o.x;
    add.y = this->y + o.y;
    return add;
  }
  float2 operator-( float2 o ) {
    float2 sub;
    sub.x = this->x - o.x;
    sub.y = this->y - o.y;
    return sub;
  }
  float2 operator*( float o ) {
    float2 mul;
    mul.x = this->x * o;
    mul.y = this->y * o;
    return mul;
  }
};

struct int2 {
  int32_t x, y;

  int2() {
    x = 0; y = 0;
  }
  int2( int32_t x, int32_t y ) {
    this->x = x;
    this->y = y;
  }
  int2 operator+( int2 o ) {
    int2 add;
    add.x = this->x + o.x;
    add.y = this->y + o.y;
    return add;
  }
};