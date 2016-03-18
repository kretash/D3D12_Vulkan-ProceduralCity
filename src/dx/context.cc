#include "core/dx/context.hh"
#include "core/dx/drawable.hh"
#include "core/dx/renderer.hh"
#include "core/dx/geometry.hh"
#include "core/engine.hh"
#include "core/engine_settings.hh"
#include "core/window.hh"
#include "core/drawable.hh"
#include "core/world.hh"
#include "core/GPU_pool.hh"
#include <cassert>

#define ENABLE_VALIDATION 1

namespace kretash {
  /* This will create a factory in D3D12 and do nothing in Vulkan */
  void dxContext::create_factory() {

#if ENABLE_VALIDATION
    if( SUCCEEDED( D3D12GetDebugInterface( IID_PPV_ARGS( &m_debug_controller ) ) ) ) {
      m_debug_controller->EnableDebugLayer();
    }
#endif

    HRESULT result;
    result = D3D12CreateDevice( nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS( &m_device ) );
    assert( result == S_OK && "CREATING THE DEVICE FAILED" );
  }

  /* This will create a device, used in both APIs */
  void dxContext::create_device() {
    HRESULT result;
    result = CreateDXGIFactory1( IID_PPV_ARGS( &factory ) );
    assert( result == S_OK && "CREATING THE FACTORY FAILED" );
  }

  /* This will create a swap chain and get the queue, used in both APIs */
  void dxContext::create_swap_chain( Window* w ) {

    HRESULT result;
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    result = m_device->CreateCommandQueue( &queueDesc, IID_PPV_ARGS( &m_command_queue ) );
    assert( result == S_OK && "ERROR CREATING THE COMMAND QUEUE" );

    result = m_device->CreateCommandQueue( &queueDesc, IID_PPV_ARGS( &m_texture_command_queue ) );
    assert( result == S_OK && "ERROR CREATING THE COMMAND QUEUE" );

    //Swap chain
    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    swapChainDesc.BufferCount = m_frame_count;
    swapChainDesc.BufferDesc.Width = w->get_width();
    swapChainDesc.BufferDesc.Height = w->get_height();;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.OutputWindow = w->get_window_handle();
    swapChainDesc.SampleDesc.Count = 1;

    swapChainDesc.Windowed = !k_engine_settings->get_settings().fullscreen;

    m_ptr<IDXGISwapChain> swapChain;
    result = factory->CreateSwapChain( m_command_queue.Get(), &swapChainDesc, &swapChain );
    assert( result == S_OK && "ERROR CREATING THE SWAP CHAIN" );

    swapChain.As( &m_swap_chain );
    m_frame_index = m_swap_chain->GetCurrentBackBufferIndex();

    swapChain->SetFullscreenState( k_engine_settings->get_settings().fullscreen, nullptr );

  }

  /* This will create a command pool in Vulkan or command Allocator in D3D12 */
  void dxContext::create_command_pool() {

    HRESULT result;

    result = m_device->CreateCommandAllocator( D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS( &m_texture_command_allocator ) );
    assert( result == S_OK && "ERROR CREATING THE TEXTURE COMMAND ALLOCATOR" );


    result = m_device->CreateCommandAllocator( D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS( &m_buffer_command_allocator ) );
    assert( result == S_OK && "ERROR CREATING THE BUFFER COMMAND ALLOCATOR" );


    result = m_device->CreateCommandAllocator( D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS( &m_render_command_allocator ) );
    assert( result == S_OK && "ERROR CREATING THE RENDER COMMAND ALLOCATOR" );

  }

  /* This will create the setup command buffer in Vulkan or command lists in D3D12*/
  void dxContext::create_buffer_command_buffer() {
    HRESULT result;

    result = m_device->CreateCommandList( 0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_buffer_command_allocator.Get(),
      nullptr, IID_PPV_ARGS( &m_buffer_command_list ) );
    assert( result == S_OK && "ERROR CREATING THE TEXTURE COMMNAND LIST" );

    result = m_buffer_command_list->Close();
    assert( result == S_OK && "ERROR CLOSING THE TEXTURE COMMAND LIST" );

  }

  /* This will create the texture command buffer in Vulkan or command lists in D3D12*/
  void dxContext::create_texture_command_buffer() {
    HRESULT result;

    result = m_device->CreateCommandList( 0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_texture_command_allocator.Get(),
      nullptr, IID_PPV_ARGS( &m_texture_command_list ) );
    assert( result == S_OK && "ERROR CREATING THE TEXTURE COMMNAND LIST" );

    result = m_texture_command_list->Close();
    assert( result == S_OK && "ERROR CLOSING THE TEXTURE COMMAND LIST" );
  }

  /* This will create the render command buffer in Vulkan or command lists in D3D12*/
  void dxContext::create_render_command_buffer() {
    HRESULT result;

    result = m_device->CreateCommandList( 0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_render_command_allocator.Get(),
      nullptr, IID_PPV_ARGS( &m_render_command_list ) );
    assert( result == S_OK && "ERROR CREATING THE COMMNAND LIST" );

    result = m_render_command_list->Close();
    assert( result == S_OK && "ERROR CLOSING THE COMMAND LIST" );

  }

