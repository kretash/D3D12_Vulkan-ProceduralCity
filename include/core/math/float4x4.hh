#pragma once

#include <cmath>
#include <memory>
#include "core/core.hh"
#include "float3.hh"
#include "float4.hh"

#define to_rad(deg) deg * float(M_PI / 180.0f)

struct float4x4 {
  union {
    float data[16];
    float m[4][4];
  };

  float4x4() {
    float tmp[16]{
      1.0f, 0.0f, 0.0f, 0.0f,
      0.0f, 1.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 1.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f
    };

    memcpy( data, tmp, sizeof( float ) * 16 );
  }

  float4x4( float i ) {
    float tmp[16]{
      1.0f, 0.0f, 0.0f, 0.0f,
      0.0f, 1.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 1.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f
    };

    memcpy( data, tmp, sizeof( float ) * 16 );
  }

  float4x4( float4 x, float4 y, float4 z, float4 w ) {
    float tmp[16]{
      x.x, x.y, x.z, x.w,
      y.x, y.y, y.z, y.w,
      z.x, z.y, z.z, z.w,
      w.x, w.y, w.z, w.w,
    };
    memcpy( data, tmp, sizeof( float ) * 16 );
  }

  float4x4( float* p ) {
    memcpy( data, p, sizeof( float ) * 16 );
  }

  void identity() {
    float tmp[16]{
      1.0f, 0.0f, 0.0f, 0.0f,
      0.0f, 1.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 1.0f, 0.0f,
      0.0f, 0.0f, 0.0f,1.0f
    };
    memcpy( data, tmp, sizeof( float ) * 16 );
  }

  void translate( float x, float y, float z ) {
    data[12] = x;
    data[13] = y;
    data[14] = z;
  }

  void translate( float3 p ) {
    data[12] = p.x;
    data[13] = p.y;
    data[14] = p.z;
  }

  void rotate_x( float a ) {
    data[5] = cosf( a );
    data[6] = -sinf( a );
    data[9] = sinf( a );
    data[10] = cosf( a );
  }

  void rotate_y( float a ) {
    data[0] = cosf( a );
    data[2] = sinf( a );
    data[8] = -sinf( a );
    data[10] = cosf( a );
  }

  void rotate_z( float a ) {
    data[0] = cosf( a );
    data[1] = -sinf( a );
    data[4] = sinf( a );
    data[5] = cosf( a );
  }

  void scale( float3 scale ) {
    data[0] = data[0] * scale.x;
    data[5] = data[5] * scale.y;
    data[10] = data[10] * scale.z;
  }

  void prespective( float fov, float aspect_ratio, float n, float f ) {
    float tan_fov = tanf( fov / 2.0f );
    float4x4( 1.0f );
    data[0] = 1.0f / ( aspect_ratio*tan_fov );
    data[5] = 1.0f / tan_fov;
    data[10] = ( f + n ) / ( f - n );
    data[11] = 1.0f;
    data[14] = -( 2.0f * f * n ) / ( f - n );
    data[15] = 0.0f;
  }

  void look_at( float3 eye, float3 focus, float3 up ) {

    float4x4( 1.0f );
    float3 z_axis = float3::normal( focus - eye );
    float3 x_axis = float3::normal( float3::cross( up, z_axis ) );
    float3 y_axis = float3::cross( z_axis, x_axis );

    data[0] = x_axis.x;
    data[1] = y_axis.x;
    data[2] = z_axis.x;
    data[3] = 0;
    data[4] = x_axis.y;
    data[5] = y_axis.y;
    data[6] = z_axis.y;
    data[7] = 0;
    data[8] = x_axis.z;
    data[9] = y_axis.z;
    data[10] = z_axis.z;
    data[11] = 0;

    data[12] = -float3::dot( x_axis, eye );
    data[13] = -float3::dot( y_axis, eye );
    data[14] = -float3::dot( z_axis, eye );
    data[15] = 1;

  }

  void transpose() {
    float4x4 t = float4x4(
      float4( data[0], data[4], data[8], data[12] ),
      float4( data[1], data[5], data[9], data[13] ),
      float4( data[2], data[6], data[10], data[14] ),
      float4( data[3], data[7], data[11], data[15] ) );

    memcpy( data, t.data, sizeof( float ) * 16 );
  }

  operator float*( ) { return( &m[0][0] ); }
  operator const float*( ) const { return( &m[0][0] ); }

