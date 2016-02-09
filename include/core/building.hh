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
#include <atomic>
#include <thread>
#include <mutex>

#include "building_gen.hh"
#include "drawable.hh"

class                                 OpenSimplexNoise;

class                                 Building : public Drawable {
public:
  Building( bool placeholder = false );
  ~Building();

  //prepare to generate, reuse safe (should be)
  void                                prepare( float seed_x, float seed_y );

  //main heavy function
  void                                generate();

  //quick clean and upload
  void                                upload_and_clean();

  void                                clear();
  bool                                is_empty() { return m_empty; }
  bool                                is_ready_to_process() { return m_ready_to_process; }
  void                                set_ready_to_process( bool r ) { m_ready_to_process = r; }
  void                                generate_placeholder();

private:

  void                                _generate_classic_building();
  void                                _generate_modern_building();
  void                                _generate_factory();

  void                                _init_noise( float seed_x, float seed_y );
  int32_t                             _get_p_rand( int32_t min, int32_t max, float sample );
  float                               _get_p_rand( float min, float max, float sample );

  std::shared_ptr<BuildingGen>        m_building_generator_LOD0;
  std::shared_ptr<BuildingGen>        m_building_generator_LOD1;
  std::shared_ptr<BuildingGen>        m_building_generator_LOD2;
  std::shared_ptr<OpenSimplexNoise>   m_noise_handle;

  bool                                m_empty;
  bool                                m_ready_to_process;
  float                               m_noise;
  int32_t                             m_num_floors;
  int32_t                             m_num_sides;
  int32_t                             m_instances;
  int32_t*                            m_angle_s;
  int32_t                             m_side_neibours;

  float                               m_base_size;
  float                               m_instance_decrement;
  float                               m_ground_height;
  float                               m_ground_size_offset;
  float                               m_ground_multiplier;
  float                               m_spacers_size;
  float                               m_side_neibours_size;

  groups                              m_iteration_group;
  texture_set                         m_main_texture_set;
  texture_set                         m_roof_texture_set;

};