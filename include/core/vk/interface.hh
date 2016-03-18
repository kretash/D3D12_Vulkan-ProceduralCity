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
#include "core/xx/interface.hh"
#include "vulkan/vulkan.h"
#include "imgui/imgui.h"

namespace                               kretash {

  class                                 vkInterface : public virtual xxInterface {
  public:
    vkInterface();
    ~vkInterface();

    /* Initialize the interface in Vulkan and D3D12 */
    virtual void                        init( Window* w ) final;

  private:
    static void                         _render( ImDrawData* draw_data );
    
    VkPipeline                          m_pipeline = VK_NULL_HANDLE;
    VkPipelineCache                     m_pipeline_cache = VK_NULL_HANDLE;
    VkPipelineLayout                    m_pipeline_layout = VK_NULL_HANDLE;
    VkDescriptorSet                     m_descriptor_set = VK_NULL_HANDLE;
    VkDescriptorSetLayout               m_descriptor_set_layout = VK_NULL_HANDLE;
    VkDescriptorPool                    m_descriptor_pool = VK_NULL_HANDLE;
    VkVertexInputAttributeDescription   m_vertex_input_attribute[3] = {};
    VkVertexInputBindingDescription     m_vertex_input_binding = {};
    VkRenderPass                        m_render_pass = VK_NULL_HANDLE;
    VkShaderModule                      m_vertex_shader = VK_NULL_HANDLE;
    VkShaderModule                      m_fragment_shader = VK_NULL_HANDLE;
    VkImage                             m_font_image = VK_NULL_HANDLE;
    VkImageView                         m_font_image_view = VK_NULL_HANDLE;
    VkSampler                           m_font_sampler = VK_NULL_HANDLE;
    VkDeviceMemory                      m_font_memory = VK_NULL_HANDLE;
    VkBuffer                            m_render_buffer = VK_NULL_HANDLE;
    VkDeviceMemory                      m_render_buffer_mem = VK_NULL_HANDLE;
  };
}