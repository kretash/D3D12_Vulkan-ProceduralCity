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
#include "core/xx/texture.hh"
#include <wrl.h>
#include "d3dx12.h"
#include <d3d12.h>
#include <dxgi1_4.h>
#include <d3dcompiler.h>

#define m_ptr Microsoft::WRL::ComPtr

namespace                   kretash {

  class                     dxTexture : public virtual xxTexture {
  public:
    dxTexture();
    ~dxTexture();

    /* Creates a texture in Vulkan and D3D12 */
    virtual void            create_texture( void* data, int32_t width, int32_t height, int32_t channels ) final;

    /* Creates a texture view in Vulkan and D3D12 */
    virtual void            create_shader_resource_view( xxRenderer* r, int32_t offset ) final;

    /* Clears the view resources in Vulkan and D3D12 */
    virtual void            clear_texture_upload() final;

    /* Clears the whole texture in Vulkan and D3D12 */
    virtual void            clear_texture() final;

  private:


    m_ptr<ID3D12Resource>                   m_texture;
    m_ptr<ID3D12Resource>                   m_texture_upload;
    uint32_t                                m_width = 0;
    uint32_t                                m_height = 0;
    uint32_t                                m_mip_levels = 0;

  };
}