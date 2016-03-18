#include "core/dx/interface.hh"
#include "core/window.hh"
#include "core/dx/context.hh"
#include "core/engine.hh"
#include "core/input.hh"

namespace kretash {
  dxInterface::dxInterface() {

  }

  dxInterface::~dxInterface() {

  }

  /* Initialize the interface in Vulkan and D3D12 */
  void dxInterface::init( Window* w ) {

    ImGuiIO& io = ImGui::GetIO();
    io.ImeWindowHandle = w->get_window_handle();
    io.RenderDrawListsFn = _render;
    io.UserData = this;

    HRESULT result;
    dxContext* m_context = dynamic_cast< dxContext* >( k_engine->get_context() );

    io.DisplaySize.x = static_cast< float >( k_engine->get_window()->get_width() );
    io.DisplaySize.y = static_cast< float >( k_engine->get_window()->get_height() );

    // Create one shader resource descriptor for the font texture
    D3D12_DESCRIPTOR_HEAP_DESC srv_desc_heap = {};
    srv_desc_heap.NumDescriptors = 1;
    srv_desc_heap.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srv_desc_heap.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    srv_desc_heap.NodeMask = 0;
    result = m_context->m_device->CreateDescriptorHeap( &srv_desc_heap, IID_PPV_ARGS( &m_srv_heap ) );
    assert( result == S_OK && "ERROR CREATING THE DESCRIPTOR SET" );

    D3D12_DESCRIPTOR_RANGE desc_range[] =
    { { D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND } };

    CD3DX12_ROOT_PARAMETER root_parameters[2];
    root_parameters[0].InitAsDescriptorTable( 1, &desc_range[0], D3D12_SHADER_VISIBILITY_ALL );
    root_parameters[1].InitAsConstantBufferView( 0, 0, D3D12_SHADER_VISIBILITY_ALL );

    D3D12_STATIC_SAMPLER_DESC sampler_desc = {};
    sampler_desc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    sampler_desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler_desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler_desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler_desc.MipLODBias = 0.f;
    sampler_desc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    sampler_desc.MinLOD = 0.f;
    sampler_desc.MaxLOD = 0.f;
    sampler_desc.ShaderRegister = 0;
    sampler_desc.RegisterSpace = 0;
    sampler_desc.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    D3D12_ROOT_SIGNATURE_DESC root_signature_desc;
    root_signature_desc.NumParameters = 2;
    root_signature_desc.pParameters = root_parameters;
    root_signature_desc.NumStaticSamplers = 1;
    root_signature_desc.pStaticSamplers = &sampler_desc;
    root_signature_desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    m_ptr<ID3DBlob> signature;
    m_ptr<ID3DBlob> error;
    result = D3D12SerializeRootSignature( &root_signature_desc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error );
    assert( result == S_OK && "ERROR SERIALIZING THE ROOT SIGNATURE" );

    // Create the root signature using the binary blob
    result = m_context->m_device->CreateRootSignature( 0, signature->GetBufferPointer(), signature->GetBufferSize(),
      IID_PPV_ARGS( &m_root_signature ) );
    assert( result == S_OK && "ERROR CREATING THE ROOT SIGNATURE" );

    static const char* v_shader =
      "cbuffer vertexBuffer : register(b0) \
        {\
        float4x4 ProjectionMatrix; \
        };\
        struct VS_INPUT\
        {\
        float2 pos : POSITION;\
        float4 col : COLOR0;\
        float2 uv  : TEXCOORD0;\
        };\
        \
        struct PS_INPUT\
        {\
        float4 pos : SV_POSITION;\
        float4 col : COLOR0;\
        float2 uv  : TEXCOORD0;\
        };\
        \
        PS_INPUT main(VS_INPUT input)\
        {\
        PS_INPUT output;\
        output.pos = mul( ProjectionMatrix, float4(input.pos.xy, 0.f, 1.f));\
        output.col = input.col;\
        output.uv  = input.uv;\
        return output;\
        }";

    static const char* p_shader =
      "struct PS_INPUT\
        {\
        float4 pos : SV_POSITION;\
        float4 col : COLOR0;\
        float2 uv  : TEXCOORD0;\
        };\
        SamplerState sampler0 : register(s0);\
        Texture2D texture0 : register(t0);\
        \
        float4 main(PS_INPUT input) : SV_Target\
        {\
        float4 out_col = input.col* texture0.Sample(sampler0, input.uv); \
        return out_col; \
        }";

    ID3DBlob*     vertex_blob = 0;
    ID3DBlob*     pixel_blob = 0;
    ID3DBlob*     error_blob = 0;

    D3D12_INPUT_ELEMENT_DESC input_elements_desc[] =
    {
      { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, ( size_t ) ( &( ( ImDrawVert* ) 0 )->pos ),
      D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
      { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, ( size_t ) ( &( ( ImDrawVert* ) 0 )->uv ),
      D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
      { "COLOR",    0, DXGI_FORMAT_R8G8B8A8_UNORM,     0, ( size_t ) ( &( ( ImDrawVert* ) 0 )->col ),
      D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    result = D3DCompile( v_shader, strlen( v_shader ), NULL, NULL, NULL, "main", "vs_5_0",
      D3DCOMPILE_DEBUG, 0, &vertex_blob, &error_blob );
    result = D3DCompile( p_shader, strlen( p_shader ), NULL, NULL, NULL, "main", "ps_5_0",
      D3DCOMPILE_DEBUG, 0, &pixel_blob, &error_blob );

    D3D12_RASTERIZER_DESC rasterizer_desc = {};
    rasterizer_desc.FillMode = D3D12_FILL_MODE_SOLID;
    rasterizer_desc.CullMode = D3D12_CULL_MODE_NONE;
    rasterizer_desc.FrontCounterClockwise = true;
    rasterizer_desc.DepthBias = 0;
    rasterizer_desc.DepthBiasClamp = 0.f;
    rasterizer_desc.SlopeScaledDepthBias = 0.f;
    rasterizer_desc.DepthClipEnable = true;
    rasterizer_desc.MultisampleEnable = false;
    rasterizer_desc.AntialiasedLineEnable = true;
    rasterizer_desc.ForcedSampleCount = 1;
    rasterizer_desc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

    D3D12_BLEND_DESC blend_desc;
    blend_desc.AlphaToCoverageEnable = false;
    blend_desc.IndependentBlendEnable = false;
    blend_desc.RenderTarget[0].BlendEnable = true;
    blend_desc.RenderTarget[0].LogicOpEnable = false;
    blend_desc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
    blend_desc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
    blend_desc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
    blend_desc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
    blend_desc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
    blend_desc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
    blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { input_elements_desc, _countof( input_elements_desc ) };
    psoDesc.pRootSignature = m_root_signature.Get();
    psoDesc.VS = { ( uint8_t* ) ( vertex_blob->GetBufferPointer() ), vertex_blob->GetBufferSize() };
    psoDesc.PS = { ( uint8_t* ) ( pixel_blob->GetBufferPointer() ), pixel_blob->GetBufferSize() };
    psoDesc.RasterizerState = rasterizer_desc;
    psoDesc.BlendState = blend_desc;
    psoDesc.DepthStencilState.DepthEnable = FALSE;
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc.Count = 1;
    m_context->m_device->CreateGraphicsPipelineState( &psoDesc, IID_PPV_ARGS( &m_pipeline_state ) );

    {
      D3D12_RESOURCE_DESC upload_buffer_desc;
      upload_buffer_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
      upload_buffer_desc.Alignment = 0;
      upload_buffer_desc.Width = 1024 * 1024 * 8;
      upload_buffer_desc.Height = 1;
      upload_buffer_desc.DepthOrArraySize = 1;
      upload_buffer_desc.MipLevels = 1;
      upload_buffer_desc.Format = DXGI_FORMAT_UNKNOWN;
      upload_buffer_desc.SampleDesc.Count = 1;
      upload_buffer_desc.SampleDesc.Quality = 0;
      upload_buffer_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
      upload_buffer_desc.Flags = D3D12_RESOURCE_FLAG_NONE;

      CD3DX12_HEAP_PROPERTIES heap_properties = CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_UPLOAD );
      m_context->m_device->CreateCommittedResource( &heap_properties, D3D12_HEAP_FLAG_NONE, &upload_buffer_desc,
        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS( &m_upload_buffer ) );
    }
    {
      uint8_t* pixels = nullptr;
      int32_t width = 0, height = 0;

      io.Fonts->GetTexDataAsRGBA32( &pixels, &width, &height );

      D3D12_RESOURCE_DESC desc;
      desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
      desc.Alignment = 0;
      desc.Width = width;
      desc.Height = height;
      desc.DepthOrArraySize = 1;
      desc.MipLevels = 1;
      desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
      desc.SampleDesc.Count = 1;
      desc.SampleDesc.Quality = 0;
      desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
      desc.Flags = D3D12_RESOURCE_FLAG_NONE;

      CD3DX12_HEAP_PROPERTIES heap_properties = CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_UPLOAD );
      m_context->m_device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_DEFAULT ), D3D12_HEAP_FLAG_NONE, &desc,
        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS( &m_font ) );

      uint32_t subres = 0;
      uint32_t num_rows = 0;
      uint64_t row_pitch = 0;
      uint64_t total_bytes = 0;
      D3D12_PLACED_SUBRESOURCE_FOOTPRINT layout = {};
      m_context->m_device->GetCopyableFootprints( &desc, subres, 1, 0, &layout, &num_rows, &row_pitch, &total_bytes );

      uint8_t* mapped_buffer = nullptr;

      m_upload_buffer->Map( 0, nullptr, ( void** ) &mapped_buffer );
      memcpy( mapped_buffer, pixels, ( size_t ) total_bytes );
      m_upload_buffer->Unmap( 0, nullptr );

      D3D12_BOX box = {};
      box.left = 0;
      box.top = 0;
      box.right = ( UINT ) desc.Width;
      box.bottom = ( UINT ) desc.Height;
      box.front = 0;
      box.back = 1;

      D3D12_TEXTURE_COPY_LOCATION dst = { m_font.Get(),   D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX,{ subres } };
      D3D12_TEXTURE_COPY_LOCATION src = { m_upload_buffer.Get(), D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT,  layout };

      m_ptr<ID3D12GraphicsCommandList> command_list = nullptr;

      result = m_context->m_device->CreateCommandList( 0, D3D12_COMMAND_LIST_TYPE_DIRECT,
        m_context->m_texture_command_allocator.Get(),
        nullptr, IID_PPV_ARGS( &command_list ) );
      assert( result == S_OK && "ERROR CREATING THE TEXTURE COMMNAND LIST" );

      command_list->CopyTextureRegion( &dst, 0, 0, 0, &src, &box );
      result = command_list->Close();
      assert( result == S_OK && "CLOSING THE COMMAND LIST FAILED" );

      ID3D12CommandList* ppCommandLists[] = { command_list.Get() };
      m_context->m_command_queue->ExecuteCommandLists( 1, ppCommandLists );

      //This just waits for the queue, even though it has a specific name. Probably should clear this.
      m_context->wait_for_setup_completion();

      D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc;
      srv_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
      srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
      srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
      srv_desc.Texture2D.MostDetailedMip = 0;
      srv_desc.Texture2D.MipLevels = 1;
      srv_desc.Texture2D.PlaneSlice = 0;
      srv_desc.Texture2D.ResourceMinLODClamp = 0.f;

      m_context->m_device->CreateShaderResourceView( m_font.Get(), &srv_desc,
        m_srv_heap->GetCPUDescriptorHandleForHeapStart() );
    }

  }
  
