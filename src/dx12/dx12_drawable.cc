#include "core/k_directx12.hh"

#ifdef __DIRECTX12__

#include <cassert>
#include "core/engine.hh"
#include "core/window.hh"
#include "core/drawable.hh"

namespace dx {
  void create_constant_buffer_object( constant_buffer_data* cbd, constant_buffer cb ) {
    HRESULT result;

    //Constant buffers // Uniforms
    const UINT buffer_size = sizeof( constant_buffer ) + 255 & ~255;  //has to be a multiple of 256bytes
    CD3DX12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_UPLOAD );
    CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer( sizeof( constant_buffer ) );
    result = k_engine->get_engine_data()->m_device->CreateCommittedResource(
      &heapProperties,
      D3D12_HEAP_FLAG_NONE,
      &resourceDesc,
      D3D12_RESOURCE_STATE_GENERIC_READ,
      nullptr,
      IID_PPV_ARGS( &cbd->m_constant_buffer ) );
    assert( result == S_OK && "CREATING THE CONSTANT BUFFER FAILED" );
    cbd->m_constant_buffer->SetName(L"CBV");
  }

  void update_constant_buffer_object( constant_buffer_data* cbd, constant_buffer cb ) {
    HRESULT result = cbd->m_constant_buffer->Map( 0, nullptr, 
      reinterpret_cast< void** >( &cbd->m_constant_buffer_WO ) );
    assert( result == S_OK && "MAPPING THE CONSTANT BUFFER FALILED" );
    memcpy( cbd->m_constant_buffer_WO, &cb, sizeof( cb ) );
    cbd->m_constant_buffer->Unmap( 0, nullptr );
  }
  
  void create_instance_buffer_object( renderer_data* r, instance_buffer* ub) {
    HRESULT result;

    CD3DX12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_UPLOAD );
    CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer( 
      sizeof( instance_buffer ) * k_engine->get_total_drawables() );

    result = k_engine->get_engine_data()->m_device->CreateCommittedResource(
      &heapProperties,
      D3D12_HEAP_FLAG_NONE,
      &resourceDesc,
      D3D12_RESOURCE_STATE_GENERIC_READ,
      nullptr,
      IID_PPV_ARGS( &r->m_instance_buffer ) );
    assert( result == S_OK && "CREATING THE CONSTANT BUFFER FAILED" );
    r->m_instance_buffer->SetName( L"INSTANCE BUFFER" );

  }

  void create_instance_buffer_view( renderer_data* r, drawable_data* d, uint64_t buffer_offset, int32_t offset ){
    const UINT buffer_size = sizeof( instance_buffer ) + 255 & ~255;
    r->m_instance_buffer_desc = {};
    D3D12_GPU_VIRTUAL_ADDRESS addr = r->m_instance_buffer->GetGPUVirtualAddress();
    r->m_instance_buffer_desc.BufferLocation = addr + buffer_offset;
    r->m_instance_buffer_desc.SizeInBytes = buffer_size;

    CD3DX12_CPU_DESCRIPTOR_HANDLE cbvSrvHandle(
      r->m_cbv_heap->GetCPUDescriptorHandleForHeapStart(),
      offset,
      r->m_cbv_srv_descriptor_size );

    k_engine->get_engine_data()->m_device->CreateConstantBufferView( &r->m_instance_buffer_desc, cbvSrvHandle );
  
  }

  void update_instance_buffer_object( renderer_data* r, instance_buffer* ub ) {
    HRESULT result = r->m_instance_buffer->Map( 0, nullptr, reinterpret_cast< void** >( &r->m_instance_buffer_WO ) );
    assert( result == S_OK && "MAPPING THE CONSTANT BUFFER FALILED" );
    memcpy( r->m_instance_buffer_WO, ub, sizeof( instance_buffer ) * k_engine->get_total_drawables() );
    r->m_instance_buffer->Unmap( 0, nullptr );
  }
}

#endif