  /* This will create a depth stencil texture and a view in both APIs */
  void dxContext::create_depth_stencil( Window* w ) {

    HRESULT result;
    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    result = m_device->CreateDescriptorHeap( &dsvHeapDesc, IID_PPV_ARGS( &m_dsv_heap ) );
    assert( result == S_OK && "ERROR CREATING THE DSV HEAP" );


    m_msaa_enabled = k_engine_settings->get_settings().msaa_enabled;
    m_mssa_count = k_engine_settings->get_settings().msaa_count;
    float u_r = k_engine_settings->get_settings().upscale_render;

    CD3DX12_RESOURCE_DESC depth_texture(
      D3D12_RESOURCE_DIMENSION_TEXTURE2D,
      0,
      static_cast< UINT >( w->get_viewport().Width * u_r ),
      static_cast< UINT >( w->get_viewport().Height * u_r ),
      1,
      1,
      DXGI_FORMAT_D32_FLOAT,
      1,
      0,
      D3D12_TEXTURE_LAYOUT_UNKNOWN,
      D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL | D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE );

    //RENDER TO TEXTURE
    if( m_msaa_enabled ) {
      depth_texture.SampleDesc.Count = m_mssa_count;
      depth_texture.SampleDesc.Quality = DXGI_STANDARD_MULTISAMPLE_QUALITY_PATTERN;
    }

    D3D12_CLEAR_VALUE clear_value;
    clear_value.Format = DXGI_FORMAT_D32_FLOAT;
    clear_value.DepthStencil.Depth = 1.0f;
    clear_value.DepthStencil.Stencil = 0;

    D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
    depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
    //RENDER TO TEXTURE
    //depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
    depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

    result = m_device->CreateCommittedResource(
      &CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_DEFAULT ),
      D3D12_HEAP_FLAG_NONE, &depth_texture, D3D12_RESOURCE_STATE_DEPTH_WRITE, &clear_value,
      IID_PPV_ARGS( &m_depth_stencil ) );
    assert( result == S_OK && "CREATING THE DEPTH STENCIL FAILED" );

