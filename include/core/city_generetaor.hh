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
#include "types.hh"
#include <atomic>
#include <vector>
#include <memory>
#include <mutex>
#include <queue>
#include <map>

class Renderer;
class Building;
class Drawable;
class Geometry;
class Texture;

class                                         CityGenerator {
public:

  CityGenerator();
  ~CityGenerator();

  void                                        generate( std::shared_ptr<Renderer> ren );
  void                                        update();
private:

  void                                        _prepare_vectors();
  void                                        _generate_move_buildings();
  void                                        _apply_move_buildings( uint32_t count );

  void                                        _apply_carry_to_outline( building_details outline );

  outline_type                                _oposite( outline_type s );

  //threaded function
  void _generate_loop();
  std::vector<Building*>                      m_to_generate;
  std::mutex                                  m_to_generate_lock;
  std::vector<Building*>                      m_to_upload;
  std::mutex                                  m_to_upload_lock;
  std::vector<std::thread>                    m_threads;
  std::atomic_bool                            m_exit_threads;

  int32_t                                     m_count;
  int32_t                                     m_grid;
  int32_t                                     m_half_grid;
  float                                       m_scale;
  float                                       m_max_radius;

  std::vector<std::shared_ptr<Building>>      m_buildigs;
  std::vector<std::shared_ptr<Drawable>>      m_street_block_D;
  std::shared_ptr<Renderer>                   m_renderer;
  std::shared_ptr<Building>                   m_placeholder_building;
  std::shared_ptr<Texture>                    m_texture;
  std::shared_ptr<Geometry>                   m_street_block;

  std::map<int32_t, float3>                   m_building_positions;
  std::vector<building_details>               m_all_buildings;
  std::vector<building_details>               m_outline_positions;
  std::vector<move_operation>                 m_move_operations;
};