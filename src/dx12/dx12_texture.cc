#include "core/k_directx12.hh"

#ifdef __DIRECTX12__

#include <cassert>
#include "core/engine.hh"
#include "core/window.hh"
#include "core/drawable.hh"

namespace dx {

  void reset_texture_command_list( engine_data* d ) {
    HRESULT result;

    result = d->m_texture_command_allocator->Reset();
    assert( result == S_OK && "COMMAND ALLOCATOR RESET FAILED" );

    result = d->m_texture_command_list->Reset( d->m_texture_command_allocator.Get(), nullptr );
    assert( result == S_OK && "COMMAND LIST RESET FAILED" );
  }

  void create_texture( engine_data* d, texture_data* t, void* data, int32_t width, int32_t height, int32_t channels ) {
    HRESULT result;

    assert( channels == 4 && "MISSING IMPLEMENTATION FOR !4 CHANNELS" );

    D3D12_RESOURCE_DESC textureDesc{};
    textureDesc.MipLevels = 1;
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    textureDesc.Width = width;
    textureDesc.Height = height;
    textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
    textureDesc.DepthOrArraySize = 1;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

    result = d->m_device->CreateCommittedResource(
      &CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_DEFAULT ),
      D3D12_HEAP_FLAG_NONE,
      &textureDesc,
      D3D12_RESOURCE_STATE_COPY_DEST,
      nullptr,
      IID_PPV_ARGS( &t->m_texture ) );
    assert( result == S_OK && "CreateCommittedResource FALILED" );

    const UINT subresourceCount = textureDesc.DepthOrArraySize * textureDesc.MipLevels;
    const UINT64 uploadBufferSize = GetRequiredIntermediateSize( t->m_texture.Get(), 0, subresourceCount );

    result = d->m_device->CreateCommittedResource(
      &CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_UPLOAD ),
      D3D12_HEAP_FLAG_NONE,
      &CD3DX12_RESOURCE_DESC::Buffer( uploadBufferSize ),
      D3D12_RESOURCE_STATE_GENERIC_READ,
      nullptr,
      IID_PPV_ARGS( &t->m_texture_upload ) );
    assert( result == S_OK && "CreateCommittedResource FALILED" );

    D3D12_SUBRESOURCE_DATA textureData = {};
    textureData.pData = data;
    textureData.RowPitch = static_cast< LONG_PTR >( ( channels * width ) );;
    textureData.SlicePitch = textureData.RowPitch*height;

    UpdateSubresources( d->m_texture_command_list.Get(), t->m_texture.Get(), t->m_texture_upload.Get(), 0, 0, subresourceCount, &textureData );
    d->m_texture_command_list->ResourceBarrier( 1, &CD3DX12_RESOURCE_BARRIER::Transition( t->m_texture.Get(),
      D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE ) );
  }

  void create_shader_resource_view( engine_data* d, renderer_data* r, texture_data* t, int32_t offset ) {
    D3D12_SHADER_RESOURCE_VIEW_DESC diffuseSrvDesc = {};
    diffuseSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    diffuseSrvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    diffuseSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    diffuseSrvDesc.Texture2D.MipLevels = 1;

    CD3DX12_CPU_DESCRIPTOR_HANDLE SrvHandle(
      r->m_srv_heap->GetCPUDescriptorHandleForHeapStart(),
      offset,
      r->m_cbv_srv_descriptor_size );

    k_engine->get_engine_data()->m_device->CreateShaderResourceView( t->m_texture.Get(), &diffuseSrvDesc, SrvHandle );
  }

  void compute_texture_upload( engine_data* d ) {
    HRESULT result;

    result = d->m_texture_command_list->Close();
    assert( result == S_OK && "ERROR CLOSING THE COMMAND LIST" );
    ID3D12CommandList* ppCommandLists[] = { d->m_texture_command_list.Get() };
    d->m_texture_command_queue->ExecuteCommandLists( _countof( ppCommandLists ), ppCommandLists );

    result = d->m_device->CreateFence( d->m_fence_value, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS( &d->m_fence ) );
    assert( result == S_OK && "ERROR CREATING FENCE" );
    d->m_fence_value++;

    // Create an event handle to use for frame synchronization.
    d->m_fence_event = CreateEvent( nullptr, FALSE, FALSE, nullptr );
    if( d->m_fence_event == nullptr ) {
      HRESULT_FROM_WIN32( GetLastError() );
    }

    d->m_fence_texture_upload_pending = true;
    d->m_fence_texture_upload = d->m_fence_value;
    d->m_fence_value++;

    result = d->m_texture_command_queue->Signal( d->m_fence.Get(), d->m_fence_texture_upload );
    assert( result == S_OK && "ERROR SIGNALING FENCE" );


  }

  void wait_for_texture_upload( engine_data* d ) {
    HRESULT result;

    if( d->m_fence_texture_upload_pending ) {
      // Wait until the fence is completed.
      d->m_fence_texture_upload_pending = false;

      if( d->m_fence->GetCompletedValue() < d->m_fence_texture_upload ) {
        result = d->m_fence->SetEventOnCompletion( d->m_fence_texture_upload, d->m_fence_event );
        assert( result == S_OK && "SET EVENT ON COMPLETITION FAILED" );
        WaitForSingleObject( d->m_fence_event, INFINITE );
      }

    }
  }

  void clear_texture_upload( engine_data* d, texture_data* t ) {
    t->m_texture_upload = nullptr;
  }

  void clear_texture( engine_data* d, texture_data* t ) {
    t->m_texture = nullptr;
  }

  void clear_descriptor_set( engine_data* e, texture_data* pt, uint32_t offset ) {
  
  }
}

#endif