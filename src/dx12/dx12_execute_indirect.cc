#include "core/k_directx12.hh"

#ifdef __DIRECTX12__

#include "core/engine_settings.hh"
#include "core/texture_manager.hh"
#include "core/drawable.hh"
#include "core/GPU_pool.hh"
#include "core/geometry.hh"
#include "core/texture.hh"
#include "core/world.hh"
#include <cassert>
#include <iostream>

namespace dx {

  void create_command_signature( engine_data* e, renderer_data* r ) {
    D3D12_INDIRECT_ARGUMENT_DESC argument_desc[2];
    argument_desc[0].Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT_BUFFER_VIEW;
    argument_desc[0].ConstantBufferView.RootParameterIndex = GRP_INSTANCE_CBV;
    argument_desc[1].Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;

    D3D12_COMMAND_SIGNATURE_DESC command_signature_desc = {};
    command_signature_desc.pArgumentDescs = argument_desc;
    command_signature_desc.NumArgumentDescs = _countof( argument_desc );
    command_signature_desc.ByteStride = sizeof( indirect_command );
    command_signature_desc.NodeMask = 1;

    HRESULT result = e->m_device->CreateCommandSignature( &command_signature_desc,
      r->m_root_signature.Get(), IID_PPV_ARGS( &r->m_command_signature ) );
    assert( result == S_OK && "CREATING THE COMMAND SIGNATURE FAILED" );
  }

  void create_and_fill_command_buffer( engine_data* d, renderer_data* r, Drawable** draw, uint32_t d_count ) {

    HRESULT result = ~S_OK;

    result = d->m_buffer_command_allocator->Reset();
    assert( result == S_OK && "COMMAND ALLOCATOR RESET FAILED" );

    result = d->m_buffer_command_list->Reset( d->m_buffer_command_allocator.Get(), nullptr );
    assert( result == S_OK && "COMMAND LIST RESET FAILED" );

    UINT command_buffer_size = d_count * sizeof( indirect_command );

    D3D12_RESOURCE_DESC command_buffer_desc = CD3DX12_RESOURCE_DESC::Buffer( command_buffer_size );

    result = d->m_device->CreateCommittedResource(
      &CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_DEFAULT ),
      D3D12_HEAP_FLAG_NONE,
      &command_buffer_desc,
      D3D12_RESOURCE_STATE_COPY_DEST,
      nullptr,
      IID_PPV_ARGS( &r->m_command_buffer ) );
    assert( result == S_OK && "CREATING THE COMMAND BUFFER FAILED" );
    r->m_command_buffer->SetName( L"CommandBuffer" );

