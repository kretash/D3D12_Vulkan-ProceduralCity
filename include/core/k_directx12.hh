/*
----------------------------------------------------------------------------------------------------
------                  _   _____ _  __                     ------------ /_/\  ---------------------
------              |/ |_) |_  | |_|(_ |_|                  ----------- / /\ \  --------------------
------              |\ | \ |__ | | |__)| |                  ---------- / / /\ \  -------------------
------   CARLOS MARTINEZ ROMERO - kretash.wordpress.com     --------- / / /\ \ \  ------------------
------                                                      -------- / /_/__\ \ \  -----------------
------       PROCEDURAL CITY RENDERING WITH THE NEW         ------  /_/______\_\/\  ----------------
------            GENERATION GRAPHICS APIS                  ------- \_\_________\/ -----------------
----------------------------------------------------------------------------------------------------

Licensed under the MIT License (the "License"); you may not use this file except
in compliance with the License. You may obtain a copy of the License at
http://opensource.org/licenses/MIT
*/

#pragma once

#include "core.hh"

#ifdef __DIRECTX12__

#include "d3dx12.h"
#include <d3d12.h>
#include <dxgi1_4.h>
#include <d3dcompiler.h>

#include <wrl.h>
#include <string>
#include <vector>

#include "types.hh"
#include "math/float4x4.hh"
#include "tools.hh"

class Window;
class Drawable;

#define m_ptr Microsoft::WRL::ComPtr

//variables of shame
namespace dx {
  static const int                        m_stride = 14;
  static const UINT                       m_frame_count = 2;
  static UINT                             p_pipeline_state_object_id_counter = 0;
};

struct engine_data {
#ifdef _DEBUG
  m_ptr<ID3D12Debug>                      m_debug_controller;
#endif
  m_ptr<IDXGIFactory4>                    factory;
  m_ptr<IDXGISwapChain3>                  m_swap_chain;
  m_ptr<ID3D12Device>                     m_device;
  m_ptr<ID3D12CommandQueue>               m_command_queue;

  m_ptr<ID3D12CommandAllocator>           m_buffer_command_allocator;
  m_ptr<ID3D12GraphicsCommandList>        m_buffer_command_list;

  m_ptr<ID3D12CommandQueue>               m_texture_command_queue;
  m_ptr<ID3D12CommandAllocator>           m_texture_command_allocator;
  m_ptr<ID3D12GraphicsCommandList>        m_texture_command_list;

  m_ptr<ID3D12CommandAllocator>           m_render_command_allocator;
  m_ptr<ID3D12GraphicsCommandList>        m_render_command_list;

  m_ptr<ID3D12Resource>                   m_render_targets[dx::m_frame_count];
  m_ptr<ID3D12Resource>                   m_msaa_render_target;
  m_ptr<ID3D12Resource>                   m_post_render_target;
  m_ptr<ID3D12Resource>                   m_depth_stencil;
  uint32_t                                m_rtv_descriptor_size;
  m_ptr<ID3D12DescriptorHeap>             m_rtv_heap;
  m_ptr<ID3D12DescriptorHeap>             m_dsv_heap;

  int32_t                                 m_frame_index;
  HANDLE                                  m_fence_event;
  m_ptr<ID3D12Fence>                      m_fence;
  uint64_t                                m_fence_value;

  bool                                    m_fence_texture_upload_pending;
  uint64_t                                m_fence_texture_upload;

  bool                                    m_msaa_enabled;
  int32_t                                 m_mssa_count;

  engine_data() :
    m_frame_index( 0 ),
    m_fence_event( 0 ),
    m_fence_texture_upload_pending( false ),
    m_fence_texture_upload( 0 ),
    m_msaa_enabled( false ),
    m_mssa_count(0){
  }
  ~engine_data() {
#ifdef _DEBUG
    m_debug_controller = nullptr;
#endif
    factory = nullptr;
    m_swap_chain = nullptr;
    m_device = nullptr;
    m_command_queue = nullptr;
    m_buffer_command_allocator = nullptr;
    m_buffer_command_list = nullptr;

    m_texture_command_queue = nullptr;
    m_texture_command_allocator = nullptr;
    m_texture_command_list = nullptr;
    m_render_command_allocator = nullptr;
    m_render_command_list = nullptr;

    for( int i = 0; i < dx::m_frame_count; ++i )
      m_render_targets[i] = nullptr;
    m_depth_stencil = nullptr;
    m_rtv_heap = nullptr;
    m_dsv_heap = nullptr;

    m_fence = nullptr;
  }
};

