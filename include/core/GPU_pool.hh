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
#include <memory>
#include <atomic>
#include <mutex>

#include "base.hh"

class                               Geometry;
struct                              geometry_data;
struct                              queue;
struct                              remove_queue;
struct                              mem_block;

class                               GPU_pool : public Base {
public:
  GPU_pool();
  ~GPU_pool();

  void                              init();
  void                              update();

  void                              queue_geometry( Geometry* b, float* v_data, uint32_t v_size, 
                                      uint32_t* e_data, uint32_t e_size );
  void                              set_placeholder_building( Geometry* b );
  Geometry*                         get_placeholder_building() { return m_placeholder_building; }
  void                              remove( Geometry* b );
  void                              start_remove_thread();
  geometry_data*                    get_geometry_data() { return g_data.get(); }

private:
  void                              _debug_log();
  void                              _save( Geometry* b, float* v_data, uint32_t v_size, uint32_t* e_data, 
                                      uint32_t e_size );
  void                              _remove( remove_queue remove_me );
  void                              _defrag_vectors();
  void                              _remove_thread();

  std::shared_ptr<geometry_data>    g_data;
  Geometry*                         m_placeholder_building;
  uint32_t                          m_vertex_pointer;
  uint32_t                          m_index_pointer;
  int32_t                           m_instances;
  uint32_t                          m_max_vertex_buffer;
  uint32_t                          m_max_index_buffer;

  std::vector<queue>                m_upload_queue;

  std::atomic_bool                  m_removing_geometry;
  std::mutex                        m_pool_mutex;
  std::vector<std::thread>          m_remove_threads;
  std::vector<remove_queue>         m_remove_queue;

  std::vector<mem_block>            m_V_free_memory;
  std::vector<mem_block>            m_V_used_memory;
  std::vector<mem_block>            m_I_free_memory;
  std::vector<mem_block>            m_I_used_memory;
};