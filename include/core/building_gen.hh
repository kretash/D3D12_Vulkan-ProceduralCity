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
#include <vector>
#include "types.hh"
#include "geometry.hh"

class                     BuildingGen : public Geometry {
public:
  BuildingGen();
  ~BuildingGen();

  void                    generate( building_settings s );
  float                   get_radius() { return m_radius; }
  void                    finish_and_upload();

private:
  void                    _copy_and_upload_buffers();

  void                    _generate_floor( uint32_t i_s, uint32_t i_f, bool top );
  void                    _generate_bot_face();
  void                    _generate_top_face();

  void                    _generate_angles_1( float* angles_array, uint32_t sides );
  void                    _generate_angles_2( float* angles_array, uint32_t sides );
  void                    _generate_angles_3( float* angles_array, uint32_t sides );
  void                    _generate_angles_4( float* angles_array, uint32_t sides );
  void                    _generate_angles_5( float* angles_array, uint32_t sides );

  void                    _generate_sizes_1( float* sizes_array, uint32_t sides );
  void                    _generate_sizes_2( float* sizes_array, uint32_t sides );
  void                    _generate_sizes_3( float* sizes_array, uint32_t sides );
  void                    _generate_sizes_4( float* sizes_array, uint32_t sides );
  void                    _generate_sizes_5( float* sizes_array, uint32_t sides );

  static const float      pattern_set_5[5][5];
  static const float      pattern_set_4[5][4];
  static const float      pattern_set_3[5][3];
  static const float      pattern_set_2[5][2];
  static const float      pattern_set_1[5];

  static const float      angle_set_5[5][5];
  static const float      angle_set_4[5][4];
  static const float      angle_set_3[5][3];
  static const float      angle_set_2[5][2];

  std::vector<float3>     n_vertices;
  std::vector<float3>     n_normals;
  std::vector<float2>     n_uvs;
  std::vector<uint32_t>   n_elems;
  uint32_t                n_elem_offset;

  uint32_t                n_sides;
  uint32_t                n_floors;
  float                   n_floor_height;
  float                   n_base_size;
  float                   c_angle;
  float*                  n_angles;
  float*                  n_side_size;
  float3                  c_pos;
  float3                  c_norm;
  float2                  n_uv;
  uint32_t                a_id;
  uint32_t                p_id;
  float                   m_radius;
  float                   uv_x_min;
  float                   uv_y_min;
  float                   uv_x_max;
  float                   uv_y_max;
  float3                  center_pos;
  float3                  center_offset;
};