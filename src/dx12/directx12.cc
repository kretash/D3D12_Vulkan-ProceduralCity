#include "core/k_directx12.hh"

#ifdef __DIRECTX12__

#include <cassert>
#include "core/window.hh"
#include "core/drawable.hh"
#include "core/engine_settings.hh"

namespace dx {

#ifdef _DEBUG
  void enable_debug_layer( engine_data* d ) {
    if( SUCCEEDED( D3D12GetDebugInterface( IID_PPV_ARGS( &d->m_debug_controller ) ) ) ) {
      d->m_debug_controller->EnableDebugLayer();
    }
  }
#endif

  void create_factory( engine_data* d ) {
    HRESULT result;
    result = D3D12CreateDevice( nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS( &d->m_device ) );
    assert( result == S_OK && "CREATING THE DEVICE FAILED" );
  }

  void create_device( engine_data* d ) {
    HRESULT result;
    result = CreateDXGIFactory1( IID_PPV_ARGS( &d->factory ) );
    assert( result == S_OK && "CREATING THE FACTORY FAILED" );
  }

  void create_command_queue( engine_data* d ) {
    HRESULT result;
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    result = d->m_device->CreateCommandQueue( &queueDesc, IID_PPV_ARGS( &d->m_command_queue ) );
    assert( result == S_OK && "ERROR CREATING THE COMMAND QUEUE" );

    result = d->m_device->CreateCommandQueue( &queueDesc, IID_PPV_ARGS( &d->m_texture_command_queue ) );
    assert( result == S_OK && "ERROR CREATING THE COMMAND QUEUE" );
  }

  void create_swap_chain( engine_data* d, Window* w ) {
    HRESULT result;
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
    result = d->factory->CreateSwapChain( d->m_command_queue.Get(), &swapChainDesc, &swapChain );
    assert( result == S_OK && "ERROR CREATING THE SWAP CHAIN" );

    swapChain.As( &d->m_swap_chain );
    d->m_frame_index = d->m_swap_chain->GetCurrentBackBufferIndex();

    swapChain->SetFullscreenState( k_engine_settings->get_settings().fullscreen, nullptr );
  }

  void create_command_allocator( engine_data* d ) {
    HRESULT result;

    result = d->m_device->CreateCommandAllocator( D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS( &d->m_texture_command_allocator ) );
    assert( result == S_OK && "ERROR CREATING THE TEXTURE COMMAND ALLOCATOR" );


    result = d->m_device->CreateCommandAllocator( D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS( &d->m_buffer_command_allocator ) );
    assert( result == S_OK && "ERROR CREATING THE BUFFER COMMAND ALLOCATOR" );


    result = d->m_device->CreateCommandAllocator( D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS( &d->m_render_command_allocator ) );
    assert( result == S_OK && "ERROR CREATING THE RENDER COMMAND ALLOCATOR" );
  }

  void create_command_list( engine_data* d ) {
    HRESULT result;

    result = d->m_device->CreateCommandList( 0, D3D12_COMMAND_LIST_TYPE_DIRECT, d->m_texture_command_allocator.Get(),
      nullptr, IID_PPV_ARGS( &d->m_texture_command_list ) );
    assert( result == S_OK && "ERROR CREATING THE TEXTURE COMMNAND LIST" );

    result = d->m_texture_command_list->Close();
    assert( result == S_OK && "ERROR CLOSING THE TEXTURE COMMAND LIST" );


    result = d->m_device->CreateCommandList( 0, D3D12_COMMAND_LIST_TYPE_DIRECT, d->m_buffer_command_allocator.Get(),
      nullptr, IID_PPV_ARGS( &d->m_buffer_command_list ) );
    assert( result == S_OK && "ERROR CREATING THE TEXTURE COMMNAND LIST" );

    result = d->m_buffer_command_list->Close();
    assert( result == S_OK && "ERROR CLOSING THE TEXTURE COMMAND LIST" );


    result = d->m_device->CreateCommandList( 0, D3D12_COMMAND_LIST_TYPE_DIRECT, d->m_render_command_allocator.Get(),
      nullptr, IID_PPV_ARGS( &d->m_render_command_list ) );
    assert( result == S_OK && "ERROR CREATING THE COMMNAND LIST" );

    result = d->m_render_command_list->Close();
    assert( result == S_OK && "ERROR CLOSING THE COMMAND LIST" );
  }



  void create_render_target_view_heap( engine_data* e ) {
    HRESULT result;
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = m_frame_count + 2;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    result = e->m_device->CreateDescriptorHeap( &rtvHeapDesc, IID_PPV_ARGS( &e->m_rtv_heap ) );
    assert( result == S_OK && "ERROR CREATING THE DESCRIPTOR HEAP" );
    e->m_rtv_descriptor_size = e->m_device->GetDescriptorHandleIncrementSize( D3D12_DESCRIPTOR_HEAP_TYPE_RTV );
  }

  void create_depth_stencil_view_heap( engine_data* e ) {
    HRESULT result;
    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    result = e->m_device->CreateDescriptorHeap( &dsvHeapDesc, IID_PPV_ARGS( &e->m_dsv_heap ) );
    assert( result == S_OK && "ERROR CREATING THE DSV HEAP" );
  }

