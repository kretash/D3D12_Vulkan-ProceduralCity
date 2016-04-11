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

#include "math/float3.hh"
#include "math/float4x4.hh"
#include "base.hh"

namespace kretash {
  class           Camera : public Base {
  public:
    Camera();
    ~Camera();

    void          init();
    void          init( float aspect_ratio, float fov, float znear, float zfar );
    void          update();


    void          api_swap();
    void          set_cinematic_camera( bool c ) { m_cinematic_camera = c; }
    bool          get_cinematic_camera() { return m_cinematic_camera; }
    float4x4      get_view() { return m_view; }
    float4x4      get_projection() { return m_projection; }
    float3        get_position() { return m_eye; }
    float3        get_look_direction() { return m_look_dir; }

  private:
    float4x4      m_view;
    float4x4      m_projection;

    bool          m_cinematic_camera;
    void          _controlled_camera();

    // Cinematic camera and sub functions
    void          _cinematic_camera();
    void          _barrel_roll( std::vector<float>* spectrum );
    bool          m_barrel_rolling;
    float         m_last_barrel_roll;
    void          _loop_the_loop( std::vector<float>* spectrum );
    bool          m_looping_the_loop;
    float         m_last_loop_the_loop;

    float3        m_eye;
    float3        m_focus;
    float3        m_up;
    float3        m_look_dir;
    float3        m_move_dir;

    float         m_aspect_ratio;
    float         m_fov;
    float         m_znear;
    float         m_zfar;
  };
}