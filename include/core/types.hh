/*
----------------------------------------------------------------------------------------------------
------                  _   _____ _  __                     ------------ /_/\  ---------------------
------              |/ |_) |_  | |_|(_ |_|                  ----------- / /\ \  --------------------
------              |\ | \ |__ | | |__)| |                  ---------- / / /\ \  -------------------
------   CARLOS MARTINEZ ROMERO - kretash.wordpress.com     --------- / / /\ \ \  ------------------
------                                                      -------- / /_/__\ \ \  -----------------
------   PROCEDURAL CITY RENDERING WITH THE NEW             ------  /_/______\_\/\  ----------------
------         GENERATION GRAPHICS APIS                     ------- \_\_________\/ -----------------
----------------------------------------------------------------------------------------------------

Licensed under the MIT License (the "License"); you may not use this file except
in compliance with the License. You may obtain a copy of the License at
http://opensource.org/licenses/MIT
*/

#pragma once
#include <Windows.h>
#include "math/float3.hh"
#include "math/float2.hh"

namespace kretash {

  enum render_type {
    rBASIC = 0,
    rTEXTURE,
    rSKYDOME,
    rPOST,
    rCOUNT,
  };

  struct bin_offsets {
    int cbv_offset;
    int srv_offset;
  };

  enum outline_type {
    kNONE = 0,
    kTOP = 1,
    kBOT = 2,
    kLEFT = 3,
    kRIGHT = 4,
  };

  struct building_details {
    int32_t  building_i;
    float distance;
    float3 position;
    outline_type type;
    outline_type carry;

    building_details() :
      building_i( 0 ),
      distance( 0.0f ),
      position( 0.0f, 0.0f, 0.0f ),
      type( kNONE ),
      carry( kNONE ) {
    }
    building_details( float3 f, outline_type t ) :
      position( f ),
      type( t ) {
    }
    building_details( int32_t  i, float3 f, outline_type t ) :
      building_i( i ),
      position( f ),
      type( t ) {
    }
    bool operator<( building_details o ) {
      return distance < o.distance;
    }

    ~building_details() {}
  };

  enum move_type {
    kBUILDING_MOVE = 0,
    kOUTLINE_MOVE = 1,
  };

  struct move_operation {
    move_type type;
    int32_t  building_i;
    float3 end_position;
    float2 end_seed;
  };

  struct mem_block {
    uint64_t m_start;
    uint64_t m_size;

    mem_block() : m_start( 0 ), m_size( 0 ) {}
    mem_block( uint64_t start, uint64_t size ) : m_start( start ), m_size( size ) {}
    bool operator<( mem_block o ) {
      return m_start < o.m_start;
    }
  };

  struct queue {
    mem_block   v_block;
    float*      v_data;
    mem_block   i_block;
    uint32_t*   i_data;

    queue( mem_block v, float* vp, mem_block i, uint32_t* ip ) {
      v_block = v; v_data = vp;
      i_block = i; i_data = ip;
    }
    queue() :
      v_block(),
      v_data( nullptr ),
      i_block(),
      i_data( nullptr ) {
    }
  };

  struct remove_queue {
    uint32_t   v_mem;
    uint32_t   i_mem;

    remove_queue( uint32_t v, uint32_t i ) {
      v_mem = v; i_mem = i;
    }
    remove_queue() :
      v_mem( 0 ),
      i_mem( 0 ) {
    }
  };

  struct texel {
    uint8_t r, g, b, a;
    texel() :
      r( 0 ), g( 0 ), b( 0 ), a( 0 ) {
    }
    texel( uint8_t r, uint8_t b, uint8_t g, uint8_t a = 255 ) {
      this->r = r;
      this->g = g;
      this->b = b;
      this->a = a;
    }
    ~texel() {}

    void set_grayscale( uint8_t gg ) {
      r = gg, g = gg, b = gg;
    }

    void set_texel( uint8_t r, uint8_t g, uint8_t b ) {
      this->r = r; this->b = b; this->g = g;
    }