  static float4x4 inverse( float4x4 m ) {
    float temp = ( m.data[0] * m.data[5] * m.data[10] * m.data[15] ) + ( m.data[0] * m.data[6] * m.data[11] * m.data[13] )//DONE
      + ( m.data[0] * m.data[7] * m.data[9] * m.data[14] ) + ( m.data[1] * m.data[4] * m.data[11] * m.data[14] )//DONE
      + ( m.data[1] * m.data[6] * m.data[8] * m.data[15] ) + ( m.data[1] * m.data[7] * m.data[10] * m.data[12] )//DONE
      + ( m.data[2] * m.data[4] * m.data[9] * m.data[15] ) + ( m.data[2] * m.data[5] * m.data[11] * m.data[12] )//DONE
      + ( m.data[2] * m.data[7] * m.data[8] * m.data[13] ) + ( m.data[3] * m.data[4] * m.data[10] * m.data[13] )//DONE
      + ( m.data[3] * m.data[5] * m.data[8] * m.data[14] ) + ( m.data[3] * m.data[6] * m.data[9] * m.data[12] )//DONE

      - ( m.data[0] * m.data[5] * m.data[11] * m.data[14] ) - ( m.data[0] * m.data[6] * m.data[9] * m.data[15] )//DONE
      - ( m.data[0] * m.data[7] * m.data[10] * m.data[13] ) - ( m.data[1] * m.data[4] * m.data[10] * m.data[15] )//DONE
      - ( m.data[1] * m.data[6] * m.data[11] * m.data[12] ) - ( m.data[1] * m.data[7] * m.data[8] * m.data[14] )//DONE
      - ( m.data[2] * m.data[4] * m.data[11] * m.data[13] ) - ( m.data[2] * m.data[5] * m.data[8] * m.data[15] )//DONE
      - ( m.data[2] * m.data[7] * m.data[9] * m.data[12] ) - ( m.data[3] * m.data[4] * m.data[9] * m.data[14] )//DONE
      - ( m.data[3] * m.data[5] * m.data[10] * m.data[12] ) - ( m.data[3] * m.data[6] * m.data[8] * m.data[13] );//DONE
    if( temp != 0 ) {
      temp = 1.0f / temp;
      float4x4 b;
      b.data[0] = m.data[5] * m.data[10] * m.data[15] + m.data[6] * m.data[11] * m.data[13] + m.data[7] * m.data[9] *
        m.data[14] - m.data[5] * m.data[11] * m.data[14] - m.data[6] * m.data[9] * m.data[15] - m.data[7] * m.data[10] * m.data[13];

      b.data[1] = m.data[1] * m.data[11] * m.data[14] + m.data[2] * m.data[9] * m.data[15] + m.data[3] * m.data[10] *
        m.data[13] - m.data[1] * m.data[10] * m.data[15] - m.data[2] * m.data[11] * m.data[13] - m.data[3] * m.data[9] * m.data[14];

      b.data[2] = m.data[1] * m.data[6] * m.data[15] + m.data[2] * m.data[7] * m.data[13] + m.data[3] * m.data[5] *
        m.data[14] - m.data[1] * m.data[7] * m.data[14] - m.data[2] * m.data[5] * m.data[15] - m.data[3] * m.data[6] * m.data[13];

      b.data[3] = m.data[1] * m.data[7] * m.data[10] + m.data[2] * m.data[5] * m.data[11] + m.data[3] * m.data[6] *
        m.data[9] - m.data[1] * m.data[6] * m.data[11] - m.data[2] * m.data[7] * m.data[9] - m.data[3] * m.data[5] * m.data[10];

      b.data[4] = m.data[4] * m.data[11] * m.data[14] + m.data[6] * m.data[8] * m.data[15] + m.data[7] * m.data[10] *
        m.data[12] - m.data[4] * m.data[10] * m.data[15] - m.data[6] * m.data[11] * m.data[12] - m.data[7] * m.data[8] * m.data[14];

      b.data[5] = m.data[0] * m.data[10] * m.data[15] + m.data[2] * m.data[11] * m.data[12] + m.data[3] * m.data[8] *
        m.data[14] - m.data[0] * m.data[11] * m.data[14] - m.data[2] * m.data[8] * m.data[15] - m.data[3] * m.data[10] * m.data[12];

      b.data[6] = m.data[0] * m.data[7] * m.data[14] + m.data[2] * m.data[4] * m.data[15] + m.data[3] * m.data[6] *
        m.data[12] - m.data[0] * m.data[6] * m.data[15] - m.data[2] * m.data[7] * m.data[12] - m.data[3] * m.data[4] * m.data[14];

      b.data[7] = m.data[0] * m.data[6] * m.data[11] + m.data[2] * m.data[7] * m.data[8] + m.data[3] * m.data[4] *
        m.data[10] - m.data[0] * m.data[7] * m.data[10] - m.data[2] * m.data[4] * m.data[11] - m.data[3] * m.data[6] * m.data[8];

      b.data[8] = m.data[4] * m.data[9] * m.data[15] + m.data[5] * m.data[11] * m.data[12] + m.data[7] * m.data[8] *
        m.data[13] - m.data[4] * m.data[11] * m.data[13] - m.data[5] * m.data[8] * m.data[15] - m.data[7] * m.data[9] * m.data[12];

      b.data[9] = m.data[0] * m.data[11] * m.data[13] + m.data[1] * m.data[8] * m.data[15] + m.data[3] * m.data[9] *
        m.data[12] - m.data[0] * m.data[9] * m.data[15] - m.data[1] * m.data[11] * m.data[12] - m.data[3] * m.data[8] * m.data[13];

      b.data[10] = m.data[0] * m.data[5] * m.data[15] + m.data[1] * m.data[7] * m.data[12] + m.data[3] * m.data[4] *
        m.data[13] - m.data[0] * m.data[7] * m.data[13] - m.data[1] * m.data[4] * m.data[15] - m.data[3] * m.data[5] * m.data[12];

      b.data[11] = m.data[0] * m.data[7] * m.data[9] + m.data[1] * m.data[4] * m.data[11] + m.data[3] * m.data[5] *
        m.data[8] - m.data[0] * m.data[5] * m.data[11] - m.data[1] * m.data[7] * m.data[8] - m.data[3] * m.data[4] * m.data[9];

      b.data[12] = m.data[4] * m.data[10] * m.data[13] + m.data[5] * m.data[8] * m.data[14] + m.data[6] * m.data[9] *
        m.data[12] - m.data[4] * m.data[9] * m.data[14] - m.data[5] * m.data[10] * m.data[12] - m.data[6] * m.data[8] * m.data[13];

      b.data[13] = m.data[0] * m.data[9] * m.data[14] + m.data[1] * m.data[10] * m.data[12] + m.data[2] * m.data[8] *
        m.data[13] - m.data[0] * m.data[10] * m.data[13] - m.data[1] * m.data[8] * m.data[14] - m.data[2] * m.data[9] * m.data[12];

      b.data[14] = m.data[0] * m.data[6] * m.data[13] + m.data[1] * m.data[4] * m.data[14] + m.data[2] * m.data[5] *
        m.data[12] - m.data[0] * m.data[5] * m.data[14] - m.data[1] * m.data[6] * m.data[12] - m.data[2] * m.data[4] * m.data[13];

      b.data[15] = m.data[0] * m.data[5] * m.data[10] + m.data[1] * m.data[6] * m.data[8] + m.data[2] * m.data[4] *
        m.data[9] - m.data[0] * m.data[6] * m.data[9] - m.data[1] * m.data[4] * m.data[10] - m.data[2] * m.data[5] * m.data[8];

      b = b * temp;

      m.data[0] = b.data[0];
      m.data[1] = b.data[1];
      m.data[2] = b.data[2];
      m.data[3] = b.data[3];
      m.data[4] = b.data[4];
      m.data[5] = b.data[5];
      m.data[6] = b.data[6];
      m.data[7] = b.data[7];
      m.data[8] = b.data[8];
      m.data[9] = b.data[9];
      m.data[10] = b.data[10];
      m.data[11] = b.data[11];
      m.data[12] = b.data[12];
      m.data[13] = b.data[13];
      m.data[14] = b.data[14];
      m.data[15] = b.data[15];

      return b;
    } else {
      return float4x4( 1.0f );
    }
  }

