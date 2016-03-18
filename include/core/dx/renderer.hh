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
#include "core/dx/drawable.hh"
#include <wrl.h>
#include "d3dx12.h"
#include <d3d12.h>
#include <dxgi1_4.h>
#include <d3dcompiler.h>

#define m_ptr Microsoft::WRL::ComPtr

namespace                   kretash {

  struct indirect_command {
    D3D12_GPU_VIRTUAL_ADDRESS               cbv;
    D3D12_DRAW_INDEXED_ARGUMENTS            draw_arguments;
  };

  class                     dxRenderer : public virtual xxRenderer {
  public:

    friend class            dxContext;
    friend class            dxTexture;

    dxRenderer();
    ~dxRenderer();

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

    void _show_shader_error_message( ID3D10Blob* errorMessage, std::wstring filename );
    void _load_and_compile_shaders( render_type s );

    m_ptr<ID3D12CommandSignature>           m_command_signature = nullptr;
    std::vector<indirect_command>           m_indirect_commands = {};
    m_ptr<ID3D12Resource>                   m_command_buffer = nullptr;
    m_ptr<ID3D12Resource>                   m_command_buffer_upload = nullptr;

    m_ptr<ID3D12DescriptorHeap>             m_cbv_heap = nullptr;
    m_ptr<ID3D12DescriptorHeap>             m_srv_heap = nullptr;
    uint32_t                                m_cbv_srv_descriptor_size = 0;

    m_ptr<ID3DBlob>                         vertexShader = nullptr;
    m_ptr<ID3DBlob>                         pixelShader = nullptr;
    m_ptr<ID3D12RootSignature>              m_root_signature = nullptr;
    m_ptr<ID3D12PipelineState>              m_pipeline_state = nullptr;
    uint32_t                                m_pipeline_state_id = 0;

    m_ptr<ID3D12Resource>                   m_instance_buffer = nullptr;
    uint8_t*                                m_instance_buffer_WO = nullptr;
    D3D12_CONSTANT_BUFFER_VIEW_DESC         m_instance_buffer_desc = {};

    m_ptr<ID3D12Resource>                   m_constant_buffer = nullptr;
    uint8_t*                                m_constant_buffer_WO = nullptr;
    D3D12_CONSTANT_BUFFER_VIEW_DESC         m_constant_buffer_desc {};
  };
}