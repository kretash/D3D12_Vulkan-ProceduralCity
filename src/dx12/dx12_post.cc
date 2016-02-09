#include "core/k_directx12.hh"

#ifdef __DIRECTX12__

#include <cassert>
#include "core/engine.hh"
#include "core/window.hh"
#include "core/drawable.hh"
#include "core/engine_settings.hh"

namespace dx {

  void create_post_root_signature( renderer_data* r ) {
    HRESULT result;

    CD3DX12_DESCRIPTOR_RANGE ranges[3];
    //ranges[0].Init( D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0 );
    ranges[1].Init( D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0 );
    ranges[2].Init( D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0 );

    CD3DX12_ROOT_PARAMETER rootParameters[2];
    //rootParameters[0].InitAsConstantBufferView( 0, 0, D3D12_SHADER_VISIBILITY_ALL );
    rootParameters[0].InitAsDescriptorTable( 1, &ranges[1], D3D12_SHADER_VISIBILITY_ALL );
    rootParameters[1].InitAsDescriptorTable( 1, &ranges[2], D3D12_SHADER_VISIBILITY_ALL );

    D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
      D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    D3D12_STATIC_SAMPLER_DESC sampler = {};
    sampler.Filter = D3D12_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT;
    sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    sampler.MipLODBias = 0;
    sampler.MaxAnisotropy = 0;
    sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
    sampler.MinLOD = 0.0f;
    sampler.MaxLOD = 9999.0f;
    sampler.ShaderRegister = 0;
    sampler.RegisterSpace = 0;
    sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init( _countof( rootParameters ), rootParameters, 0, &sampler, rootSignatureFlags );

    m_ptr<ID3DBlob> signature;
    m_ptr<ID3DBlob> error;
    result = D3D12SerializeRootSignature( &rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error );
    assert( result == S_OK && "ERROR SERIALIZING THE ROOT SIGNATURE" );
    result = k_engine->get_engine_data()->m_device->CreateRootSignature( 0, signature->GetBufferPointer(), signature->GetBufferSize(),
      IID_PPV_ARGS( &r->m_root_signature ) );
    assert( result == S_OK && "ERROR CREATING THE ROOT SIGNATURE" );
  }

  void add_post_textures_to_srv( engine_data* e, renderer_data* r ) {

    D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
    srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srv_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
    srv_desc.Texture2D.MipLevels = 1;

    CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle( r->m_srv_heap->GetCPUDescriptorHandleForHeapStart() );
    e->m_device->CreateShaderResourceView( e->m_msaa_render_target.Get(), &srv_desc, srvHandle );

    srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvHandle.Offset( 1, r->m_cbv_srv_descriptor_size );
    e->m_device->CreateShaderResourceView( e->m_post_render_target.Get(), &srv_desc, srvHandle );
  }

  void create_post_pipeline_state_object( renderer_data* r ) {
    HRESULT result;
    D3D12_INPUT_ELEMENT_DESC inputElementsDesc[5];

    ++p_pipeline_state_object_id_counter;
    r->m_pipeline_state_id = p_pipeline_state_object_id_counter;

    inputElementsDesc[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
      D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
    inputElementsDesc[1] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12,
      D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
    inputElementsDesc[2] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24,
      D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
    inputElementsDesc[3] = { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32,
      D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
    inputElementsDesc[4] = { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 44,
      D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { inputElementsDesc, 5 };
    psoDesc.pRootSignature = r->m_root_signature.Get();
    psoDesc.VS = { reinterpret_cast< UINT8* >( r->vertexShader->GetBufferPointer() ),
      r->vertexShader->GetBufferSize() };
    psoDesc.PS = { reinterpret_cast< UINT8* >( r->pixelShader->GetBufferPointer() ),
      r->pixelShader->GetBufferSize() };
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC( D3D12_DEFAULT );
    psoDesc.BlendState = CD3DX12_BLEND_DESC( D3D12_DEFAULT );
    psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC( D3D12_DEFAULT );
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc.Count = 1;

    result = k_engine->get_engine_data()->m_device->CreateGraphicsPipelineState( &psoDesc, IID_PPV_ARGS( &r->m_pipeline_state ) );
    assert( result == S_OK && "ERROR CREATING THE PIPELINE STATE" );

  }

  void post_render( engine_data* e, renderer_data* r, Window* w ) {

    e->m_render_command_list->SetPipelineState( r->m_pipeline_state.Get() );
    e->m_render_command_list->SetGraphicsRootSignature( r->m_root_signature.Get() );
    e->m_render_command_list->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );

    ID3D12DescriptorHeap* ppHeaps[] = { r->m_srv_heap.Get() };
    e->m_render_command_list->SetDescriptorHeaps( _countof( ppHeaps ), ppHeaps );

    const float clear_color[] = { 0.529f, 0.8f, 0.98f, 1.0f };
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle( e->m_rtv_heap->GetCPUDescriptorHandleForHeapStart(),
      e->m_frame_index, e->m_rtv_descriptor_size );

    e->m_render_command_list->OMSetRenderTargets( 1, &rtvHandle, FALSE, nullptr );
    e->m_render_command_list->ClearRenderTargetView( rtvHandle, clear_color, 0, nullptr );

    D3D12_RESOURCE_BARRIER barriers[] = {
      CD3DX12_RESOURCE_BARRIER::Transition( e->m_render_targets[e->m_frame_index].Get(),
      D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET ),
      CD3DX12_RESOURCE_BARRIER::Transition( e->m_msaa_render_target.Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE )
    };
    e->m_render_command_list->ResourceBarrier( _countof( barriers ), barriers );

    int32_t srv_index = 0;
    if( e->m_msaa_enabled ) {
      e->m_render_command_list->ResolveSubresource( e->m_post_render_target.Get(), 0,
        e->m_msaa_render_target.Get(), 0, DXGI_FORMAT_R8G8B8A8_UNORM );

      srv_index = 1;
    }

    CD3DX12_GPU_DESCRIPTOR_HANDLE srvHandle( r->m_srv_heap->GetGPUDescriptorHandleForHeapStart(), srv_index,
      r->m_cbv_srv_descriptor_size );
    e->m_render_command_list->SetGraphicsRootDescriptorTable( 0, srvHandle );

    e->m_render_command_list->DrawInstanced( 4, 1, 0, 0 );

  }

}

#endif