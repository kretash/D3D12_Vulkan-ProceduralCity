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
#include "core/xx/renderer.hh"
#include "core/vk/drawable.hh"
#include "vulkan/vulkan.h"

namespace                   kretash {

  class                     vkRenderer : public virtual xxRenderer {
  public:

    friend class            vkContext;

    vkRenderer();//TODO
    ~vkRenderer();//TODO

    /* This will create the instance buffers object in Vulkan and D3D12 */
    virtual void            create_instance_buffer_objects( std::vector<Drawable*>* d ) final;

    /* This will create the instance buffer object view in D3D12 and do nothing in Vulkan*/
    virtual void            create_instance_buffer_object_view( Drawable* d, uint32_t offset ) final;

    /* This will update the instance buffers object in Vulkan and D3D12 */
    virtual void update_instance_buffer_objects( std::vector<Drawable*>* d, std::vector<instance_buffer>* ib ) final;

    /* This will create the root signature in Vulkan and D3D12 */
    virtual void            create_root_signature() final;

    /* This will create the graphics pipeline in Vulkan and D3D12 */
    virtual void            create_graphics_pipeline( render_type rt ) final;

  private:

    VkPipelineShaderStageCreateInfo                 _load_shaders( std::string filename, VkShaderStageFlagBits flags );

    std::vector<VkShaderModule>                     m_shader_modules = {};
    VkPipeline                                      m_pipeline = VK_NULL_HANDLE;
    VkPipelineVertexInputStateCreateInfo            m_vi = {};
    std::vector<VkVertexInputBindingDescription>    m_binding_descriptions = {};
    std::vector<VkVertexInputAttributeDescription>  m_attribute_descriptions = {};
    vkDescriptorBuffer                              m_uniform_buffer = {};
  };
}