struct indirect_command {
  D3D12_GPU_VIRTUAL_ADDRESS               cbv;
  D3D12_DRAW_INDEXED_ARGUMENTS            draw_arguments;
};

struct renderer_data {
  m_ptr<ID3D12CommandSignature>           m_command_signature;
  std::vector<indirect_command>           m_indirect_commands;
  m_ptr<ID3D12Resource>                   m_command_buffer;
  m_ptr<ID3D12Resource>                   m_command_buffer_upload;

  m_ptr<ID3D12DescriptorHeap>             m_cbv_heap;
  m_ptr<ID3D12DescriptorHeap>             m_srv_heap;
  uint32_t                                m_cbv_srv_descriptor_size;

  m_ptr<ID3DBlob>                         vertexShader;
  m_ptr<ID3DBlob>                         pixelShader;
  m_ptr<ID3D12RootSignature>              m_root_signature;
  m_ptr<ID3D12PipelineState>              m_pipeline_state;
  uint32_t                                m_pipeline_state_id;

  m_ptr<ID3D12Resource>                   m_instance_buffer;
  uint8_t*                                m_instance_buffer_WO;
  D3D12_CONSTANT_BUFFER_VIEW_DESC         m_instance_buffer_desc;

  m_ptr<ID3D12Resource>                   m_constant_buffer;
  uint8_t*                                m_constant_buffer_WO;
  D3D12_CONSTANT_BUFFER_VIEW_DESC         m_constant_buffer_desc;

  renderer_data() :
    m_pipeline_state_id( 0 ),
    m_cbv_srv_descriptor_size( 0 ) {
  }
  ~renderer_data() {

    m_indirect_commands.clear();
    m_indirect_commands.shrink_to_fit();

    m_command_signature = nullptr;
    m_command_buffer = nullptr;
    m_command_buffer_upload = nullptr;
    m_cbv_heap = nullptr;
    m_srv_heap = nullptr;
    vertexShader = nullptr;
    pixelShader = nullptr;
    m_root_signature = nullptr;
    m_pipeline_state = nullptr;
    m_instance_buffer = nullptr;
    m_constant_buffer = nullptr;

  }
};

struct geometry_data {

  m_ptr<ID3D12Resource>                   m_vertexBuffer;
  D3D12_VERTEX_BUFFER_VIEW                m_vertexBufferView;
  m_ptr<ID3D12Resource>                   m_indexBuffer;
  D3D12_INDEX_BUFFER_VIEW                 m_indexBufferView;

  void release_resrources() {
    m_vertexBuffer = nullptr;
    m_indexBuffer = nullptr;
  }
  geometry_data() {}
  ~geometry_data() {
    m_vertexBuffer = nullptr;
    m_indexBuffer = nullptr;
  }
};

struct texture_data {

  m_ptr<ID3D12Resource>                   m_texture;
  m_ptr<ID3D12Resource>                   m_texture_upload;

  texture_data() {}
  ~texture_data() {
    m_texture = nullptr;
    m_texture_upload = nullptr;
  }
};

struct drawable_data {

  int                                     m_geo_lod;
  m_ptr<ID3D12Resource>                   m_constant_buffer;
  UINT8*                                  m_constant_buffer_WO;
  D3D12_CONSTANT_BUFFER_VIEW_DESC         m_constant_buffer_desc;

  drawable_data() {}
  ~drawable_data() {
    m_constant_buffer = nullptr;
  }
};

struct constant_buffer_data {
  m_ptr<ID3D12Resource>                   m_constant_buffer;
  UINT8*                                  m_constant_buffer_WO;
  D3D12_CONSTANT_BUFFER_VIEW_DESC         m_constant_buffer_desc;
};

struct constant_buffer {
  float3                                  light_pos;
  float                                   sky_color;
  float3                                  fog_color;
  float                                   ambient_intensity;
  float3                                  sun_light_intensity;
  float                                   padding;
  float3                                  eye_view;
  float4x4                                view;

  constant_buffer() {}
  ~constant_buffer() {}
};

struct instance_buffer {
  float4x4                                mvp;
  float4x4                                model;
  float4x4                                normal_matrix;

  uint32_t                                d_texture_id;
  uint32_t                                n_texture_id;
  uint32_t                                s_texture_id;

  uint32_t                                pad[13];

  instance_buffer() :
    d_texture_id( 0 ),
    n_texture_id( 0 ),
    s_texture_id( 0 ) {
  }
};

