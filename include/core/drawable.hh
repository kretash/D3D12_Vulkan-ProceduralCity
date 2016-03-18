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
#include "xx/drawable.hh"
#include "geometry.hh"
#include "texture.hh"
#include "camera.hh"
#include "base.hh"

namespace kretash {
  class                             Drawable : public Base {
  public:
    Drawable();
    ~Drawable();

    void                            init( Geometry* g );
    void                            prepare();
    void                            update();

    void                            set_position( float x, float y, float z );
    void                            set_position( float3 pos );
    void                            scale( float x, float y, float z );
    void                            rotate( float x, float y, float z );
    void                            set_lod( int32_t  i );
    void                            set_offsets( int32_t  cvb, int32_t  srv );
    void                            set_frustum_size( float radious, float max_height );
    void                            set_ignore_frustum();
    void                            set_active( bool a ) { m_in_frustum = a; }
    void                            set_distance( float d ) { m_distance = d; }

    int32_t                         get_drawable_id() { return drawable_id; }
    xxDrawable*                     get_drawable() { return m_drawable.get(); }
    xxDescriptorBuffer*             get_buffer() { return m_buffer.get(); }
    instance_buffer*                get_instance_buffer() { return &m_instance_buffer; }
    void                            set_instance_buffer( instance_buffer* i ) { memcpy( &m_instance_buffer, i, sizeof( instance_buffer ) ); }
    Texture*                        get_texture() { return m_texture.get(); }
    Geometry*                       get_geometry() { return m_geometry[m_geo_lod].get(); }
    Geometry*                       get_geometry( int32_t  lod ) { return m_geometry[lod].get(); }
    float3                          get_position() { return m_position; }
    float4x4                        get_model();
    int32_t                         get_cvb_offset() { return m_offsets.cbv_offset; }
    int32_t                         get_srv_offset() { return m_offsets.srv_offset; }
    const float                     get_max_height() const { return m_max_height; }
    const float                     get_radius() const { return m_radius; }
    const float                     get_distance() const { return m_distance; }
    const bool                      get_active() const { return m_in_frustum; }

  protected:
    int32_t                         drawable_id;
    float                           m_distance;
    float                           m_radius;
    bool                            m_in_frustum;
    int32_t                         m_has_lod;
    int32_t                         m_geo_lod;
    float                           m_max_height;

    std::shared_ptr<xxDrawable>     m_drawable;
    std::shared_ptr<xxDescriptorBuffer>       m_buffer;
    std::shared_ptr<Texture>        m_texture;
    std::shared_ptr<Geometry>       m_geometry[3];
    bin_offsets                     m_offsets;
    instance_buffer                 m_instance_buffer;
    float4x4                        m_model;
    float3                          m_position;
    float3                          m_scale;
    float3                          m_rotation;
  };
}