    result = d->m_device->CreateCommittedResource(
      &CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_UPLOAD ),
      D3D12_HEAP_FLAG_NONE,
      &CD3DX12_RESOURCE_DESC::Buffer( command_buffer_size ),
      D3D12_RESOURCE_STATE_GENERIC_READ,
      nullptr,
      IID_PPV_ARGS( &r->m_command_buffer_upload ) );
    assert( result == S_OK && "CREATING THE COMMAND BUFFER UPLOAD FAILED" );
    r->m_command_buffer_upload->SetName( L"CommandBufferUpload" );

    D3D12_GPU_VIRTUAL_ADDRESS addr = r->m_instance_buffer->GetGPUVirtualAddress();

    UINT index = 0;
    r->m_indirect_commands.resize( d_count );

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

      r->m_indirect_commands[index] = tmp;

    }

    D3D12_SUBRESOURCE_DATA command_data = {};
    command_data.pData = reinterpret_cast< UINT8* >( &r->m_indirect_commands[0] );
    command_data.RowPitch = command_buffer_size;
    command_data.SlicePitch = command_buffer_size;

    UpdateSubresources<1>( d->m_buffer_command_list.Get(), r->m_command_buffer.Get(),
      r->m_command_buffer_upload.Get(), 0, 0, 1, &command_data );
    d->m_buffer_command_list->ResourceBarrier( 1, &CD3DX12_RESOURCE_BARRIER::Transition( r->m_command_buffer.Get(),
      D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE ) );

    result = d->m_buffer_command_list->Close();
    assert( result == S_OK && "ERROR CLOSING THE COMMAND LIST" );
    ID3D12CommandList* ppCommandLists[] = { d->m_buffer_command_list.Get() };
    d->m_command_queue->ExecuteCommandLists( _countof( ppCommandLists ), ppCommandLists );

    //Wait for upload
    result = d->m_device->CreateFence( d->m_fence_value, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS( &d->m_fence ) );
    assert( result == S_OK && "ERROR CREATING FENCE" );
    d->m_fence_value++;

    // Create an event handle to use for frame synchronization.
    d->m_fence_event = CreateEvent( nullptr, FALSE, FALSE, nullptr );
    if( d->m_fence_event == nullptr ) {
      HRESULT_FROM_WIN32( GetLastError() );
    }

    const UINT64 fenceToWaitFor = d->m_fence_value;
    result = d->m_command_queue->Signal( d->m_fence.Get(), fenceToWaitFor );
    assert( result == S_OK && "ERROR SIGNALING FENCE" );
    d->m_fence_value++;

    // Wait until the fence is completed.
    result = d->m_fence->SetEventOnCompletion( fenceToWaitFor, d->m_fence_event );
    assert( result == S_OK && "ERROR SETTING COMPLETITION EVENT" );
    WaitForSingleObject( d->m_fence_event, INFINITE );

  }

  void update_command_buffer( engine_data* d, renderer_data* r, Drawable** draw, uint32_t d_count ) {

    uint32_t index = 0;
    D3D12_GPU_VIRTUAL_ADDRESS addr = r->m_instance_buffer->GetGPUVirtualAddress();
    uint32_t command_buffer_size = d_count * sizeof( indirect_command );

    HRESULT result = E_FAIL;

    result = d->m_buffer_command_allocator->Reset();
    assert( result == S_OK && "COMMAND ALLOCATOR RESET FAILED" );

    result = d->m_buffer_command_list->Reset( d->m_buffer_command_allocator.Get(), nullptr );
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

      r->m_indirect_commands[index] = tmp;

    }

    D3D12_SUBRESOURCE_DATA command_data = {};
    command_data.pData = reinterpret_cast< UINT8* >( &r->m_indirect_commands.at( 0 ) );
    command_data.RowPitch = command_buffer_size;
    command_data.SlicePitch = command_data.RowPitch;

    UpdateSubresources<1>( d->m_buffer_command_list.Get(), r->m_command_buffer.Get(),
      r->m_command_buffer_upload.Get(), 0, 0, 1, &command_data );
    d->m_buffer_command_list->ResourceBarrier( 1, &CD3DX12_RESOURCE_BARRIER::Transition( r->m_command_buffer.Get(),
      D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE ) );

    result = d->m_buffer_command_list->Close();
    assert( result == S_OK && "ERROR CLOSING THE COMMAND LIST" );
    ID3D12CommandList* ppCommandLists[] = { d->m_buffer_command_list.Get() };
    d->m_command_queue->ExecuteCommandLists( _countof( ppCommandLists ), ppCommandLists );

    //Wait for upload
    result = d->m_device->CreateFence( d->m_fence_value, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS( &d->m_fence ) );
    assert( result == S_OK && "ERROR CREATING FENCE" );
    d->m_fence_value++;

    // Create an event handle to use for frame synchronization.
    d->m_fence_event = CreateEvent( nullptr, FALSE, FALSE, nullptr );
    if( d->m_fence_event == nullptr ) {
      HRESULT_FROM_WIN32( GetLastError() );
    }

    const UINT64 fenceToWaitFor = d->m_fence_value;
    result = d->m_command_queue->Signal( d->m_fence.Get(), fenceToWaitFor );
    assert( result == S_OK && "ERROR SIGNALING FENCE" );
    d->m_fence_value++;

    // Wait until the fence is completed.
    result = d->m_fence->SetEventOnCompletion( fenceToWaitFor, d->m_fence_event );
    assert( result == S_OK && "ERROR SETTING COMPLETITION EVENT" );
    WaitForSingleObject( d->m_fence_event, INFINITE );

  }

  void populate_indirect_command_list( engine_data* e, renderer_data* r, Window* w, Drawable** draw, uint32_t d_count ) {

    e->m_render_command_list->SetPipelineState( r->m_pipeline_state.Get() );
    e->m_render_command_list->SetGraphicsRootSignature( r->m_root_signature.Get() );

    ID3D12DescriptorHeap* ppHeaps[] = { r->m_srv_heap.Get() };
    e->m_render_command_list->SetDescriptorHeaps( _countof( ppHeaps ), ppHeaps );

    e->m_render_command_list->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

    geometry_data* g_data = k_engine->get_GPU_pool()->get_geometry_data();

    e->m_render_command_list->IASetVertexBuffers( 0, 1, &g_data->m_vertexBufferView );

    e->m_render_command_list->IASetIndexBuffer( &g_data->m_indexBufferView );

    e->m_render_command_list->SetGraphicsRootConstantBufferView(
      GRP_INSTANCE_CBV, r->m_instance_buffer->GetGPUVirtualAddress() );

    e->m_render_command_list->SetGraphicsRootConstantBufferView(
      GRP_CONSTANT_CBV, k_engine->get_world()->get_buffer_data()->m_constant_buffer->GetGPUVirtualAddress() );

    CD3DX12_GPU_DESCRIPTOR_HANDLE srvHandle( r->m_srv_heap->GetGPUDescriptorHandleForHeapStart(), 0,
      r->m_cbv_srv_descriptor_size );

    e->m_render_command_list->SetGraphicsRootDescriptorTable( GRP_SRV, srvHandle );

    //Draw all
    e->m_render_command_list->ExecuteIndirect(
      r->m_command_signature.Get(),
      d_count,
      r->m_command_buffer.Get(),
      0,
      nullptr,
      0 );

  }

}

#endif