namespace dx {

  enum GraphicsRootsParameters {
    GRP_INSTANCE_CBV = 0,
    GRP_SRV,
    GRP_CONSTANT_CBV,
    GRP_SAMPLER,
    GRP_COUNT,
  };
  //pipeline
#ifdef _DEBUG
  void enable_debug_layer( engine_data* d );
#endif
  void create_device( engine_data* d );
  void create_factory( engine_data* d );
  void create_command_queue( engine_data* d );
  void create_swap_chain( engine_data* d, Window* w );
  void create_command_allocator( engine_data* d );
  void create_command_list( engine_data* d );
  void create_fences( engine_data* d );
  void wait_for_previous_frame( engine_data* d );
  void create_depth_stencil_view_heap( engine_data* e );
  void create_render_target_view_heap( engine_data* e );
  void create_depth_stencil_view( engine_data* e, Window* w );
  void create_render_target_view( engine_data* e, Window* w );

  //buffer
  void create_empty_vertex_buffer( engine_data* e, geometry_data* g, uint32_t size );
  void create_empty_index_buffer( engine_data* e, geometry_data* g, uint32_t size );
  void upload_into_vertex_buffer( engine_data* e, geometry_data* g, uint32_t offset, float* array_data, uint32_t size );
  void upload_queue_into_vertex_buffer( engine_data* e, geometry_data* g, std::vector<queue>* queue );
  void upload_into_index_buffer( engine_data* e, geometry_data* g, uint32_t offset, uint32_t* elements_data, uint32_t size );
  void upload_queue_into_index_buffer( engine_data* e, geometry_data* g, std::vector<queue>* queue );

  //renderer
  void create_srv_view_heap( engine_data* e, renderer_data* r, int32_t size );
  void create_root_signature( renderer_data* r );
  void load_and_compile_shaders( renderer_data* r, render_type s );
  void create_pipeline_state_object( renderer_data* r );
  void show_shader_error_message( ID3D10Blob* errorMessage, std::wstring filename );

  //post - renderer
  void create_post_root_signature( renderer_data* r );
  void create_post_pipeline_state_object( renderer_data* r );
  void add_post_textures_to_srv( engine_data* e, renderer_data* r );
  void post_render( engine_data* e, renderer_data* r, Window* w );

  //texture
  void reset_texture_command_list( engine_data* d );
  void create_texture( engine_data* d, texture_data* t, void* data, int32_t width, int32_t height, int32_t channels );
  void create_shader_resource_view( renderer_data* r, texture_data* t, int32_t offset );
  void compute_texture_upload( engine_data* d );
  void wait_for_texture_upload( engine_data* d );
  void clear_texture_upload( texture_data* t );
  void clear_texture( texture_data* t );

  //drawable
  void init_descriptor_pool_and_layout( engine_data* e );
  void create_constant_buffer_object( engine_data* e, constant_buffer_data* cbd, constant_buffer cb );
  void update_constant_buffer_object( engine_data* e, constant_buffer_data* cbd, constant_buffer cb );
  void create_instance_buffer_object( engine_data* e, renderer_data* r, instance_buffer* ub );
  void create_instance_buffer_view( engine_data* e, renderer_data* r, drawable_data* d, uint64_t buffer_offset, int32_t cbv_offset );
  void update_instance_buffer_object( engine_data* e, renderer_data* r, instance_buffer* ub );
  void create_and_update_descriptor_sets( engine_data* e, renderer_data* r, std::vector<Drawable*>* draw );
  void update_decriptor_sets( engine_data* e, renderer_data* r, std::vector<Drawable*>* draw );

  //render
  void reset_render_command_list( engine_data* e, Window* w );
  void clear_color( engine_data* e, Window* w );
  void clear_depth( engine_data* e, Window* w );

  void populate_command_list( engine_data* d, renderer_data* r, Window* w, Drawable** draw, uint32_t d_count );
  void execute_command_lists( engine_data* d, renderer_data* r );
  void present_swap_chain( engine_data* d );


  //execute indirect
  void create_command_signature( engine_data* d, renderer_data* r );
  void create_and_fill_command_buffer( engine_data* d, renderer_data* r, Drawable** draw, uint32_t d_count );
  void update_command_buffer( engine_data* d, renderer_data* r, Drawable** draw, uint32_t d_count );
  void populate_indirect_command_list( engine_data* e, renderer_data* r, Window* w, Drawable** draw, uint32_t d_count );

}

#endif