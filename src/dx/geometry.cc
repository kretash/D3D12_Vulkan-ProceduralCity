#include "core/dx/geometry.hh"
#include "core/dx/context.hh"
#include "core/engine.hh"
#include <cassert>

namespace kretash {

  dxGeometry::dxGeometry(){
  
  }

  dxGeometry::~dxGeometry(){
  
  }

  /* This will create an empty vertex buffer in Vulkan and D3D12 */
  void dxGeometry::create_empty_vertex_buffer( uint64_t size ) {

    HRESULT result;
    dxContext* m_context = dynamic_cast<dxContext*>( k_engine->get_context() );

    const UINT data_buffer_size = sizeof( float ) * ( UINT ) size;
    CD3DX12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_UPLOAD );
    CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer( data_buffer_size );

    result = m_context->m_device->CreateCommittedResource( &heapProperties, D3D12_HEAP_FLAG_NONE,
      &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS( &m_vertexBuffer ) );

    assert( result == S_OK && "CREATING THE VERTEX BUFFER FAILED" );
    m_vertexBuffer->SetName( L"bbVertexBuffer" );

    m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
    m_vertexBufferView.StrideInBytes = sizeof( float ) * m_context->m_stride;
    m_vertexBufferView.SizeInBytes = data_buffer_size;

    UINT8* data_begin;
    result = m_vertexBuffer->Map( 0, nullptr, reinterpret_cast< void** >( &data_begin ) );
    assert( result == S_OK && "MAPPING THE VERTEX BUFFER FAILED" );
    memset( data_begin, 0, data_buffer_size );
    m_vertexBuffer->Unmap( 0, nullptr );  

  }

  /* This will create an empty index buffer in Vulkan and D3D12 */
  void dxGeometry::create_empty_index_buffer( uint64_t size ) {

    HRESULT result;
    dxContext* m_context = dynamic_cast<dxContext*>( k_engine->get_context() );

    const UINT indices_buffer_size = sizeof( unsigned int ) * ( UINT ) size;
    CD3DX12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_UPLOAD );
    CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer( indices_buffer_size );

    result = m_context->m_device->CreateCommittedResource( &heapProperties, D3D12_HEAP_FLAG_NONE,
      &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS( &m_indexBuffer ) );

    assert( result == S_OK && "CREATING THE INDEX BUFFER FAILED" );
    m_indexBuffer->SetName( L"bbIndexBuffer" );

    m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
    m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;
    m_indexBufferView.SizeInBytes = indices_buffer_size;

    UINT8* data_begin;
    result = m_indexBuffer->Map( 0, nullptr, reinterpret_cast< void** >( &data_begin ) );
    assert( result == S_OK && "MAPPING THE INDEX BUFFER FAILED" );
    memset( data_begin, 0, indices_buffer_size );
    m_indexBuffer->Unmap( 0, nullptr );
  
  }

  /* This will upload into an vertex buffer in Vulkan and D3D12 */
  void dxGeometry::upload_into_vertex_buffer( uint64_t offset, float* array_data, uint64_t size ) {

    UINT8* data_begin = nullptr;
    D3D12_RANGE range = { offset, offset + size* sizeof( float ) };
    HRESULT result = m_vertexBuffer->Map( 0, &range, reinterpret_cast< void** >( &data_begin ) );

    assert( result == S_OK && "MAPPING THE VERTEX BUFFER FAILED" );

    UINT8* data_offset = data_begin + offset;
    memcpy( data_offset, array_data, size * sizeof( float ) );
    m_vertexBuffer->Unmap( 0, nullptr );

  }

  /* This will queue an upload into an vertex buffer in Vulkan and D3D12 */
  void dxGeometry::upload_queue_into_vertex_buffer( std::vector<kretash::queue>* queue ) {

    UINT8* data_begin = nullptr;
    HRESULT result = m_vertexBuffer->Map( 0, nullptr, reinterpret_cast< void** >( &data_begin ) );

    for( int i = 0; i < queue->size(); ++i ) {
      UINT8* data_offset = data_begin + ( *queue )[i].v_block.m_start;
      memcpy( data_offset, ( *queue )[i].v_data, ( *queue )[i].v_block.m_size );
    }

    m_vertexBuffer->Unmap( 0, nullptr );

  }

  /* This will upload into an index buffer in Vulkan and D3D12 */
  void dxGeometry::upload_into_index_buffer( uint64_t offset, uint32_t* elements_data, uint64_t size ) {

    UINT8* data_begin = nullptr;
    D3D12_RANGE range = { offset, offset + size  * sizeof( uint32_t ) };
    HRESULT result = m_indexBuffer->Map( 0, &range, reinterpret_cast< void** >( &data_begin ) );

    assert( result == S_OK && "MAPPING THE INDEX BUFFER FAILED" );

    UINT8* data_offset = data_begin + offset;
    memcpy( data_offset, elements_data, size * sizeof( uint32_t ) );
    m_indexBuffer->Unmap( 0, nullptr );

  }

  /* This will queue an upload into an index buffer in Vulkan and D3D12 */
  void dxGeometry::upload_queue_into_index_buffer( std::vector<kretash::queue>* queue ) {

    UINT8* data_begin = nullptr;
    HRESULT result = m_indexBuffer->Map( 0, nullptr, reinterpret_cast< void** >( &data_begin ) );

    for( int i = 0; i < queue->size(); ++i ) {
      UINT8* data_offset = data_begin + ( *queue )[i].i_block.m_start;
      memcpy( data_offset, ( *queue )[i].i_data, ( *queue )[i].i_block.m_size );
    }

    m_indexBuffer->Unmap( 0, nullptr );

  }

}