  void create_render_target_view( engine_data* e, Window* w ) {
    HRESULT result;

    e->m_msaa_enabled = k_engine_settings->get_settings().msaa_enabled;
    e->m_mssa_count = k_engine_settings->get_settings().msaa_count;
    float u_r = k_engine_settings->get_settings().upscale_render;

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle( e->m_rtv_heap->GetCPUDescriptorHandleForHeapStart() );

    for( UINT n = 0; n < m_frame_count; ++n ) {
      result = e->m_swap_chain->GetBuffer( n, IID_PPV_ARGS( &e->m_render_targets[n] ) );
      assert( result == S_OK && "CREATING RTV FAILED" );

      D3D12_RENDER_TARGET_VIEW_DESC render_target_desc = {};
      render_target_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
      render_target_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

      e->m_device->CreateRenderTargetView( e->m_render_targets[n].Get(), &render_target_desc,
        rtvHandle );
      rtvHandle.Offset( 1, e->m_rtv_descriptor_size );
    }
    {
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

      if( e->m_msaa_enabled ){
        color_texture.SampleDesc.Count = e->m_mssa_count;
        color_texture.SampleDesc.Quality = DXGI_STANDARD_MULTISAMPLE_QUALITY_PATTERN;
      }

      D3D12_CLEAR_VALUE clear_value;
      clear_value.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

      D3D12_RENDER_TARGET_VIEW_DESC render_target_desc = {};
      render_target_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
      render_target_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;

      result = e->m_device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_DEFAULT ),
        D3D12_HEAP_FLAG_NONE,
        &color_texture,
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        &clear_value,
        IID_PPV_ARGS( &e->m_msaa_render_target ) );

      assert( result == S_OK && "CREATING THE RTV FAILED" );

      e->m_device->CreateRenderTargetView(
        e->m_msaa_render_target.Get(),
        &render_target_desc,
        rtvHandle );
      rtvHandle.Offset( 1, e->m_rtv_descriptor_size );
    }
    {
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

      result = e->m_device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_DEFAULT ),
        D3D12_HEAP_FLAG_NONE,
        &color_texture,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        &clear_value,
        IID_PPV_ARGS( &e->m_post_render_target ) );

      assert( result == S_OK && "CREATING THE RTV FAILED" );

      e->m_device->CreateRenderTargetView(
        e->m_post_render_target.Get(),
        &render_target_desc,
        rtvHandle );
      rtvHandle.Offset( 1, e->m_rtv_descriptor_size );
    }
  }

  void create_depth_stencil_view( engine_data* e, Window* w ) {
    HRESULT result;

    e->m_msaa_enabled = k_engine_settings->get_settings().msaa_enabled;
    e->m_mssa_count = k_engine_settings->get_settings().msaa_count;
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

    if( e->m_msaa_enabled ) {
      depth_texture.SampleDesc.Count = e->m_mssa_count;
      depth_texture.SampleDesc.Quality = DXGI_STANDARD_MULTISAMPLE_QUALITY_PATTERN;
    }

    D3D12_CLEAR_VALUE clear_value;
    clear_value.Format = DXGI_FORMAT_D32_FLOAT;
    clear_value.DepthStencil.Depth = 1.0f;
    clear_value.DepthStencil.Stencil = 0;

    D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
    depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
    depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
    depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

    result = k_engine->get_engine_data()->m_device->CreateCommittedResource( &CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_DEFAULT ),
      D3D12_HEAP_FLAG_NONE, &depth_texture, D3D12_RESOURCE_STATE_DEPTH_WRITE, &clear_value,
      IID_PPV_ARGS( &e->m_depth_stencil ) );
    assert( result == S_OK && "CREATING THE DEPTH STENCIL FAILED" );

    k_engine->get_engine_data()->m_device->CreateDepthStencilView( e->m_depth_stencil.Get(), &depthStencilDesc,
      e->m_dsv_heap->GetCPUDescriptorHandleForHeapStart() );

  }
  void create_fences( engine_data* d ) {
    HRESULT result;
    result = d->m_device->CreateFence( 0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS( &d->m_fence ) );
    assert( result == S_OK && "ERROR CREATING THE FENCE" );
    d->m_fence_value = 1;

    d->m_fence_event = CreateEventEx( nullptr, FALSE, FALSE, EVENT_ALL_ACCESS );
    assert( d->m_fence_event != nullptr && "ERROR CREATING THE FENCE EVENT" );

  }

  void wait_for_previous_frame( engine_data* d ) {
    const UINT64 fence = d->m_fence_value;
    HRESULT result = d->m_command_queue->Signal( d->m_fence.Get(), fence );
    assert( result == S_OK && "ERROR SIGNALING THE FENCE" );
    d->m_fence_value++;

    if( d->m_fence->GetCompletedValue() < fence ) {
      result = d->m_fence->SetEventOnCompletion( fence, d->m_fence_event );
      assert( result == S_OK && "SET EVENT ON COMPLETITION FAILED" );
      WaitForSingleObject( d->m_fence_event, INFINITE );
    }

    d->m_frame_index = d->m_swap_chain->GetCurrentBackBufferIndex();
  }
}

#endif