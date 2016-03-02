#include "core/k_directx12.hh"

#ifdef __DIRECTX12__
#include "core/engine_settings.hh"
#include "core/drawable.hh"
#include "core/geometry.hh"
#include "core/GPU_pool.hh"
#include "core/window.hh"
#include "core/world.hh"
#include <cassert>

namespace dx {

  void reset_render_command_list( engine_data* e, Window* w ) {
    HRESULT result;

    result = e->m_render_command_allocator->Reset();
    assert( result == S_OK && "COMMAND ALLOCATOR RESET FAILED" );

    result = e->m_render_command_list->Reset( e->m_render_command_allocator.Get(), nullptr );
    assert( result == S_OK && "COMMAND LIST RESET FAILED" );

    e->m_render_command_list->RSSetScissorRects( 1, &w->get_scissor() );
    e->m_render_command_list->RSSetViewports( 1, &w->get_viewport() );
  }

  void clear_color( engine_data* e, Window* w ) {
    
    std::vector<D3D12_RESOURCE_BARRIER> barriers;
    barriers.push_back(
      CD3DX12_RESOURCE_BARRIER::Transition( e->m_render_targets[e->m_frame_index].Get(),
        D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET ) );
    e->m_render_command_list->ResourceBarrier( barriers.size(), barriers.data() );


    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle( e->m_rtv_heap->GetCPUDescriptorHandleForHeapStart(),
      e->m_frame_index, e->m_rtv_descriptor_size );
    CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle( e->m_dsv_heap->GetCPUDescriptorHandleForHeapStart() );

    //RENDER TO TEXTURE
    e->m_render_command_list->OMSetRenderTargets( 1, &rtvHandle, FALSE, &dsvHandle );

    const float clear_color[] = { 0.529f, 0.8f, 0.98f, 1.0f };
    e->m_render_command_list->ClearRenderTargetView( rtvHandle, clear_color, 0, nullptr );

  }

  void clear_depth( engine_data* e, Window* w ) {

    e->m_render_command_list->ClearDepthStencilView( e->m_dsv_heap->GetCPUDescriptorHandleForHeapStart(),
      D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr );
  }

  void populate_command_list( engine_data* e, renderer_data* r, Window* w, Drawable** draw, uint32_t d_count ) {

    uint32_t count = 0;
    e->m_render_command_list->SetPipelineState( r->m_pipeline_state.Get() );
    e->m_render_command_list->SetGraphicsRootSignature( r->m_root_signature.Get() );
    e->m_render_command_list->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

    if( r->m_srv_heap != nullptr ) {

      ID3D12DescriptorHeap* ppHeaps[] = { r->m_srv_heap.Get() };
      e->m_render_command_list->SetDescriptorHeaps( _countof( ppHeaps ), ppHeaps );

      CD3DX12_GPU_DESCRIPTOR_HANDLE srvHandle( r->m_srv_heap->GetGPUDescriptorHandleForHeapStart(), 0,
        r->m_cbv_srv_descriptor_size );

      e->m_render_command_list->SetGraphicsRootDescriptorTable( GRP_SRV, srvHandle );
    }

    e->m_render_command_list->SetGraphicsRootConstantBufferView(
      GRP_CONSTANT_CBV, k_engine->get_world()->get_buffer_data()->m_constant_buffer->GetGPUVirtualAddress() );

    for( count; count < d_count; ++count ) {

      D3D12_GPU_VIRTUAL_ADDRESS addr = r->m_instance_buffer->GetGPUVirtualAddress();
      int id = draw[count]->get_drawable_id();

      e->m_render_command_list->SetGraphicsRootConstantBufferView(
        GRP_INSTANCE_CBV, addr + ( id * sizeof( instance_buffer ) ) );

      geometry_data* g_data = k_engine->get_GPU_pool()->get_geometry_data();
      e->m_render_command_list->IASetVertexBuffers( 0, 1, &g_data->m_vertexBufferView );
      e->m_render_command_list->IASetIndexBuffer( &g_data->m_indexBufferView );

      e->m_render_command_list->DrawIndexedInstanced( draw[count]->get_geometry()->get_indicies_count(), 1,
        draw[count]->get_geometry()->get_indicies_offset(),
        draw[count]->get_geometry()->get_vertex_offset() / m_stride, 0 );
    }
  }

  void execute_command_lists( engine_data* e, renderer_data* r ) {

    std::vector<D3D12_RESOURCE_BARRIER> barriers;
    //RENDER TO TEXTURE
    if( false ) {
      barriers.push_back(
        CD3DX12_RESOURCE_BARRIER::Transition( e->m_render_targets[e->m_frame_index].Get(),
          D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT ) );

      barriers.push_back(
        CD3DX12_RESOURCE_BARRIER::Transition( e->m_msaa_render_target.Get(),
          D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET ) );

    } else {
      barriers.push_back(
        CD3DX12_RESOURCE_BARRIER::Transition( e->m_render_targets[e->m_frame_index].Get(),
          D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT ) );
    }

    e->m_render_command_list->ResourceBarrier( barriers.size(), barriers.data() );

    HRESULT result = e->m_render_command_list->Close();
    assert( result == S_OK && "CLOSING THE COMMAND LIST FAILED" );

    ID3D12CommandList* command_lists[] = { e->m_render_command_list.Get() };
    e->m_command_queue->ExecuteCommandLists( _countof( command_lists ), command_lists );
  }

  void present_swap_chain( engine_data* d ) {
    HRESULT result = d->m_swap_chain->Present( 0, 0 );
    assert( result == S_OK && "PRESENTING THE SWAP CHAIN FAILED" );
    d->m_frame_index = d->m_swap_chain->GetCurrentBackBufferIndex();
  }
}

#endif