    m_device->CreateDepthStencilView( m_depth_stencil.Get(), &depthStencilDesc,
      m_dsv_heap->GetCPUDescriptorHandleForHeapStart() );

  }

  /* This will create a framebuffer/render target and a view in both APIs */
  void dxContext::create_framebuffer( Window* w ) {

    HRESULT result;
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    //RENDER TO TEXTURE
    rtvHeapDesc.NumDescriptors = m_frame_count + 0;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    result = m_device->CreateDescriptorHeap( &rtvHeapDesc, IID_PPV_ARGS( &m_rtv_heap ) );
    assert( result == S_OK && "ERROR CREATING THE DESCRIPTOR HEAP" );
    m_rtv_descriptor_size = m_device->GetDescriptorHandleIncrementSize( D3D12_DESCRIPTOR_HEAP_TYPE_RTV );

    m_msaa_enabled = k_engine_settings->get_settings().msaa_enabled;
    m_mssa_count = k_engine_settings->get_settings().msaa_count;
    float u_r = k_engine_settings->get_settings().upscale_render;

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle( m_rtv_heap->GetCPUDescriptorHandleForHeapStart() );

    m_render_targets.resize( m_frame_count );

    for( UINT n = 0; n < m_frame_count; ++n ) {
      result = m_swap_chain->GetBuffer( n, IID_PPV_ARGS( &m_render_targets[n] ) );
      assert( result == S_OK && "CREATING RTV FAILED" );

      D3D12_RENDER_TARGET_VIEW_DESC render_target_desc = {};
      render_target_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
      render_target_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

      m_device->CreateRenderTargetView( m_render_targets[n].Get(), &render_target_desc,
        rtvHandle );
      rtvHandle.Offset( 1, m_rtv_descriptor_size );
    }
    //RENDER TO TEXTURE
    if( false ) {
      CD3DX12_RESOURCE_DESC color_texture(
        D3D12_RESOURCE_DIMENSION_TEXTURE2D,
        0,
        static_cast< UINT >( w->get_viewport().Width ),
        static_cast< UINT >( w->get_viewport().Height ),
        1,
        1,
        DXGI_FORMAT_R8G8B8A8_UNORM,
        1,
        0,
        D3D12_TEXTURE_LAYOUT_UNKNOWN,
        D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET );

      if( m_msaa_enabled ) {
        color_texture.SampleDesc.Count = m_mssa_count;
        color_texture.SampleDesc.Quality = DXGI_STANDARD_MULTISAMPLE_QUALITY_PATTERN;
      }

      D3D12_CLEAR_VALUE clear_value;
      clear_value.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

      D3D12_RENDER_TARGET_VIEW_DESC render_target_desc = {};
      render_target_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
      render_target_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;

      result = m_device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_DEFAULT ),
        D3D12_HEAP_FLAG_NONE,
        &color_texture,
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        &clear_value,
        IID_PPV_ARGS( &m_msaa_render_target ) );

      assert( result == S_OK && "CREATING THE RTV FAILED" );

      m_device->CreateRenderTargetView(
        m_msaa_render_target.Get(),
        &render_target_desc,
        rtvHandle );
      rtvHandle.Offset( 1, m_rtv_descriptor_size );
    }
    //RENDER TO TEXTURE
    if( false ) {
      CD3DX12_RESOURCE_DESC color_texture(
        D3D12_RESOURCE_DIMENSION_TEXTURE2D,
        0,
        static_cast< UINT >( w->get_viewport().Width * u_r ),
        static_cast< UINT >( w->get_viewport().Height * u_r ),
        1,
        1,
        DXGI_FORMAT_R8G8B8A8_UNORM,
        1,
        0,
        D3D12_TEXTURE_LAYOUT_UNKNOWN,
        D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET );

      D3D12_CLEAR_VALUE clear_value;
      clear_value.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

      D3D12_RENDER_TARGET_VIEW_DESC render_target_desc = {};
      render_target_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
      render_target_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

      result = m_device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_DEFAULT ),
        D3D12_HEAP_FLAG_NONE,
        &color_texture,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        &clear_value,
        IID_PPV_ARGS( &m_post_render_target ) );

      assert( result == S_OK && "CREATING THE RTV FAILED" );

      m_device->CreateRenderTargetView(
        m_post_render_target.Get(),
        &render_target_desc,
        rtvHandle );
      rtvHandle.Offset( 1, m_rtv_descriptor_size );
    }
  }

  /* This will create fences in D3D12 and do nothing in Vulkan */
  void dxContext::create_fences() {

    HRESULT result;
    result = m_device->CreateFence( 0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS( &m_fence ) );
    assert( result == S_OK && "ERROR CREATING THE FENCE" );
    m_fence_value = 1;

    m_fence_event = CreateEventEx( nullptr, FALSE, FALSE, EVENT_ALL_ACCESS );
    assert( m_fence_event != nullptr && "ERROR CREATING THE FENCE EVENT" );

  }

  /* This will wait for all setup actions to be completed */
  void dxContext::wait_for_setup_completion() {

    const UINT64 fence = m_fence_value;
    HRESULT result = m_command_queue->Signal( m_fence.Get(), fence );
    assert( result == S_OK && "ERROR SIGNALING THE FENCE" );
    m_fence_value++;

    if( m_fence->GetCompletedValue() < fence ) {
      result = m_fence->SetEventOnCompletion( fence, m_fence_event );
      assert( result == S_OK && "SET EVENT ON COMPLETITION FAILED" );
      WaitForSingleObject( m_fence_event, INFINITE );
    }

  }

  /* This will create the constant buffer object in Vulkan and D3D12 */
  void dxContext::create_constant_buffer_object( xxDescriptorBuffer* db, constant_buffer* cb ) {
    HRESULT result;

    dxDescriptorBuffer* m_buffer = dynamic_cast< dxDescriptorBuffer* >( db );

    //Constant buffers // Uniforms
    const UINT buffer_size = sizeof( constant_buffer ) + 255 & ~255;  //has to be a multiple of 256bytes
    CD3DX12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_UPLOAD );
    CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer( sizeof( constant_buffer ) );
    result = m_device->CreateCommittedResource(
      &heapProperties,
      D3D12_HEAP_FLAG_NONE,
      &resourceDesc,
      D3D12_RESOURCE_STATE_GENERIC_READ,
      nullptr,
      IID_PPV_ARGS( &m_buffer->m_constant_buffer ) );
    assert( result == S_OK && "CREATING THE CONSTANT BUFFER FAILED" );

    m_buffer->m_constant_buffer->SetName( L"ConstantBuffer" );

  }

  /* This will update the constant buffer object in Vulkan and D3D12 */
  void dxContext::update_constant_buffer_object( xxDescriptorBuffer* db, constant_buffer* cb ) {

    HRESULT result;
    dxDescriptorBuffer* m_buffer = dynamic_cast< dxDescriptorBuffer* >( db );

    result = m_buffer->m_constant_buffer->Map( 0, nullptr, reinterpret_cast< void** >( &m_buffer->m_constant_buffer_WO ) );
    assert( result == S_OK && "MAPPING THE CONSTANT BUFFER FALILED" );
    memcpy( m_buffer->m_constant_buffer_WO, cb, sizeof( constant_buffer ) );
    m_buffer->m_constant_buffer->Unmap( 0, nullptr );

  }

  /* This will create shader resource view in D3D12 and do nothing in Vulkan */
  void dxContext::create_srv_view_heap( xxRenderer* r ) {

    HRESULT result;
    dxRenderer* m_renderer = dynamic_cast< dxRenderer* >( r );

    D3D12_DESCRIPTOR_HEAP_DESC srv_heap_desc = {};
    srv_heap_desc.NumDescriptors = m_max_textures;
    srv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

    result = m_device->CreateDescriptorHeap(
      &srv_heap_desc, IID_PPV_ARGS( &m_renderer->m_srv_heap ) );

    assert( result == S_OK && "ERROR CREATING THE SRV HEAP" );

    m_renderer->m_cbv_srv_descriptor_size = m_device->GetDescriptorHandleIncrementSize(
      D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV );

  }

  void dxContext::create_sampler_view_heap(){

    HRESULT result;

    D3D12_DESCRIPTOR_HEAP_DESC sampler_heap_desc = {};
    sampler_heap_desc.NumDescriptors = 2;
    sampler_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
    sampler_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    result = m_device->CreateDescriptorHeap( &sampler_heap_desc, IID_PPV_ARGS( &m_sampler_heap ) );

    m_sampler_descriptor_size = m_device->GetDescriptorHandleIncrementSize( D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER );
    CD3DX12_CPU_DESCRIPTOR_HANDLE sampler_handle( m_sampler_heap->GetCPUDescriptorHandleForHeapStart() );

    D3D12_SAMPLER_DESC linear_sampler_desc = {};
    linear_sampler_desc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    linear_sampler_desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    linear_sampler_desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    linear_sampler_desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    linear_sampler_desc.MinLOD = 0;
    linear_sampler_desc.MaxLOD = D3D12_FLOAT32_MAX;
    linear_sampler_desc.MipLODBias = 0.0f;
    linear_sampler_desc.MaxAnisotropy = 1;
    linear_sampler_desc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    m_device->CreateSampler( &linear_sampler_desc, sampler_handle );


    sampler_handle.Offset( m_sampler_descriptor_size );
    

    D3D12_SAMPLER_DESC point_sampler_desc = {};
    point_sampler_desc.Filter = D3D12_FILTER_ANISOTROPIC;
    point_sampler_desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
    point_sampler_desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
    point_sampler_desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
    point_sampler_desc.MipLODBias = 0.0f;
    point_sampler_desc.MaxAnisotropy = 16;
    point_sampler_desc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    point_sampler_desc.MinLOD = 0;
    point_sampler_desc.MaxLOD = D3D12_FLOAT32_MAX;
    m_device->CreateSampler( &point_sampler_desc, sampler_handle );


  }

  /* This will reset the texture command list in Vulkan and D3D12 */
  void dxContext::reset_texture_command_list() {
    HRESULT result;

    result = m_texture_command_allocator->Reset();
    assert( result == S_OK && "COMMAND ALLOCATOR RESET FAILED" );

    result = m_texture_command_list->Reset( m_texture_command_allocator.Get(), nullptr );
    assert( result == S_OK && "COMMAND LIST RESET FAILED" );
  }

  /* This will execute the texture command list in Vulkan and D3D12 */
  void dxContext::compute_texture_upload() {
    HRESULT result;

    result = m_texture_command_list->Close();
    assert( result == S_OK && "ERROR CLOSING THE COMMAND LIST" );
    ID3D12CommandList* ppCommandLists[] = { m_texture_command_list.Get() };
    m_texture_command_queue->ExecuteCommandLists( _countof( ppCommandLists ), ppCommandLists );

    result = m_device->CreateFence( m_fence_value, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS( &m_fence ) );
    if( result == 0x887a0005 ){
      result = m_device->GetDeviceRemovedReason();
      assert( false && "DEVICE LOST" );
    }
    assert( result == S_OK && "ERROR CREATING FENCE" );
    m_fence_value++;

    // Create an event handle to use for frame synchronization.
    m_fence_event = CreateEvent( nullptr, FALSE, FALSE, nullptr );
    if( m_fence_event == nullptr ) {
      HRESULT_FROM_WIN32( GetLastError() );
    }

    m_fence_texture_upload_pending = true;
    m_fence_texture_upload = m_fence_value;
    m_fence_value++;

    result = m_texture_command_queue->Signal( m_fence.Get(), m_fence_texture_upload );
    assert( result == S_OK && "ERROR SIGNALING FENCE" );
  }

  /* This will wait for texture command list in Vulkan and D3D12 */
  void dxContext::wait_for_texture_upload() {
    HRESULT result;

    if( m_fence_texture_upload_pending ) {
      // Wait until the fence is completed.
      m_fence_texture_upload_pending = false;

      if( m_fence->GetCompletedValue() < m_fence_texture_upload ) {
        result = m_fence->SetEventOnCompletion( m_fence_texture_upload, m_fence_event );
        assert( result == S_OK && "SET EVENT ON COMPLETITION FAILED" );
        WaitForSingleObject( m_fence_event, INFINITE );
      }

    }
  }

  /* This will create the command signature in D3D12 and do nothing in D3D12*/
  void dxContext::create_indirect_command_signature( xxRenderer* r ) {

    dxRenderer* m_renderer = dynamic_cast< dxRenderer* >( r );

    D3D12_INDIRECT_ARGUMENT_DESC argument_desc[2];
    argument_desc[0].Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT_BUFFER_VIEW;
    argument_desc[0].ConstantBufferView.RootParameterIndex = GRP_INSTANCE_CBV;
    argument_desc[1].Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;

    D3D12_COMMAND_SIGNATURE_DESC command_signature_desc = {};
    command_signature_desc.pArgumentDescs = argument_desc;
    command_signature_desc.NumArgumentDescs = _countof( argument_desc );
    command_signature_desc.ByteStride = sizeof( indirect_command );
    command_signature_desc.NodeMask = 1;

    HRESULT result = m_device->CreateCommandSignature( &command_signature_desc,
      m_renderer->m_root_signature.Get(), IID_PPV_ARGS( &m_renderer->m_command_signature ) );
    assert( result == S_OK && "CREATING THE COMMAND SIGNATURE FAILED" );

  }

  /* This will create the indirect command buffer in D3D12 and do nothing in D3D12*/
  void dxContext::create_indirect_command_buffer( xxRenderer* r, Drawable** draw, uint32_t d_count ) {

    HRESULT result;
    dxRenderer* m_renderer = dynamic_cast< dxRenderer* >( r );

    result = m_buffer_command_allocator->Reset();
    assert( result == S_OK && "COMMAND ALLOCATOR RESET FAILED" );

    result = m_buffer_command_list->Reset( m_buffer_command_allocator.Get(), nullptr );
    assert( result == S_OK && "COMMAND LIST RESET FAILED" );

    UINT command_buffer_size = d_count * sizeof( indirect_command );

    D3D12_RESOURCE_DESC command_buffer_desc = CD3DX12_RESOURCE_DESC::Buffer( command_buffer_size );

    result = m_device->CreateCommittedResource(
      &CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_DEFAULT ),
      D3D12_HEAP_FLAG_NONE,
      &command_buffer_desc,
      D3D12_RESOURCE_STATE_COPY_DEST,
      nullptr,
      IID_PPV_ARGS( &m_renderer->m_command_buffer ) );
    assert( result == S_OK && "CREATING THE COMMAND BUFFER FAILED" );
    m_renderer->m_command_buffer->SetName( L"CommandBuffer" );

    result = m_device->CreateCommittedResource(
      &CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_UPLOAD ),
      D3D12_HEAP_FLAG_NONE,
      &CD3DX12_RESOURCE_DESC::Buffer( command_buffer_size ),
      D3D12_RESOURCE_STATE_GENERIC_READ,
      nullptr,
      IID_PPV_ARGS( &m_renderer->m_command_buffer_upload ) );
    assert( result == S_OK && "CREATING THE COMMAND BUFFER UPLOAD FAILED" );
    m_renderer->m_command_buffer_upload->SetName( L"CommandBufferUpload" );

    D3D12_GPU_VIRTUAL_ADDRESS addr = m_renderer->m_instance_buffer->GetGPUVirtualAddress();

    UINT index = 0;
    m_renderer->m_indirect_commands.resize( d_count );

    for( index = 0; index < d_count; ++index ) {

      int id = draw[index]->get_drawable_id();

      indirect_command tmp = {};

      tmp.cbv = addr + ( id * sizeof( instance_buffer ) );

      tmp.draw_arguments.BaseVertexLocation =
        draw[index]->get_geometry()->get_vertex_offset() / m_stride;

      tmp.draw_arguments.IndexCountPerInstance =
        draw[index]->get_geometry()->get_indicies_count();

      tmp.draw_arguments.StartIndexLocation =
        draw[index]->get_geometry()->get_indicies_offset();

      tmp.draw_arguments.InstanceCount = 1;
      tmp.draw_arguments.StartInstanceLocation = 0;

      m_renderer->m_indirect_commands[index] = tmp;

    }

    D3D12_SUBRESOURCE_DATA command_data = {};
    command_data.pData = reinterpret_cast< UINT8* >( &m_renderer->m_indirect_commands[0] );
    command_data.RowPitch = command_buffer_size;
    command_data.SlicePitch = command_buffer_size;

    UpdateSubresources<1>( m_buffer_command_list.Get(), m_renderer->m_command_buffer.Get(),
      m_renderer->m_command_buffer_upload.Get(), 0, 0, 1, &command_data );

    m_buffer_command_list->ResourceBarrier( 1, &CD3DX12_RESOURCE_BARRIER::Transition(
      m_renderer->m_command_buffer.Get(),
      D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE ) );

    result = m_buffer_command_list->Close();
    assert( result == S_OK && "ERROR CLOSING THE COMMAND LIST" );
    ID3D12CommandList* ppCommandLists[] = { m_buffer_command_list.Get() };
    m_command_queue->ExecuteCommandLists( _countof( ppCommandLists ), ppCommandLists );

    //Wait for upload
    result = m_device->CreateFence( m_fence_value, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS( &m_fence ) );
    assert( result == S_OK && "ERROR CREATING FENCE" );
    m_fence_value++;

    // Create an event handle to use for frame synchronization.
    m_fence_event = CreateEvent( nullptr, FALSE, FALSE, nullptr );
    if( m_fence_event == nullptr ) {
      HRESULT_FROM_WIN32( GetLastError() );
    }

    const UINT64 fenceToWaitFor = m_fence_value;
    result = m_command_queue->Signal( m_fence.Get(), fenceToWaitFor );
    assert( result == S_OK && "ERROR SIGNALING FENCE" );
    m_fence_value++;

    // Wait until the fence is completed.
    result = m_fence->SetEventOnCompletion( fenceToWaitFor, m_fence_event );
    assert( result == S_OK && "ERROR SETTING COMPLETITION EVENT" );
    WaitForSingleObject( m_fence_event, INFINITE );

  }

  /* This will update the indirect command buffer in D3D12 and do nothing in D3D12*/
  void dxContext::update_indirect_command_buffer( xxRenderer* r, Drawable** draw, uint32_t d_count ) {

    uint32_t index = 0;
    dxRenderer* m_renderer = dynamic_cast< dxRenderer* >( r );

    D3D12_GPU_VIRTUAL_ADDRESS addr = m_renderer->m_instance_buffer->GetGPUVirtualAddress();
    uint32_t command_buffer_size = d_count * sizeof( indirect_command );

    HRESULT result = E_FAIL;

    result = m_buffer_command_allocator->Reset();
    assert( result == S_OK && "COMMAND ALLOCATOR RESET FAILED" );

    result = m_buffer_command_list->Reset( m_buffer_command_allocator.Get(), nullptr );
    assert( result == S_OK && "COMMAND LIST RESET FAILED" );

    for( index = 0; index < d_count; ++index ) {

      int id = draw[index]->get_drawable_id();

      indirect_command tmp = {};

      tmp.cbv = addr + ( id * sizeof( instance_buffer ) );

      tmp.draw_arguments.BaseVertexLocation =
        draw[index]->get_geometry()->get_vertex_offset() / m_stride;

      tmp.draw_arguments.IndexCountPerInstance =
        draw[index]->get_geometry()->get_indicies_count();

      tmp.draw_arguments.StartIndexLocation =
        draw[index]->get_geometry()->get_indicies_offset();

      tmp.draw_arguments.InstanceCount = 1;
      tmp.draw_arguments.StartInstanceLocation = 0;

      m_renderer->m_indirect_commands[index] = tmp;

    }

    D3D12_SUBRESOURCE_DATA command_data = {};
    command_data.pData = reinterpret_cast< UINT8* >( &m_renderer->m_indirect_commands.at( 0 ) );
    command_data.RowPitch = command_buffer_size;
    command_data.SlicePitch = command_data.RowPitch;

    UpdateSubresources<1>( m_buffer_command_list.Get(), m_renderer->m_command_buffer.Get(),
      m_renderer->m_command_buffer_upload.Get(), 0, 0, 1, &command_data );
    m_buffer_command_list->ResourceBarrier( 1, &CD3DX12_RESOURCE_BARRIER::Transition( m_renderer->m_command_buffer.Get(),
      D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE ) );

    result = m_buffer_command_list->Close();
    assert( result == S_OK && "ERROR CLOSING THE COMMAND LIST" );
    ID3D12CommandList* ppCommandLists[] = { m_buffer_command_list.Get() };
    m_command_queue->ExecuteCommandLists( _countof( ppCommandLists ), ppCommandLists );

    //Wait for upload
    result = m_device->CreateFence( m_fence_value, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS( &m_fence ) );
    assert( result == S_OK && "ERROR CREATING FENCE" );
    m_fence_value++;

    // Create an event handle to use for frame synchronization.
    m_fence_event = CreateEvent( nullptr, FALSE, FALSE, nullptr );
    if( m_fence_event == nullptr ) {
      HRESULT_FROM_WIN32( GetLastError() );
    }

    const UINT64 fenceToWaitFor = m_fence_value;
    result = m_command_queue->Signal( m_fence.Get(), fenceToWaitFor );
    assert( result == S_OK && "ERROR SIGNALING FENCE" );
    m_fence_value++;

    // Wait until the fence is completed.
    result = m_fence->SetEventOnCompletion( fenceToWaitFor, m_fence_event );
    assert( result == S_OK && "ERROR SETTING COMPLETITION EVENT" );
    WaitForSingleObject( m_fence_event, INFINITE );

  }

  /* This will reset the render command list in Vulkan and D3D12 */
  void dxContext::reset_render_command_list( Window* w ) {
    HRESULT result;

    result = m_render_command_allocator->Reset();
    assert( result == S_OK && "COMMAND ALLOCATOR RESET FAILED" );

    result = m_render_command_list->Reset( m_render_command_allocator.Get(), nullptr );
    assert( result == S_OK && "COMMAND LIST RESET FAILED" );

    m_render_command_list->RSSetScissorRects( 1, &w->get_scissor() );
    m_render_command_list->RSSetViewports( 1, &w->get_viewport() );
  }

  /* This will clear the color buffer in Vulkan and D3D12 */
  void dxContext::clear_color() {

    std::vector<D3D12_RESOURCE_BARRIER> barriers;
    barriers.push_back(
      CD3DX12_RESOURCE_BARRIER::Transition( m_render_targets[m_frame_index].Get(),
        D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET ) );
    m_render_command_list->ResourceBarrier( ( UINT ) barriers.size(), barriers.data() );

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle( m_rtv_heap->GetCPUDescriptorHandleForHeapStart(),
      m_frame_index, m_rtv_descriptor_size );
    CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle( m_dsv_heap->GetCPUDescriptorHandleForHeapStart() );

    //RENDER TO TEXTURE
    m_render_command_list->OMSetRenderTargets( 1, &rtvHandle, FALSE, &dsvHandle );

    const float clear_color[] = { 0.529f, 0.8f, 0.98f, 1.0f };
    m_render_command_list->ClearRenderTargetView( rtvHandle, clear_color, 0, nullptr );

  }

  /* This will clear the depth in Vulkan and D3D12 */
  void dxContext::clear_depth() {

    m_render_command_list->ClearDepthStencilView( m_dsv_heap->GetCPUDescriptorHandleForHeapStart(),
      D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr );

  }

  void dxContext::record_commands( xxRenderer* r, Window* w, Drawable** draw, uint32_t d_count ) {

    dxRenderer* m_renderer = dynamic_cast< dxRenderer* >( r );

    uint32_t count = 0;
    m_render_command_list->SetPipelineState( m_renderer->m_pipeline_state.Get() );
    m_render_command_list->SetGraphicsRootSignature( m_renderer->m_root_signature.Get() );
    m_render_command_list->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

    if( m_renderer->m_srv_heap != nullptr ) {

      ID3D12DescriptorHeap* ppHeaps[] = { m_renderer->m_srv_heap.Get() };
      m_render_command_list->SetDescriptorHeaps( _countof( ppHeaps ), ppHeaps );

      CD3DX12_GPU_DESCRIPTOR_HANDLE srvHandle( m_renderer->m_srv_heap->GetGPUDescriptorHandleForHeapStart(), 0,
        m_renderer->m_cbv_srv_descriptor_size );

      m_render_command_list->SetGraphicsRootDescriptorTable( GRP_SRV, srvHandle );
    }

    dxDescriptorBuffer* m_buffer = dynamic_cast< dxDescriptorBuffer* >( k_engine->get_world()->get_buffer() );

    m_render_command_list->SetGraphicsRootConstantBufferView(
      GRP_CONSTANT_CBV, m_buffer->m_constant_buffer->GetGPUVirtualAddress() );

    dxGeometry* m_geometry = dynamic_cast< dxGeometry* >( k_engine->get_GPU_pool()->get_xx_geometry() );
    m_render_command_list->IASetVertexBuffers( 0, 1, &m_geometry->m_vertexBufferView );
    m_render_command_list->IASetIndexBuffer( &m_geometry->m_indexBufferView );

    for( count; count < d_count; ++count ) {

      D3D12_GPU_VIRTUAL_ADDRESS addr = m_renderer->m_instance_buffer->GetGPUVirtualAddress();
      int id = draw[count]->get_drawable_id();

      m_render_command_list->SetGraphicsRootConstantBufferView(
        GRP_INSTANCE_CBV, addr + ( id * sizeof( instance_buffer ) ) );

      m_render_command_list->DrawIndexedInstanced( draw[count]->get_geometry()->get_indicies_count(), 1,
        draw[count]->get_geometry()->get_indicies_offset(),
        draw[count]->get_geometry()->get_vertex_offset() / m_stride, 0 );
    }
  }

  /* This will record commands list in Vulkan and D3D12 */
  void dxContext::record_indirect_commands( xxRenderer* r, Window* w, Drawable** draw, uint32_t d_count ) {

    dxRenderer* m_renderer = dynamic_cast< dxRenderer* >( r );
    dxDescriptorBuffer* m_buffer = dynamic_cast< dxDescriptorBuffer* >( k_engine->get_world()->get_buffer() );
    dxGeometry* m_geometry = dynamic_cast< dxGeometry* >( k_engine->get_GPU_pool()->get_xx_geometry() );

    uint32_t count = 0;
    m_render_command_list->SetPipelineState( m_renderer->m_pipeline_state.Get() );
    m_render_command_list->SetGraphicsRootSignature( m_renderer->m_root_signature.Get() );
    m_render_command_list->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

    m_render_command_list->IASetVertexBuffers( 0, 1, &m_geometry->m_vertexBufferView );
    m_render_command_list->IASetIndexBuffer( &m_geometry->m_indexBufferView );

    m_render_command_list->SetGraphicsRootConstantBufferView(
      GRP_INSTANCE_CBV, m_renderer->m_instance_buffer->GetGPUVirtualAddress() );

    m_render_command_list->SetGraphicsRootConstantBufferView(
      GRP_CONSTANT_CBV, m_buffer->m_constant_buffer->GetGPUVirtualAddress() );

    ID3D12DescriptorHeap* ppHeaps[] = { m_renderer->m_srv_heap.Get(), m_sampler_heap.Get() };
    m_render_command_list->SetDescriptorHeaps( _countof( ppHeaps ), ppHeaps );

    CD3DX12_GPU_DESCRIPTOR_HANDLE srv_handle( 
      m_renderer->m_srv_heap->GetGPUDescriptorHandleForHeapStart(), 
      0,
      m_renderer->m_cbv_srv_descriptor_size );
    m_render_command_list->SetGraphicsRootDescriptorTable( GRP_SRV, srv_handle );


    CD3DX12_GPU_DESCRIPTOR_HANDLE sampler_handle( 
      m_sampler_heap->GetGPUDescriptorHandleForHeapStart(),
      0,
      m_sampler_descriptor_size );
    m_render_command_list->SetGraphicsRootDescriptorTable( GRP_SAMPLER, sampler_handle );

    //Draw all
    m_render_command_list->ExecuteIndirect(
      m_renderer->m_command_signature.Get(),
      d_count,
      m_renderer->m_command_buffer.Get(),
      0,
      nullptr,
      0 );
  }

  /* This will execute the command list in Vulkan and D3D12 */
  void dxContext::execute_render_command_list() {

    std::vector<D3D12_RESOURCE_BARRIER> barriers;
    //RENDER TO TEXTURE
    if( false ) {
      barriers.push_back(
        CD3DX12_RESOURCE_BARRIER::Transition( m_render_targets[m_frame_index].Get(),
          D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT ) );

      barriers.push_back(
        CD3DX12_RESOURCE_BARRIER::Transition( m_msaa_render_target.Get(),
          D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET ) );

    } else {
      barriers.push_back(
        CD3DX12_RESOURCE_BARRIER::Transition( m_render_targets[m_frame_index].Get(),
          D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT ) );
    }

    m_render_command_list->ResourceBarrier( ( UINT ) barriers.size(), barriers.data() );

    HRESULT result = m_render_command_list->Close();
    assert( result == S_OK && "CLOSING THE COMMAND LIST FAILED" );

    ID3D12CommandList* command_lists[] = { m_render_command_list.Get() };
    m_command_queue->ExecuteCommandLists( _countof( command_lists ), command_lists );

  }

  /* This will present the swap chain in Vulkan and D3D12 */
  void dxContext::present_swap_chain() {

    HRESULT result = m_swap_chain->Present( 0, 0 );
    assert( result == S_OK && "PRESENTING THE SWAP CHAIN FAILED" );
    m_frame_index = m_swap_chain->GetCurrentBackBufferIndex();

  }

  /* This will wait for render to finish in Vulkan and D3D12 */
  void dxContext::wait_render_completition() {

    const UINT64 fence = m_fence_value;
    HRESULT result = m_command_queue->Signal( m_fence.Get(), fence );
    assert( result == S_OK && "ERROR SIGNALING THE FENCE" );
    m_fence_value++;

    if( m_fence->GetCompletedValue() < fence ) {
      result = m_fence->SetEventOnCompletion( fence, m_fence_event );
      assert( result == S_OK && "SET EVENT ON COMPLETITION FAILED" );
      WaitForSingleObject( m_fence_event, INFINITE );
    }

  }

  dxContext::dxContext() {

    m_debug_controller = nullptr;
    factory = nullptr;
    m_swap_chain = nullptr;
    m_device = nullptr;
    m_command_queue = nullptr;
    m_buffer_command_allocator = nullptr;
    m_buffer_command_list = nullptr;
    m_render_command_list = nullptr;
    m_texture_command_list = nullptr;
    m_texture_command_queue = nullptr;
    m_texture_command_allocator = nullptr;
    m_render_command_allocator = nullptr;
    m_msaa_render_target = nullptr;
    m_post_render_target = nullptr;
    m_depth_stencil = nullptr;
    m_rtv_heap = nullptr;
    m_dsv_heap = nullptr;
    m_sampler_heap = nullptr;
    m_fence = nullptr;

    m_fence_event = {};

    m_render_targets.clear();
    m_render_targets.shrink_to_fit();

    m_msaa_enabled = false;
    m_fence_texture_upload_pending = false;

    m_rtv_descriptor_size = 0;
    m_sampler_descriptor_size = 0;
    m_frame_index = 0;
    m_fence_value = 0;
    m_fence_texture_upload = 0;
    m_mssa_count = 1;

  }

  dxContext::~dxContext() {

    m_debug_controller = nullptr;
    factory = nullptr;
    m_swap_chain = nullptr;
    m_device = nullptr;
    m_command_queue = nullptr;
    m_buffer_command_list = nullptr;
    m_texture_command_queue = nullptr;

    m_render_command_list = nullptr;
    m_texture_command_list = nullptr;
    m_buffer_command_allocator = nullptr;
    m_texture_command_allocator = nullptr;
    m_render_command_allocator = nullptr;
    m_msaa_render_target = nullptr;
    m_post_render_target = nullptr;
    m_depth_stencil = nullptr;
    m_rtv_heap = nullptr;
    m_dsv_heap = nullptr;
    m_sampler_heap = nullptr;
    m_fence = nullptr;

    m_fence_event = {};

    m_render_targets.clear();
    m_render_targets.shrink_to_fit();

  }

}