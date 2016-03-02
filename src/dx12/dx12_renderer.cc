#include "core/k_directx12.hh"

#ifdef __DIRECTX12__

#include <cassert>
#include "core/engine.hh"
#include "core/window.hh"
#include "core/drawable.hh"
#include "core/engine_settings.hh"

namespace dx {
  void create_srv_view_heap( engine_data* e, renderer_data* r, int32_t size ) {
    HRESULT result = 0;

    D3D12_DESCRIPTOR_HEAP_DESC srv_heap_desc = {};
    srv_heap_desc.NumDescriptors = size;
    srv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

    result = k_engine->get_engine_data()->m_device->CreateDescriptorHeap(
      &srv_heap_desc, IID_PPV_ARGS( &r->m_srv_heap ) );

    assert( result == S_OK && "ERROR CREATING THE SRV HEAP" );

    r->m_cbv_srv_descriptor_size = k_engine->get_engine_data()->m_device->GetDescriptorHandleIncrementSize(
      D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV );
  }

  void create_root_signature( renderer_data* r ) {
    HRESULT result;

    CD3DX12_DESCRIPTOR_RANGE ranges[3];
    ranges[0].Init( D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0 );
    ranges[1].Init( D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2048, 0 );
    ranges[2].Init( D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0 );

    CD3DX12_ROOT_PARAMETER rootParameters[GRP_COUNT];
    rootParameters[GRP_INSTANCE_CBV].InitAsConstantBufferView( 0, 0, D3D12_SHADER_VISIBILITY_ALL );
    rootParameters[GRP_SRV].InitAsDescriptorTable( 1, &ranges[1], D3D12_SHADER_VISIBILITY_ALL );
    rootParameters[GRP_CONSTANT_CBV].InitAsConstantBufferView( 1, 0, D3D12_SHADER_VISIBILITY_ALL );
    rootParameters[GRP_SAMPLER].InitAsDescriptorTable( 1, &ranges[2], D3D12_SHADER_VISIBILITY_ALL );

    D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
      D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    D3D12_STATIC_SAMPLER_DESC sampler = {};
    sampler.Filter = D3D12_FILTER_ANISOTROPIC;
    sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
    sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
    sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
    sampler.MipLODBias = 0;
    sampler.MaxAnisotropy = 16;
    sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
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

  void load_and_compile_shaders( renderer_data* r, render_type s ) {
    HRESULT result;

    //D3DCOMPILE_ENABLE_UNBOUNDED_DESCRIPTOR_TABLES
#ifdef _DEBUG
    UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_PREFER_FLOW_CONTROL;
#else
    UINT compileFlags = 0;
#endif

    ID3DBlob* error_msg;
    std::wstring filename = L"UNKNOW RENDER TYPE";

    if( s == rBASIC )
      filename = SPATH"basic.hlsl";
    else if( s == rTEXTURE )
      filename = SPATH"texture.hlsl";
    else if( s == rSKYDOME )
      filename = SPATH"skydome.hlsl";
    else if( s == rPOST )
      filename = SPATH"post.hlsl";

    result = D3DCompileFromFile( filename.c_str(), nullptr, nullptr, "VSMain", "vs_5_1", compileFlags, compileFlags,
      &r->vertexShader, &error_msg );
    if( error_msg )
      show_shader_error_message( error_msg, filename );

    result = D3DCompileFromFile( filename.c_str(), nullptr, nullptr, "PSMain", "ps_5_1", compileFlags, compileFlags,
      &r->pixelShader, &error_msg );
    if( error_msg )
      show_shader_error_message( error_msg, filename );
  }

  void create_pipeline_state_object( renderer_data* r ) {
    HRESULT result;

    const UINT input_element_desc_size = 5;
    D3D12_INPUT_ELEMENT_DESC inputElementsDesc[input_element_desc_size];

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

    CD3DX12_DEPTH_STENCIL_DESC depthStencilDesc( D3D12_DEFAULT );
    depthStencilDesc.DepthEnable = true;
    depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    depthStencilDesc.StencilEnable = FALSE;

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { inputElementsDesc, input_element_desc_size };
    psoDesc.pRootSignature = r->m_root_signature.Get();
    psoDesc.VS = { reinterpret_cast< UINT8* >( r->vertexShader->GetBufferPointer() ),
      r->vertexShader->GetBufferSize() };
    psoDesc.PS = { reinterpret_cast< UINT8* >( r->pixelShader->GetBufferPointer() ),
      r->pixelShader->GetBufferSize() };

    CD3DX12_RASTERIZER_DESC raster_desc = CD3DX12_RASTERIZER_DESC( D3D12_DEFAULT );
    psoDesc.RasterizerState = raster_desc;

    psoDesc.BlendState = CD3DX12_BLEND_DESC( D3D12_DEFAULT );
    psoDesc.DepthStencilState = depthStencilDesc;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;

    bool msaa_enabled = k_engine_settings->get_settings().msaa_enabled;
    int32_t mssa_count = k_engine_settings->get_settings().msaa_count;

    psoDesc.SampleDesc.Count = mssa_count;
    psoDesc.SampleDesc.Quality = 0;
    if( msaa_enabled )
      psoDesc.SampleDesc.Quality = DXGI_STANDARD_MULTISAMPLE_QUALITY_PATTERN;

    result = k_engine->get_engine_data()->m_device->CreateGraphicsPipelineState( &psoDesc, IID_PPV_ARGS( &r->m_pipeline_state ) );
    assert( result == S_OK && "ERROR CREATING THE PIPELINE STATE" );

  }

  void show_shader_error_message( ID3D10Blob* errorMessage, std::wstring filename ) {
    char* compile_errors = ( char* ) ( errorMessage->GetBufferPointer() );

    size_t buffer_size = errorMessage->GetBufferSize();
    std::string error_message_s = "";

    for( size_t i = 0; i < buffer_size; i++ ) {
      error_message_s += compile_errors[i];
    }

    errorMessage->Release();
    errorMessage = 0;

    std::wstring error_message_LPCWSTR = std::wstring( error_message_s.begin(), error_message_s.end() );

    MessageBoxW( k_engine->get_window()->get_window_handle(), error_message_LPCWSTR.c_str(), filename.c_str(), MB_OK );
  }
}

#endif