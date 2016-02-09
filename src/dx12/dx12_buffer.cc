#include "core/k_directx12.hh"

#ifdef __DIRECTX12__

#include <cassert>
#include <iostream>
#include "core/types.hh"
#include "core/engine.hh"
#include "core/window.hh"
#include "core/drawable.hh"
#include "core/GPU_pool.hh"

namespace dx {
  void create_empty_vertex_buffer( geometry_data* g, uint32_t size ) {
    HRESULT result;
    const UINT data_buffer_size = sizeof( float ) * ( UINT ) size;
    CD3DX12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_UPLOAD );
    CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer( data_buffer_size );

    result = k_engine->get_engine_data()->m_device->CreateCommittedResource( &heapProperties, D3D12_HEAP_FLAG_NONE,
      &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS( &g->m_vertexBuffer ) );

    assert( result == S_OK && "CREATING THE VERTEX BUFFER FAILED" );
    g->m_vertexBuffer->SetName( L"bbVertexBuffer" );

    g->m_vertexBufferView.BufferLocation = g->m_vertexBuffer->GetGPUVirtualAddress();
    g->m_vertexBufferView.StrideInBytes = sizeof( float ) * m_stride;
    g->m_vertexBufferView.SizeInBytes = data_buffer_size;

    UINT8* data_begin;
    result = g->m_vertexBuffer->Map( 0, nullptr, reinterpret_cast< void** >( &data_begin ) );
    assert( result == S_OK && "MAPPING THE VERTEX BUFFER FAILED" );
    memset( data_begin, 0, data_buffer_size );
    g->m_vertexBuffer->Unmap( 0, nullptr );
  }

  void create_empty_index_buffer( geometry_data* g, uint32_t size ) {
    HRESULT result;
    const UINT indices_buffer_size = sizeof( unsigned int ) * ( UINT ) size;
    CD3DX12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_UPLOAD );
    CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer( indices_buffer_size );

    result = k_engine->get_engine_data()->m_device->CreateCommittedResource( &heapProperties, D3D12_HEAP_FLAG_NONE,
      &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS( &g->m_indexBuffer ) );

    assert( result == S_OK && "CREATING THE INDEX BUFFER FAILED" );
    g->m_indexBuffer->SetName( L"bbIndexBuffer" );

    g->m_indexBufferView.BufferLocation = g->m_indexBuffer->GetGPUVirtualAddress();
    g->m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;
    g->m_indexBufferView.SizeInBytes = indices_buffer_size;

    UINT8* data_begin;
    result = g->m_indexBuffer->Map( 0, nullptr, reinterpret_cast< void** >( &data_begin ) );
    assert( result == S_OK && "MAPPING THE INDEX BUFFER FAILED" );
    memset( data_begin, 0, indices_buffer_size );
    g->m_indexBuffer->Unmap( 0, nullptr );
  }

  void upload_into_vertex_buffer( geometry_data* g, uint32_t offset, float* array_data, uint32_t size ) {
    UINT8* data_begin;
    D3D12_RANGE range = { offset, offset + size* sizeof( float ) };
    HRESULT result = g->m_vertexBuffer->Map( 0, &range, reinterpret_cast< void** >( &data_begin ) );

    assert( result == S_OK && "MAPPING THE VERTEX BUFFER FAILED" );

    UINT8* data_offset = data_begin + offset;
    memcpy( data_offset, array_data, size * sizeof( float ) );
    g->m_vertexBuffer->Unmap( 0, nullptr );

  }

  void upload_queue_into_vertex_buffer( geometry_data* g, std::vector<queue>* queue ){
    UINT8* data_begin;
    HRESULT result = g->m_vertexBuffer->Map( 0, nullptr, reinterpret_cast< void** >( &data_begin ) );

    for( int i = 0; i < queue->size(); ++i ){
      UINT8* data_offset = data_begin + ( *queue )[i].v_block.m_start;
      memcpy( data_offset, ( *queue )[i].v_data, ( *queue )[i].v_block.m_size );
    }

    g->m_vertexBuffer->Unmap( 0, nullptr );
  }

  void upload_into_index_buffer( geometry_data* g, uint32_t offset, uint32_t* elements_data, uint32_t size ) {

    UINT8* data_begin;
    D3D12_RANGE range = { offset, offset + size  * sizeof( uint32_t ) };
    HRESULT result = g->m_indexBuffer->Map( 0, &range, reinterpret_cast< void** >( &data_begin ) );

    assert( result == S_OK && "MAPPING THE INDEX BUFFER FAILED" );

    UINT8* data_offset = data_begin + offset;
    memcpy( data_offset, elements_data, size * sizeof( uint32_t ) );
    g->m_indexBuffer->Unmap( 0, nullptr );
  }

  void upload_queue_into_index_buffer( geometry_data* g, std::vector<queue>* queue ) {
    UINT8* data_begin;
    HRESULT result = g->m_indexBuffer->Map( 0, nullptr, reinterpret_cast< void** >( &data_begin ) );

    for( int i = 0; i < queue->size(); ++i ) {
      UINT8* data_offset = data_begin + ( *queue )[i].i_block.m_start;
      memcpy( data_offset, ( *queue )[i].i_data, ( *queue )[i].i_block.m_size );
    }

    g->m_indexBuffer->Unmap( 0, nullptr );
  }

}

#endif