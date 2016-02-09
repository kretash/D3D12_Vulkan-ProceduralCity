#include "core/k_directx12.hh"

#ifdef __DIRECTX12__

#include "core/drawable.hh"
#include "core/geometry.hh"
#include "core/GPU_pool.hh"
#include "core/window.hh"
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

  void clear_rtv( engine_data* e, Window* w ) {

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle( e->m_rtv_heap->GetCPUDescriptorHandleForHeapStart(),
      dx::m_frame_count, e->m_rtv_descriptor_size );
    CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle( e->m_dsv_heap->GetCPUDescriptorHandleForHeapStart() );
    e->m_render_command_list->OMSetRenderTargets( 1, &rtvHandle, FALSE, &dsvHandle );

    const float clear_color[] = { 0.529f, 0.8f, 0.98f, 1.0f };
    e->m_render_command_list->ClearRenderTargetView( rtvHandle, clear_color, 0, nullptr );

  }

  void clear_dsv( engine_data* e,  Window* w ) {

    e->m_render_command_list->ClearDepthStencilView( e->m_dsv_heap->GetCPUDescriptorHandleForHeapStart(),
      D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr );
  }

  void populate_command_list( engine_data* e, renderer_data* r, Window* w, Drawable** draw, uint32_t d_count ) {

    uint32_t count = 0;
    int32_t current_srv_set = -1;
    e->m_render_command_list->SetPipelineState( r->m_pipeline_state.Get() );
    e->m_render_command_list->SetGraphicsRootSignature( r->m_root_signature.Get() );

    //e->m_render_command_list->SetDescriptorHeaps( 1, (ID3D12DescriptorHeap* const* )r->m_cbv_heap.Get() );
    //if( &(r->m_srv_heap) != nullptr )
    //  e->m_render_command_list->SetDescriptorHeaps( 1, ( ID3D12DescriptorHeap* const* ) r->m_srv_heap.Get() );

    e->m_render_command_list->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

    for( count; count < d_count; ++count ) {

      e->m_render_command_list->SetGraphicsRootConstantBufferView( 0, 
        r->m_instance_buffer->GetGPUVirtualAddress() + ( draw[count]->get_drawable_id() * sizeof( instance_buffer ) ) );

      geometry_data* g_data = k_engine->get_GPU_pool()->get_geometry_data();

      e->m_render_command_list->IASetVertexBuffers( 0, 1, &g_data->m_vertexBufferView );

      e->m_render_command_list->IASetIndexBuffer( &g_data->m_indexBufferView );

      e->m_render_command_list->DrawIndexedInstanced( draw[count]->get_geometry()->get_indicies_count(), 1,
        draw[count]->get_geometry()->get_indicies_offset(),
        draw[count]->get_geometry()->get_vertex_offset() / m_stride, 0 );
      }
  }
  
    void execute_command_lists( engine_data* e, renderer_data* r ) {
      D3D12_RESOURCE_BARRIER barriers[] = {
        CD3DX12_RESOURCE_BARRIER::Transition( e->m_render_targets[e->m_frame_index].Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT ),
        CD3DX12_RESOURCE_BARRIER::Transition( e->m_msaa_render_target.Get(),
          D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET )
      };
      e->m_render_command_list->ResourceBarrier( _countof( barriers ), barriers );

      HRESULT result = e->m_render_command_list->Close();
      assert( result == S_OK && "CLOSING THE COMMAND LIST FAILED" );

      ID3D12CommandList* command_lists[] = { e->m_render_command_list.Get() };
      e->m_command_queue->ExecuteCommandLists( _countof( command_lists ), command_lists );
    }

    void present_swap_chain( engine_data* d ) {
      HRESULT result = d->m_swap_chain->Present( 1, 0 );
      assert( result == S_OK && "PRESENTING THE SWAP CHAIN FAILED" );
    }
}

#endif