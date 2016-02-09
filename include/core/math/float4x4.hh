#pragma once

#include <cmath>
#include <memory>

struct float3;
struct float4;

struct float4x4 {
  union {
    float data[16];
    float m[4][4];
  };

  float4x4();
  float4x4( float i );
  float4x4( float4 x, float4 y, float4 z, float4 w );
  float4x4( float* p );
  void identity();

  void translate( float x, float y, float z );
  void translate( float3 p );

  void rotate_x( float a );
  void rotate_y( float a );
  void rotate_z( float a );

  void scale( float3 scale );

  void prespective( float fov, float aspect_ratio, float n, float f );
  void look_at( float3 eye, float3 focus, float3 up );
  void transpose();

  operator float*( ) { return( &m[0][0] ); }
  operator const float*( ) const { return( &m[0][0] ); }

  static float4x4 inverse( float4x4 m );

  float4x4 operator*( const float4x4& o )const;

  float* get() { return data; }
};