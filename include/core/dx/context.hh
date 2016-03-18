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
#include <string>
#include <vector>
#include <wrl.h>
#include "d3dx12.h"
#include <d3d12.h>
#include <dxgi1_4.h>
#include <d3dcompiler.h>

#define m_ptr Microsoft::WRL::ComPtr

namespace                   kretash {

  enum                      render_type;

  enum GraphicsRootsParameters {
    GRP_INSTANCE_CBV = 0,
    GRP_SRV,
    GRP_CONSTANT_CBV,
    GRP_SAMPLER,
    GRP_COUNT,
  };

  class                     dxContext : public virtual xxContext {
  public:
    dxContext();
    ~dxContext();

    friend class            dxGeometry;
    friend class            dxRenderer;
    friend class            dxTexture;
    friend class            dxDescriptorBuffer;
    friend class            dxInterface;

    /* This will create a factory in D3D12 and do nothing in Vulkan */
    virtual void            create_factory() final;

    /* This will create a device, used in both APIs */
    virtual void            create_device() final;

    /* This will create a swap chain and get the queue, used in both APIs */
    virtual void            create_swap_chain( Window* w ) final;

    /* This will create a command pool in Vulkan or command Allocator in D3D12 */
    virtual void            create_command_pool() final;

    /* This will create the buffer command buffer in D3D12 and do nothing in Vulkan*/
    virtual void            create_buffer_command_buffer() final;

    /* This will create the texture command buffer in Vulkan or command lists in D3D12*/
    virtual void            create_texture_command_buffer() final;

    /* This will create the render command buffer in Vulkan or command lists in D3D12*/
    virtual void            create_render_command_buffer() final;

    /* This will create a depth stencil texture and a view in both APIs */
    virtual void            create_depth_stencil( Window* w ) final;

    /* This will create a framebuffer/render target and a view in both APIs */
    virtual void            create_framebuffer( Window* w ) final;

    /* This will create fences in D3D12 and do nothing in Vulkan */
    virtual void            create_fences() final;

    /* This will wait for all setup actions to be completed */
    virtual void            wait_for_setup_completion() final;

    /* This will create the constant buffer object in Vulkan and D3D12 */
    virtual void            create_constant_buffer_object( xxDescriptorBuffer* db, constant_buffer* cb ) final;

    /* This will update the constant buffer object in Vulkan and D3D12 */
    virtual void            update_constant_buffer_object( xxDescriptorBuffer* db, constant_buffer* cb ) final;

    /* This will create shader resource view in D3D12 and do nothing in Vulkan */
    virtual void            create_srv_view_heap( xxRenderer* r ) final;

    /* This will create sampler heap view in D3D12 and do nothing in Vulkan */
    virtual void            create_sampler_view_heap() final;

    /* This will reset the texture command list in Vulkan and D3D12 */
    virtual void            reset_texture_command_list() final;

    /* This will execute the texture command list in Vulkan and D3D12 */
    virtual void            compute_texture_upload() final;

    /* This will wait for texture command list in Vulkan and D3D12 */
    virtual void            wait_for_texture_upload() final;

    /* This will create the command signature in D3D12 and do nothing in D3D12*/
    virtual void            create_indirect_command_signature( xxRenderer* r ) final;

    /* This will create the indirect command buffer in D3D12 and do nothing in D3D12*/
    virtual void            create_indirect_command_buffer( xxRenderer* r, Drawable** draw, uint32_t d_count ) final;

    /* This will update the indirect command buffer in D3D12 and do nothing in D3D12*/
    virtual void            update_indirect_command_buffer( xxRenderer* r, Drawable** draw, uint32_t d_count ) final;

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
    m_ptr<ID3D12Debug>                      m_debug_controller = nullptr;
    m_ptr<IDXGIFactory4>                    factory = nullptr;
    m_ptr<IDXGISwapChain3>                  m_swap_chain = nullptr;
    m_ptr<ID3D12Device>                     m_device = nullptr;
    m_ptr<ID3D12CommandQueue>               m_command_queue = nullptr;

    m_ptr<ID3D12CommandAllocator>           m_buffer_command_allocator = nullptr;
    m_ptr<ID3D12GraphicsCommandList>        m_buffer_command_list = nullptr;

    m_ptr<ID3D12CommandQueue>               m_texture_command_queue = nullptr;
    m_ptr<ID3D12CommandAllocator>           m_texture_command_allocator = nullptr;
    m_ptr<ID3D12GraphicsCommandList>        m_texture_command_list = nullptr;

    m_ptr<ID3D12CommandAllocator>           m_render_command_allocator = nullptr;
    m_ptr<ID3D12GraphicsCommandList>        m_render_command_list = nullptr;

    std::vector<m_ptr<ID3D12Resource>>      m_render_targets;
    m_ptr<ID3D12Resource>                   m_msaa_render_target = nullptr;
    m_ptr<ID3D12Resource>                   m_post_render_target = nullptr;
    m_ptr<ID3D12Resource>                   m_depth_stencil = nullptr;
    m_ptr<ID3D12DescriptorHeap>             m_rtv_heap = nullptr;
    m_ptr<ID3D12DescriptorHeap>             m_dsv_heap = nullptr;
    m_ptr<ID3D12DescriptorHeap>             m_sampler_heap = nullptr;

    uint32_t                                m_rtv_descriptor_size = 0;
    uint32_t                                m_sampler_descriptor_size = 0;
    int32_t                                 m_frame_index = 0;
    HANDLE                                  m_fence_event{};
    m_ptr<ID3D12Fence>                      m_fence = nullptr;
    uint64_t                                m_fence_value = 0;
    uint32_t                                m_pipeline_state_object_id_counter = 0;
    bool                                    m_fence_texture_upload_pending = false;
    uint64_t                                m_fence_texture_upload = 0;
    bool                                    m_msaa_enabled = false;
    int32_t                                 m_mssa_count = 1;
  };
}