  float4x4 operator*( const float4x4& o )const {
    float4x4 r;

    r.data[0] = o.data[0] * data[0] + o.data[4] * data[1] + o.data[8] * data[2] + o.data[12] * data[3];
    r.data[1] = o.data[1] * data[0] + o.data[5] * data[1] + o.data[9] * data[2] + o.data[13] * data[3];
    r.data[2] = o.data[2] * data[0] + o.data[6] * data[1] + o.data[10] * data[2] + o.data[14] * data[3];
    r.data[3] = o.data[3] * data[0] + o.data[7] * data[1] + o.data[11] * data[2] + o.data[15] * data[3];

    r.data[4] = o.data[0] * data[4] + o.data[4] * data[5] + o.data[8] * data[6] + o.data[12] * data[7];
    r.data[5] = o.data[1] * data[4] + o.data[5] * data[5] + o.data[9] * data[6] + o.data[13] * data[7];
    r.data[6] = o.data[2] * data[4] + o.data[6] * data[5] + o.data[10] * data[6] + o.data[14] * data[7];
    r.data[7] = o.data[3] * data[4] + o.data[7] * data[5] + o.data[11] * data[6] + o.data[15] * data[7];

    r.data[8] = o.data[0] * data[8] + o.data[4] * data[9] + o.data[8] * data[10] + o.data[12] * data[11];
    r.data[9] = o.data[1] * data[8] + o.data[5] * data[9] + o.data[9] * data[10] + o.data[13] * data[11];
    r.data[10] = o.data[2] * data[8] + o.data[6] * data[9] + o.data[10] * data[10] + o.data[14] * data[11];
    r.data[11] = o.data[3] * data[8] + o.data[7] * data[9] + o.data[11] * data[10] + o.data[15] * data[11];

    r.data[12] = o.data[0] * data[12] + o.data[4] * data[13] + o.data[8] * data[14] + o.data[12] * data[15];
    r.data[13] = o.data[1] * data[12] + o.data[5] * data[13] + o.data[9] * data[14] + o.data[13] * data[15];
    r.data[14] = o.data[2] * data[12] + o.data[6] * data[13] + o.data[10] * data[14] + o.data[14] * data[15];
    r.data[15] = o.data[3] * data[12] + o.data[7] * data[13] + o.data[11] * data[14] + o.data[15] * data[15];

    return r;
  }

  float* get() { return data; }
};