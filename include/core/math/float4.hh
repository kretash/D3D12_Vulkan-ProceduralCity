#pragma once

struct float4 {
  union { float x;    float r; };
  union { float y;    float g; };
  union { float z;    float b; };
  union { float w;    float a; };

  float4() {
    x = 0.0f; y = 0.0f; z = 0.0f; w = 0.0f;
  }
  float4( float x, float y, float z, float w ) {
    this->x = x;
    this->y = y;
    this->z = z;
    this->w = w;
  }
  float4 operator+( float4 o ) {
    float4 add;
    add.x = this->x + o.x;
    add.y = this->y + o.y;
    add.z = this->z + o.z;
    add.w = this->w + o.w;
    return add;
  }
};
