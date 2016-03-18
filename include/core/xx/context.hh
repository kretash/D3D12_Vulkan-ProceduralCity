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
#include "core/math/float4x4.hh"
#include "core/math/float3.hh"

namespace                   kretash {

  enum                      render_type;
  struct                    constant_buffer;
  class                     Window;
  class                     Drawable;
  class                     xxRenderer;
  class                     xxDescriptorBuffer;

  class                     xxContext {
  public:
    xxContext() {}
    ~xxContext() {}

    /* This will create the Vulkan Instance and do nothing in D3D12 */
    virtual void            create_instance() {};

    /* This will create a factory in D3D12 and do nothing in Vulkan */
    virtual void            create_factory() {};

    /* This will create a device, used in both APIs */
    virtual void            create_device() {};

    /* This will create a swap chain and get the queue, used in both APIs */
    virtual void            create_swap_chain( Window* w ) {};

    /* This will create a command pool in Vulkan or command Allocator in D3D12 */
    virtual void            create_command_pool() {};

    /* This will create the setup command buffer in Vulkan and do nothing in D3D12*/
    virtual void            create_setup_command_buffer() {};

    /* This will set up the swap chain in Vulkan and do nothing in D3D12 */
    virtual void            setup_swap_chain( Window* w ) {};

    /* This will create the buffer command buffer in D3D12 and do nothing in Vulkan*/
    virtual void            create_buffer_command_buffer() {};

    /* This will create the texture command buffer in Vulkan or command list in D3D12*/
    virtual void            create_texture_command_buffer() {};

    /* This will create the render command buffer in Vulkan or command list in D3D12*/
    virtual void            create_render_command_buffer() {};

    /* This will create a render pass in Vulkan and do nothing in D3D12 */
    virtual void            create_render_pass() {};

    /* This will create a depth stencil texture and a view in both APIs */
    virtual void            create_depth_stencil( Window* w ) {};

    /* This will create a framebuffer/render target and a view in both APIs */
    virtual void            create_framebuffer( Window* w ) {};

    /* This will create a pipeline cache in Vulkan and do nothing in D3D12 */
    virtual void            create_pipeline_cache() {};

    /* This will create fences in D3D12 and do nothing in Vulkan */
    virtual void            create_fences() {};

    /* This will wait for all setup actions to be completed */
    virtual void            wait_for_setup_completion() {};

    /* This will allocate memory for texture upload in Vulkan and do nothing in D3D12 */
    virtual void            allocate_device_memory( uint64_t size ) {};

    /* This will allocate memory for texture upload in Vulkan and do nothing in D3D12 */
    virtual void            allocate_host_memory( uint64_t size ) {};

    /* This will create a descriptor set layout in Vulkan and do nothing in D3D12 */
    virtual void            create_descriptor_set_layout() {};

    /* This will create descriptor pool in Vulkan and do nothing in D3D12 */
    virtual void            create_descriptor_pool() {};

    /* This will create the constant buffer object in Vulkan and D3D12 */
    virtual void            create_constant_buffer_object( xxDescriptorBuffer* db, constant_buffer* cb ) {};

    /* This will update the constant buffer object in Vulkan and D3D12 */
    virtual void            update_constant_buffer_object( xxDescriptorBuffer* db, constant_buffer* cb ) {};

    /* This will create shader resource view in D3D12 and do nothing in Vulkan */
    virtual void            create_srv_view_heap( xxRenderer* r ) {};

    /* This will create sampler heap view in D3D12 and do nothing in Vulkan */
    virtual void            create_sampler_view_heap( ) {};

    /* This will reset the texture command list in Vulkan and D3D12 */
    virtual void            reset_texture_command_list() {};

    /* This will execute the texture command list in Vulkan and D3D12 */
    virtual void            compute_texture_upload() {};

    /* This will wait for texture command list in Vulkan and D3D12 */
    virtual void            wait_for_texture_upload() {};

    /* This will defrag the host memory pool in Vulkan and do nothing is D3D12 */
    virtual void            defrag_host_memory_pool() {};

    /* This will defrag the device memory pool in Vulkan and do nothing is D3D12 */
    virtual void            defrag_device_memory_pool() {};

    /* This will create the command signature in D3D12 and do nothing in D3D12*/
    virtual void            create_indirect_command_signature( xxRenderer* r ) {};

    /* This will create the indirect command buffer in D3D12 and do nothing in D3D12*/
    virtual void            create_indirect_command_buffer( xxRenderer* r, Drawable** draw, uint32_t d_count ) {};

    /* This will update the indirect command buffer in D3D12 and do nothing in D3D12*/
    virtual void            update_indirect_command_buffer( xxRenderer* r, Drawable** draw, uint32_t d_count ) {};

    /* This will reset the render command list in Vulkan and D3D12 */
    virtual void            reset_render_command_list( Window* w ) {};

    /* This will clear the color buffer in Vulkan and D3D12 */
    virtual void            clear_color() {};

    /* This will clear the depth in Vulkan and D3D12 */
    virtual void            clear_depth() {};

    /* This will record commands list in Vulkan and D3D12 */
    virtual void            record_commands( xxRenderer* r, Window* w, Drawable** draw, uint32_t d_count ) {};

    /* This will record indirect commands list in D3D12 and normal ones in Vulkan */
    virtual void            record_indirect_commands( xxRenderer* r, Window* w, Drawable** draw, uint32_t d_count ) {};

    /* This will execute the command list in Vulkan and D3D12 */
    virtual void            execute_render_command_list() {};

    /* This will present the swap chain in Vulkan and D3D12 */
    virtual void            present_swap_chain() {};

    /* This will wait for render to finish in Vulkan and D3D12 */
    virtual void            wait_render_completition() {};

  protected:
    const int32_t           m_stride = 14;
    const uint32_t          m_frame_count = 2;
    const int32_t           m_max_textures = 2048;
  };
}