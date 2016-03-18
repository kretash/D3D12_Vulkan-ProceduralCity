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
#include "core/xx/drawable.hh"

namespace                   kretash {

  class                     Drawable;
  enum                      render_type;

  class                     xxRenderer {
  public:
    xxRenderer() {}
    ~xxRenderer() {}
    
    /* This will create the instance buffers object in Vulkan and D3D12 */
    virtual void            create_instance_buffer_objects( std::vector<Drawable*>* d ) {};

    /* This will create the instance buffers object view in D3D12 and do nothing in Vulkan*/
    virtual void            create_instance_buffer_object_view( Drawable* d, uint32_t offset ) {};

    /* This will update the instance buffers object in Vulkan and D3D12 */
    virtual void update_instance_buffer_objects( std::vector<Drawable*>* d, std::vector<instance_buffer>* ib ) {};

    /* This will create the root signature in Vulkan and D3D12 */
    virtual void            create_root_signature() {};

    /* This will create the graphics pipeline in Vulkan and D3D12 */
    virtual void            create_graphics_pipeline( render_type rt ) {};


  };
}