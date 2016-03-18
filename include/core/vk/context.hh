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
#include "core/xx/context.hh"
#include "vulkan/vulkan.h"
#include "vulkan/vulkantools.h"
#include "vulkan/vulkanswapchain.hpp"

namespace                   kretash {

  enum                      render_type;
  class                     Pool;

  class                     vkContext : public virtual xxContext {
  public:
    vkContext();
    ~vkContext();

    friend class            vkGeometry;
    friend class            vkRenderer;
    friend class            vkTexture;
    friend class            vkDescriptorBuffer;
    friend class            vkInterface;

    /* This will create the Vulkan Instance and do nothing in D3D12 */
    virtual void            create_instance() final;

    /* This will create a device, used in both APIs */
    virtual void            create_device() final;

    /* This will create a swap chain, used in both APIs */
    virtual void            create_swap_chain( Window* w ) final;

    /* This will create a command pool in Vulkan or command Allocator in D3D12 */
    virtual void            create_command_pool() final;

    /* This will create the setup command buffer in Vulkan or command lists in D3D12*/
    virtual void            create_setup_command_buffer() final;

    /* This will create a swap chain and get the queue, used in both APIs */
    virtual void            setup_swap_chain( Window* w ) final;

    /* This will create the texture command buffer in Vulkan or command lists in D3D12*/
    virtual void            create_texture_command_buffer() final;

    /* This will create the render command buffer in Vulkan or command lists in D3D12*/
    virtual void            create_render_command_buffer() final;
    
    /* This will create a render pass in Vulkan and do nothing in D3D12 */
    virtual void            create_render_pass() final;

    /* This will create a depth stencil texture and a view in both APIs */
    virtual void            create_depth_stencil( Window* w ) final;

    /* This will create a framebuffer/render target and a view in both APIs */
    virtual void            create_framebuffer( Window* w ) final;

    /* This will create a pipeline cache in Vulkan and do nothing in D3D12 */
    virtual void            create_pipeline_cache() final;

    /* This will wait for all setup actions to be completed */
    virtual void            wait_for_setup_completion() final;

    /* This will allocate memory for texture upload in Vulkan and do nothing in D3D12 */
    virtual void            allocate_device_memory( uint64_t size ) final;

    /* This will allocate memory for texture upload in Vulkan and do nothing in D3D12 */
    virtual void            allocate_host_memory( uint64_t size ) final;

    /* This will create a descriptor set layout in Vulkan and do nothing in D3D12 */
    virtual void            create_descriptor_set_layout() final;

    /* This will create descriptor pool in Vulkan and do nothing in D3D12 */
    virtual void            create_descriptor_pool() final;

    /* This will create the constant buffer object in Vulkan and D3D12 */
    virtual void            create_constant_buffer_object( xxDescriptorBuffer* db, constant_buffer* cb ) final;

    /* This will update the constant buffer object in Vulkan and D3D12 */
    virtual void            update_constant_buffer_object( xxDescriptorBuffer* db, constant_buffer* cb ) final;

    /* This will reset the texture command list in Vulkan and D3D12 */
    virtual void            reset_texture_command_list() final;

    /* This will execute the texture command list in Vulkan and D3D12 */
    virtual void            compute_texture_upload() final;

    /* This will wait for texture command list in Vulkan and D3D12 */
    virtual void            wait_for_texture_upload() final;

    /* This will defrag the host memory pool in Vulkan and do nothing is D3D12 */
    virtual void            defrag_host_memory_pool() final;

    /* This will defrag the device memory pool in Vulkan and do nothing is D3D12 */
    virtual void            defrag_device_memory_pool() final;

    /* This will reset the render command list in Vulkan and D3D12 */
    virtual void            reset_render_command_list( Window* w ) final;

    /* This will clear the color buffer in Vulkan and D3D12 */
    virtual void            clear_color() final;

    /* This will clear the depth in Vulkan and D3D12 */
    virtual void            clear_depth() final;

    /* This will record commands list in Vulkan and D3D12 */
    virtual void            record_commands( xxRenderer* r, Window* w, Drawable** draw, uint32_t d_count ) final;

    /* This will record indirect commands list in D3D12 and normal ones in Vulkan */
    virtual void            record_indirect_commands( xxRenderer* r, Window* w, Drawable** draw, uint32_t d_count ) final;

    /* This will execute the command list in Vulkan and D3D12 */
    virtual void            execute_render_command_list() final;

    /* This will present the swap chain in Vulkan and D3D12 */
    virtual void            present_swap_chain() final;

    /* This will wait for render to finish in Vulkan and D3D12 */
    virtual void            wait_render_completition() final;

  private:

    bool _get_memory_type( uint32_t typeBits, VkFlags properties, uint32_t * typeIndex );

    struct depth_stencil {
      VkImage                                       m_image = VK_NULL_HANDLE;
      VkDeviceMemory                                m_mem = VK_NULL_HANDLE;
      VkImageView                                   m_view = VK_NULL_HANDLE;
    };

    VkInstance                                      m_instance = VK_NULL_HANDLE;
    VkPhysicalDevice                                m_physical_device = VK_NULL_HANDLE;
    VkPhysicalDeviceMemoryProperties                m_device_memory_properties = {};
    VkDevice                                        m_device = VK_NULL_HANDLE;
    uint32_t                                        m_graphics_queue_index = 0;
    VkQueue                                         m_queue = VK_NULL_HANDLE;
    uint32_t                                        m_transfer_queue_index = 0;
    VkQueue                                         m_texture_queue = VK_NULL_HANDLE;
    VulkanSwapChain                                 m_swap_chain = {};
    VkFormat                                        m_depth_format = {};
    VkCommandPool                                   m_command_pool = VK_NULL_HANDLE;
    VkCommandBuffer                                 m_setup_command_buffer = VK_NULL_HANDLE;
    VkCommandBuffer                                 m_texture_command_buffer = VK_NULL_HANDLE;
    VkCommandBuffer                                 m_post_present_command_buffer = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer>                    m_draw_command_buffers;
    depth_stencil                                   m_depth_stencil = {};
    VkRenderPass                                    m_render_pass = VK_NULL_HANDLE;
    std::vector<VkFramebuffer>                      m_framebuffers;
    VkPipelineCache                                 m_pipeline_cache = VK_NULL_HANDLE;
    uint32_t                                        m_current_buffer = 0;
    std::vector<VkShaderModule>                     m_shader_modules;
    VkDescriptorSetLayout                           m_instance_descriptor_set_layout = VK_NULL_HANDLE;
    VkDescriptorSetLayout                           m_constant_descriptor_set_layout = VK_NULL_HANDLE;
    VkDescriptorSet                                 m_constant_descriptor_set = VK_NULL_HANDLE;
    VkDescriptorPool                                m_descriptor_pool = VK_NULL_HANDLE;
    VkPipelineLayout                                m_pipeline_layout = VK_NULL_HANDLE;
    VkDeviceMemory                                  m_device_pool_memory = VK_NULL_HANDLE;
    VkDeviceMemory                                  m_host_pool_memory = VK_NULL_HANDLE;
    std::shared_ptr<Pool>                         m_vk_device_pool;
    std::shared_ptr<Pool>                         m_vk_host_pool;
  };
}