#include "core/dx/texture.hh"
#include "core/dx/context.hh"
#include "core/dx/renderer.hh"
#include "core/engine.hh"
#include <cassert>

namespace kretash {

  dxTexture::dxTexture() {
    m_texture = nullptr;
    m_texture_upload = nullptr;
  }

  dxTexture::~dxTexture() {
    m_texture = nullptr;
    m_texture_upload = nullptr;
  }

  /* Creates a texture in Vulkan and D3D12 */
  void dxTexture::create_texture( void* data, int32_t width, int32_t height, int32_t channels ) {

    HRESULT result;
    dxContext* m_context = dynamic_cast< dxContext* >( k_engine->get_context() );

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

    result = m_context->m_device->CreateCommittedResource(
      &CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_DEFAULT ),
      D3D12_HEAP_FLAG_NONE,
      &textureDesc,
      D3D12_RESOURCE_STATE_COPY_DEST,
      nullptr,
      IID_PPV_ARGS( &m_texture ) );
    assert( result == S_OK && "CreateCommittedResource FALILED" );

    const UINT subresourceCount = textureDesc.DepthOrArraySize * textureDesc.MipLevels;
    const UINT64 uploadBufferSize = GetRequiredIntermediateSize( m_texture.Get(), 0, subresourceCount );

    result = m_context->m_device->CreateCommittedResource(
      &CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_UPLOAD ),
      D3D12_HEAP_FLAG_NONE,
      &CD3DX12_RESOURCE_DESC::Buffer( uploadBufferSize ),
      D3D12_RESOURCE_STATE_GENERIC_READ,
      nullptr,
      IID_PPV_ARGS( &m_texture_upload ) );
    assert( result == S_OK && "CreateCommittedResource FALILED" );

    D3D12_SUBRESOURCE_DATA textureData = {};
    textureData.pData = data;
    textureData.RowPitch = static_cast< LONG_PTR >( ( channels * width ) );;
    textureData.SlicePitch = textureData.RowPitch*height;

    UpdateSubresources( m_context->m_texture_command_list.Get(), m_texture.Get(), m_texture_upload.Get(),
      0, 0, subresourceCount, &textureData );
    m_context->m_texture_command_list->ResourceBarrier( 1,
      &CD3DX12_RESOURCE_BARRIER::Transition( m_texture.Get(),
        D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE ) );
  }

  /* Creates a texture view in Vulkan and D3D12 */
  void dxTexture::create_shader_resource_view( xxRenderer* r, int32_t offset ) {

    dxContext* m_context = dynamic_cast< dxContext* >( k_engine->get_context() );
    dxRenderer* m_renderer = dynamic_cast< dxRenderer* >( r );

    D3D12_SHADER_RESOURCE_VIEW_DESC diffuseSrvDesc = {};
    diffuseSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    diffuseSrvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    diffuseSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    diffuseSrvDesc.Texture2D.MipLevels = 1;

    CD3DX12_CPU_DESCRIPTOR_HANDLE SrvHandle(
      m_renderer->m_srv_heap->GetCPUDescriptorHandleForHeapStart(),
      offset,
      m_renderer->m_cbv_srv_descriptor_size );

    m_context->m_device->CreateShaderResourceView( m_texture.Get(), &diffuseSrvDesc, SrvHandle );

  }

  /* Clears the view resources in Vulkan and D3D12 */
  void dxTexture::clear_texture_upload() {
    m_texture_upload = nullptr;
  }

  /* Clears the whole texture in Vulkan and D3D12 */
  void dxTexture::clear_texture() {
    m_texture = nullptr;
  }
  
}