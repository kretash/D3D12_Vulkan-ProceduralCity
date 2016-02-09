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

#include "base.hh"
#include "engine.hh"
#include "render_manager.hh"

class                       Renderer : public Base {
public:
  Renderer();
  ~Renderer();

  void                            prepare();
  void                            update();
  void                            create( render_type t );
  void                            add_child( Drawable* d );

  Drawable**                      get_render_bin() { return m_render_manager->get_active_render_bin(); }
  int                             get_render_bin_size() { return m_render_manager->get_active_render_bin_size(); }
  bin_offsets*                    get_bin_offsets() { return m_bin_offsets; }
  renderer_data*                  get_render_data() { return &r_data; }
  render_type                     get_renderer_type(){ return m_render_type; }
  
private:
  renderer_data                   r_data;
  render_type                     m_render_type;
  int32_t                         m_render_bin_objects;
  int32_t                         m_cbv_srv_offset;
  std::vector<instance_buffer>    m_instance_buffer;
  RenderManager*                  m_render_manager;
  bin_offsets                     m_bin_offsets[RENDER_BIN_SIZE];
};