    template<class T>
    void cast_and_clamp_texel( T r, T g, T b ) {
      T t_r = clamp( r, ( T ) 0, ( T ) 255 );
      T t_g = clamp( g, ( T ) 0, ( T ) 255 );
      T t_b = clamp( b, ( T ) 0, ( T ) 255 );

      this->b = ( uint8_t ) t_r;
      this->b = ( uint8_t ) t_b;
      this->g = ( uint8_t ) t_b;
    }

    int32_t get_texel() {
      return a << 24 | b << 16 | g << 8 | r;
    }

    static float3 texel_to_float3( int32_t t ) {
      float3 color;

      color.x = ( float ) ( ( t >> 16 ) & 0xff ) / 255.0f;
      color.z = ( float ) ( ( t >> 8 ) & 0xff ) / 255.0f;
      color.y = ( float ) ( ( t ) & 0xff ) / 255.0f;

      return color;
    }
    static texel float3_to_texel( float3 f3 ) {
      texel color;

      color.r = ( uint8_t ) ( f3.x * 255.0f );
      color.g = ( uint8_t ) ( f3.y * 255.0f );
      color.b = ( uint8_t ) ( f3.z * 255.0f );
      color.a = 255;

      return color;
    }
    texel operator*( float o ) {
      float3 r = texel_to_float3( get_texel() );
      r = r * o;
      return float3_to_texel( r );
    }
  };

  struct texture_element {
    int2 start;
    int2 end;
    texel color;
    texel specular;
    float frequency;
    float intensity;

    texture_element() :
      start( 0, 0 ),
      end( 0, 0 ),
      color( 0, 0, 0 ),
      specular( 0, 0, 0 ),
      frequency( 1.0f ),
      intensity( 1.0f ) {
    }
  };

  struct texture_db {
    std::string filename;
    int32_t id;

    texture_db( std::string filename, int32_t id ) {
      this->filename = filename;
      this->id = id;
    }
  };

  enum API {
    kVulkan = 1,
    kD3D12 = 2,
  };


  struct engine_settings {
    int32_t resolution_width;
    int32_t resolution_height;
    bool fullscreen;
    int32_t grid;
    bool animated_camera;
    bool play_sound;
    std::string sound_file;
    bool msaa_enabled;
    int32_t msaa_count;
    float upscale_render;
    API m_api;
    bool update_city;

    engine_settings() :
      resolution_width( 0 ),
      resolution_height( 0 ),
      fullscreen( false ),
      grid( 0 ),
      play_sound( false ),
      sound_file(),
      msaa_enabled( false ),
      update_city( false ),
      msaa_count( 0 ),
      upscale_render( 1.0f ),
      m_api( kVulkan ) {
    }
    ~engine_settings() {}
  };



  enum groups {
    eSingle = 1,
    eDouble = 2,
    eTriple = 3,
    eQuad = 4,
    ePenta = 5,
  };

  enum texture_set {
    eLEFT = 0,
    eRIGHT,
  };

  struct building_settings {
    uint32_t  n_sides;
    uint32_t  n_floors;
    float n_floor_height;
    float3 n_center_offset;
    groups n_group;
    uint32_t n_angle_seed;
    uint32_t n_size_seed;
    float n_base_size;
    texture_set n_texture_set;
    float n_vertical_uv;

    building_settings() :
      n_sides( 0 ),
      n_floors( 0 ),
      n_floor_height( 0 ),
      n_center_offset( 0.0f, 0.0f, 0.0f ),
      n_group( eSingle ),
      n_angle_seed( 0 ),
      n_size_seed( 0 ),
      n_base_size( 0 ),
      n_texture_set( eLEFT ),
      n_vertical_uv( 1.0f ) {
    }
    ~building_settings() {}

    void init_s( const uint32_t sides, const uint32_t floors, const float f_height, const float3 center ) {
      n_sides = sides;
      n_floors = floors;
      n_floor_height = f_height;
      n_center_offset = center;
    }
    void init_r( const groups g, const uint32_t angle_sx, const uint32_t size_s, const float base_size, const texture_set t ) {
      n_group = g;
      n_angle_seed = angle_sx;
      n_size_seed = size_s;
      n_base_size = base_size;
      n_texture_set = t;
    }
  };

}