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

namespace                   kretash {

  class                     xxRenderer;

  class                     xxTexture {
  public:
    xxTexture() {}
    ~xxTexture() {}

    /* Creates a texture in Vulkan and D3D12 */
    virtual void            create_texture( void* data, int32_t width, int32_t height, int32_t channels ) {};

    /* Creates a texture view in Vulkan and D3D12 */
    virtual void            create_shader_resource_view( xxRenderer* r, int32_t offset, int32_t channels ) {};

    /* Clears the view resources in Vulkan and D3D12 */
    virtual void            clear_texture_upload() {};

    /* Clears the whole texture in Vulkan and D3D12 */
    virtual void            clear_texture() {};
    
    /* Removes the texture from the descriptor set in Vulkan and D3D12 */
    virtual void            clear_descriptor_set( xxTexture* other, int32_t offset ) {};

  };
}