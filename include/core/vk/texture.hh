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
#include "vulkan/vulkan.h"
#include "core/types.hh"

namespace                   kretash {

  class                     vkTexture : public virtual xxTexture {
  public:
    vkTexture();
    ~vkTexture();

    /* Creates a texture in Vulkan and D3D12 */
    virtual void            create_texture( void* data, int32_t width, int32_t height, int32_t channels ) final;

    /* Creates a texture view in Vulkan and D3D12 */
    virtual void            create_shader_resource_view( xxRenderer* r, int32_t offset ) final;

    /* Clears the view resources in Vulkan and D3D12 */
    virtual void            clear_texture_upload() final;

    /* Clears the whole texture in Vulkan and D3D12 */
    virtual void            clear_texture() final;

    /* Removes the texture from the descriptor set in Vulkan and D3D12 */
    virtual void            clear_descriptor_set( xxTexture* other, int32_t offset ) final;
  private:

    VkSampler                                       m_sampler = VK_NULL_HANDLE;
    VkImage                                         m_image = VK_NULL_HANDLE;
    VkImage                                         m_mappable_image = VK_NULL_HANDLE;
    VkImageLayout                                   m_image_layout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkImageView                                     m_view = VK_NULL_HANDLE;
    uint32_t                                        m_width = 0;
    uint32_t                                        m_height = 0;
    uint32_t                                        m_mip_levels = 0;

    mem_block                                       m_mappable_mem_block = {};
    mem_block                                       m_image_mem_block = {};

  };
}