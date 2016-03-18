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
#include <memory>
#include <vector>
#include "core/math/float4x4.hh"

namespace                   kretash {

  struct                                            instance_buffer {
    float4x4                                        mvp;
    float4x4                                        model;
    float4x4                                        normal_matrix;

    uint32_t                                        d_texture_id;
    uint32_t                                        n_texture_id;
    uint32_t                                        s_texture_id;

    uint32_t                                        pad[13];

    instance_buffer() :
      d_texture_id( 0 ),
      n_texture_id( 0 ),
      s_texture_id( 0 ) {
    }
  };

  struct                                            constant_buffer {
    float3                                          light_pos;
    float                                           sky_color;
    float3                                          fog_color;
    float                                           ambient_intensity;
    float3                                          sun_light_intensity;
    float                                           padding;
    float3                                          eye_view;
    float4x4                                        view;

    constant_buffer() {}
    ~constant_buffer() {}
  };

  class                     xxDescriptorBuffer {
  public:
    xxDescriptorBuffer() {}
    ~xxDescriptorBuffer() {}

    /* needs at least one function to be polymorphic */
    virtual void            do_nothing() {};
    
  };

  class                     xxDrawable {
  public:
    xxDrawable() {}
    ~xxDrawable() {}

    /* needs at least one function to be polymorphic */
    virtual void            do_nothing() {};

  };
}