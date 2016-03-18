#include "core/dx/renderer.hh"
#include "core/dx/context.hh"
#include "core/engine.hh"
#include "core/engine_settings.hh"
#include "core/window.hh"
#include <cassert>

namespace kretash {

  /* This will create the instance buffers object in Vulkan and D3D12 */
  void dxRenderer::create_instance_buffer_objects( std::vector<Drawable*>* d ) {

    HRESULT result;
    dxContext* m_context = dynamic_cast< dxContext* >( k_engine->get_context() );

    CD3DX12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_UPLOAD );
    CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(
      sizeof( instance_buffer ) * k_engine->get_total_drawables() );

    result = m_context->m_device->CreateCommittedResource(
      &heapProperties,
      D3D12_HEAP_FLAG_NONE,
      &resourceDesc,
      D3D12_RESOURCE_STATE_GENERIC_READ,
      nullptr,
      IID_PPV_ARGS( &m_instance_buffer ) );
    assert( result == S_OK && "CREATING THE CONSTANT BUFFER FAILED" );
    m_instance_buffer->SetName( L"INSTANCE BUFFER" );

  }

  /* This will create the instance buffer object view in D3D12 and do nothing in Vulkan*/
  void dxRenderer::create_instance_buffer_object_view( Drawable* d, uint32_t offset ) {
    return;
    const UINT buffer_size = sizeof( instance_buffer ) + 255 & ~255;
    m_instance_buffer_desc = {};
    D3D12_GPU_VIRTUAL_ADDRESS addr = m_instance_buffer->GetGPUVirtualAddress();
    m_instance_buffer_desc.BufferLocation = addr + offset;
    m_instance_buffer_desc.SizeInBytes = buffer_size;

    CD3DX12_CPU_DESCRIPTOR_HANDLE cbvSrvHandle(
      m_cbv_heap->GetCPUDescriptorHandleForHeapStart(),
      0,
      m_cbv_srv_descriptor_size );

    dxContext* m_context = dynamic_cast< dxContext* >( k_engine->get_context() );
    m_context->m_device->CreateConstantBufferView( &m_instance_buffer_desc, cbvSrvHandle );

  }

  /* This will update the instance buffers object in Vulkan and D3D12 */
  void dxRenderer::update_instance_buffer_objects( std::vector<Drawable*>* d, std::vector<instance_buffer>* ib ) {

    if( ib->size() == 0 ) return;

    HRESULT result = m_instance_buffer->Map( 0, nullptr, reinterpret_cast< void** >( &m_instance_buffer_WO ) );
    assert( result == S_OK && "MAPPING THE CONSTANT BUFFER FALILED" );
    memcpy( m_instance_buffer_WO, &( ib->at( 0 ) ), sizeof( instance_buffer ) * k_engine->get_total_drawables() );
    m_instance_buffer->Unmap( 0, nullptr );

  }

  /* This will create the root signature in Vulkan and D3D12 */
  void dxRenderer::create_root_signature() {
    HRESULT result;

    CD3DX12_DESCRIPTOR_RANGE ranges[3];
    ranges[0].Init( D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0 );
    ranges[1].Init( D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2048, 0 );
    ranges[2].Init( D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 2, 0 );

    CD3DX12_ROOT_PARAMETER rootParameters[GRP_COUNT];
    rootParameters[GRP_INSTANCE_CBV].InitAsConstantBufferView( 0, 0, D3D12_SHADER_VISIBILITY_ALL );
    rootParameters[GRP_SRV].InitAsDescriptorTable( 1, &ranges[1], D3D12_SHADER_VISIBILITY_ALL );
    rootParameters[GRP_CONSTANT_CBV].InitAsConstantBufferView( 1, 0, D3D12_SHADER_VISIBILITY_ALL );
    rootParameters[GRP_SAMPLER].InitAsDescriptorTable( 1, &ranges[2], D3D12_SHADER_VISIBILITY_ALL );

    D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
      D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    D3D12_STATIC_SAMPLER_DESC sampler = {};
    sampler.Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
    sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
    sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
    sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
    sampler.MipLODBias = 0;
    sampler.MaxAnisotropy = 1;
    sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
    sampler.MinLOD = 0.0f;
    sampler.MaxLOD = 9999.0f;
    sampler.ShaderRegister = 0;
    sampler.RegisterSpace = 0;
    sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init( _countof( rootParameters ), rootParameters, 0, nullptr, rootSignatureFlags );

    m_ptr<ID3DBlob> signature;
    m_ptr<ID3DBlob> error;
    result = D3D12SerializeRootSignature( &rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error );
    assert( result == S_OK && "ERROR SERIALIZING THE ROOT SIGNATURE" );

    dxContext* m_context = dynamic_cast< dxContext* >( k_engine->get_context() );

    result = m_context->m_device->CreateRootSignature( 0, signature->GetBufferPointer(), signature->GetBufferSize(),
      IID_PPV_ARGS( &m_root_signature ) );
    assert( result == S_OK && "ERROR CREATING THE ROOT SIGNATURE" );

  }

  /* This will create the graphics pipeline in Vulkan and D3D12 */
  void dxRenderer::create_graphics_pipeline( render_type rt ) {

    HRESULT result;

    _load_and_compile_shaders( rt );

    const UINT input_element_desc_size = 5;
    D3D12_INPUT_ELEMENT_DESC inputElementsDesc[input_element_desc_size];

    dxContext* m_context = dynamic_cast< dxContext* >( k_engine->get_context() );

    m_context->m_pipeline_state_object_id_counter++;

    m_pipeline_state_id = m_context->m_pipeline_state_object_id_counter;

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
    psoDesc.pRootSignature = m_root_signature.Get();
    psoDesc.VS = { reinterpret_cast< UINT8* >( vertexShader->GetBufferPointer() ),
      vertexShader->GetBufferSize() };
    psoDesc.PS = { reinterpret_cast< UINT8* >( pixelShader->GetBufferPointer() ),
      pixelShader->GetBufferSize() };

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

    result = m_context->m_device->CreateGraphicsPipelineState( &psoDesc, IID_PPV_ARGS( &m_pipeline_state ) );
    assert( result == S_OK && "ERROR CREATING THE PIPELINE STATE" );
  }

  /*compiles shaders*/
  void dxRenderer::_load_and_compile_shaders( render_type s ) {
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
      filename = WSPATH"basic.hlsl";
    else if( s == rTEXTURE )
      filename = WSPATH"texture.hlsl";
    else if( s == rSKYDOME )
      filename = WSPATH"skydome.hlsl";
    else if( s == rPOST )
      filename = WSPATH"post.hlsl";

    result = D3DCompileFromFile( filename.c_str(), nullptr, nullptr, "VSMain", "vs_5_1", compileFlags, compileFlags,
      &vertexShader, &error_msg );
    if( error_msg )
      _show_shader_error_message( error_msg, filename );

    result = D3DCompileFromFile( filename.c_str(), nullptr, nullptr, "PSMain", "ps_5_1", compileFlags, compileFlags,
      &pixelShader, &error_msg );
    if( error_msg )
      _show_shader_error_message( error_msg, filename );
  }

  /*opens a window with an error message*/
  void dxRenderer::_show_shader_error_message( ID3D10Blob* errorMessage, std::wstring filename ) {
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

  dxRenderer::dxRenderer() {

    m_command_signature = nullptr;
    m_command_buffer = nullptr;
    m_command_buffer_upload = nullptr;
    m_cbv_heap = nullptr;
    m_srv_heap = nullptr;
    vertexShader = nullptr;
    pixelShader = nullptr;
    m_root_signature = nullptr;
    m_pipeline_state = nullptr;
    m_instance_buffer = nullptr;
    m_instance_buffer_WO = nullptr;
    m_constant_buffer = nullptr;
    m_constant_buffer_WO = nullptr;

    m_indirect_commands.clear();
    m_indirect_commands.shrink_to_fit();
    m_instance_buffer_desc = {};
    m_constant_buffer_desc = {};

    m_cbv_srv_descriptor_size = 0;
    m_pipeline_state_id = 0;

  }

  dxRenderer::~dxRenderer() {

    m_command_signature = nullptr;
    m_command_buffer = nullptr;
    m_command_buffer_upload = nullptr;
    m_cbv_heap = nullptr;
    m_srv_heap = nullptr;
    vertexShader = nullptr;
    pixelShader = nullptr;
    m_root_signature = nullptr;
    m_pipeline_state = nullptr;
    m_instance_buffer = nullptr;
    m_instance_buffer_WO = nullptr;
    m_constant_buffer = nullptr;
    m_constant_buffer_WO = nullptr;

    m_indirect_commands.clear();
    m_indirect_commands.shrink_to_fit();
    m_instance_buffer_desc = {};
    m_constant_buffer_desc = {};

  }

}