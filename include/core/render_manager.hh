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
#include "core/drawable.hh"
#include <vector>

class                     RenderManager {
public:
  RenderManager();
  ~RenderManager();

  void                    add_child( Drawable* d );
  void                    update( float df );

  int32_t                 get_render_bin_size() { return static_cast< int32_t >( m_render_bin.size() ); }
  Drawable**              get_render_bin() { return &m_render_bin[0]; }

  int32_t                 get_active_render_bin_size() { return static_cast< int32_t >( m_active_render_bin.size() ); }
  Drawable**              get_active_render_bin() { return &m_active_render_bin[0]; }

private:
  bool                    _inside_frustum( float3 point, float r, float maxh );

  void                    _generate_frustum_planes();
  plane                   m_frustum_planes[6];

  std::vector<Drawable*>  m_render_bin;
  std::vector<Drawable*>  m_active_render_bin;
};