  void dxInterface::_render( ImDrawData* draw_data ) {

    dxContext* m_context = dynamic_cast< dxContext* >( k_engine->get_context() );
    dxInterface& renderer = *( dxInterface* ) ImGui::GetIO().UserData;
    HRESULT result;

    D3D12_RANGE readRange = {};
    readRange.End = 0;
    readRange.Begin = 1;

    int8_t* mapped_buffer = nullptr;
    result = renderer.m_upload_buffer->Map( 0, &readRange, ( void** ) &mapped_buffer );
    assert( result == S_OK && mapped_buffer != nullptr && "MAPPING THE BUFFER FAILED" );

    {
      const float L = 0.f;
      const float R = ImGui::GetIO().DisplaySize.x;
      const float B = ImGui::GetIO().DisplaySize.y;
      const float T = 0.f;
      const float mvp[4][4] =
      {
        { 2.0f / ( R - L ), 0.0f, 0.0f, 0.0f },
        { 0.0f, 2.0f / ( T - B ), 0.0f, 0.0f, },
        { 0.0f, 0.0f, 0.5f, 0.0f },
        { ( R + L ) / ( L - R ), ( T + B ) / ( B - T ), 0.5f, 1.0f },
      };

      memcpy( mapped_buffer, &mvp[0], sizeof( mvp ) );
      mapped_buffer += sizeof( mvp );
    }

    for( int n = 0; n < draw_data->CmdListsCount; n++ ) {
      const ImDrawList* cmd_list = draw_data->CmdLists[n];
      size_t v_count = cmd_list->VtxBuffer.size();
      size_t i_count = cmd_list->IdxBuffer.size();
      size_t v_size = v_count * sizeof( ImDrawVert );
      size_t i_size = i_count * sizeof( ImDrawIdx );

      memcpy( mapped_buffer, &cmd_list->VtxBuffer[0], v_size );
      mapped_buffer += v_size;

      memcpy( mapped_buffer, &cmd_list->IdxBuffer[0], i_size );
      mapped_buffer += i_size;
    }

    renderer.m_upload_buffer->Unmap( 0, nullptr );

    D3D12_VIEWPORT viewport = {};
    viewport.Width = ImGui::GetIO().DisplaySize.x;
    viewport.Height = ImGui::GetIO().DisplaySize.y;
    viewport.MinDepth = 0.f;
    viewport.MaxDepth = 1.f;
    viewport.TopLeftX = 0.f;
    viewport.TopLeftY = 0.f;

    m_context->m_render_command_list->SetPipelineState( renderer.m_pipeline_state.Get() );
    m_context->m_render_command_list->SetGraphicsRootSignature( renderer.m_root_signature.Get() );
    m_context->m_render_command_list->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

    m_context->m_render_command_list->RSSetViewports( 1, &viewport );

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(
      m_context->m_rtv_heap->GetCPUDescriptorHandleForHeapStart(),
      m_context->m_frame_index,
      m_context->m_rtv_descriptor_size );

    m_context->m_render_command_list->OMSetRenderTargets( 1, &rtvHandle, FALSE, nullptr );

    D3D12_GPU_VIRTUAL_ADDRESS bufferAddress = renderer.m_upload_buffer->GetGPUVirtualAddress();
    m_context->m_render_command_list->SetGraphicsRootConstantBufferView( 1, bufferAddress );


    ID3D12DescriptorHeap* ppHeaps[] = { renderer.m_srv_heap.Get() };
    m_context->m_render_command_list->SetDescriptorHeaps( _countof( ppHeaps ), ppHeaps );

    m_context->m_render_command_list->SetGraphicsRootDescriptorTable( 0,
      renderer.m_srv_heap->GetGPUDescriptorHandleForHeapStart() );

    uint64_t buffer_offset = 64;

    for( int n = 0; n < draw_data->CmdListsCount; n++ ) {

      int vtx_offset = 0;
      int idx_offset = 0;

      const ImDrawList* cmd_list = draw_data->CmdLists[n];
      size_t v_count = cmd_list->VtxBuffer.size();
      size_t i_count = cmd_list->IdxBuffer.size();
      size_t v_size = v_count * sizeof( ImDrawVert );
      size_t i_size = i_count * sizeof( ImDrawIdx );

      D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
      vertexBufferView.BufferLocation = bufferAddress + buffer_offset;
      vertexBufferView.StrideInBytes = sizeof( ImDrawVert );
      vertexBufferView.SizeInBytes = ( UINT ) v_size;
      buffer_offset += v_size;

      D3D12_INDEX_BUFFER_VIEW indexBufferView;
      indexBufferView.BufferLocation = bufferAddress + buffer_offset;
      indexBufferView.SizeInBytes = ( UINT ) i_size;
      indexBufferView.Format = DXGI_FORMAT_R16_UINT;
      buffer_offset += i_size;

      m_context->m_render_command_list->IASetVertexBuffers( 0, 1, &vertexBufferView );
      m_context->m_render_command_list->IASetIndexBuffer( &indexBufferView );

      for( int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.size(); cmd_i++ ) {
        const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
        if( pcmd->UserCallback ) {
          pcmd->UserCallback( cmd_list, pcmd );
        } else {
          const D3D12_RECT r = { ( LONG ) pcmd->ClipRect.x, ( LONG ) pcmd->ClipRect.y, ( LONG ) pcmd->ClipRect.z, ( LONG ) pcmd->ClipRect.w };
          m_context->m_render_command_list->RSSetScissorRects( 1, &r );
          m_context->m_render_command_list->DrawIndexedInstanced( pcmd->ElemCount, 1, idx_offset, vtx_offset, 0 );
        }
        idx_offset += pcmd->ElemCount;
      }
      vtx_offset += ( UINT ) v_count;
    }

  }

}