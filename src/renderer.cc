#include "core/engine_settings.hh"
#include "core/k_graphics.hh"
#include "core/renderer.hh"
#include "core/drawable.hh"
#include "core/texture.hh"
#include "core/window.hh"
#include "core/camera.hh"
#include "core/tools.hh"
#include "core/world.hh"
#include "core/texture_manager.hh"
#include <cassert>
#include <algorithm>

Renderer::Renderer() {
  m_render_bin_objects = 0;
  m_cbv_srv_offset = 0;
  m_render_manager = new RenderManager();
}

void Renderer::create( render_type t ) {

  m_render_type = t;
  k_engine->set_renderer( this );

}

void Renderer::add_child( Drawable* d ) {

  d->prepare();

  k_engine->get_texture_manager()->link_drawable( d );
  m_render_manager->add_child( d );

  m_render_bin_objects++;
}

void Renderer::prepare() {

  if( m_render_type != rPOST ) {
    std::vector<Drawable*>* rb = m_render_manager->get_render_bin();

    m_instance_buffer.resize( k_engine->get_total_drawables() );
    Camera* c = k_engine->get_camera();
    float4x4 view = c->get_view();
    float4x4 proj = c->get_projection();

    GPU::create_instance_buffer_object( k_engine->get_engine_data(), &r_data, &m_instance_buffer[0] );

    uint32_t w = sizeof( instance_buffer );
    const uint32_t buffer_size = sizeof( instance_buffer ) + 255 & ~255;
    assert( w == buffer_size && "THIS IS MADNESS" );

    for( int32_t i = 0; i < m_render_bin_objects; ++i ) {

      int32_t id = ( *rb )[i]->get_drawable_id();

      m_instance_buffer[id].mvp = ( *rb )[i]->get_model() * view * proj;
      m_instance_buffer[id].normal_matrix = float4x4::inverse( ( *rb )[i]->get_model() );

      m_instance_buffer[id].d_texture_id = ( *rb )[i]->get_texture()->get_id( tDIFFUSE );
      m_instance_buffer[id].n_texture_id = ( *rb )[i]->get_texture()->get_id( tNORMAL );
      m_instance_buffer[id].s_texture_id = ( *rb )[i]->get_texture()->get_id( tSPECULAR );

#if __VULKAN__ //Temporary
      ( *rb )[i]->get_drawable_data()->m_uniform_buffer = m_instance_buffer[id];
#endif

      GPU::create_instance_buffer_view( k_engine->get_engine_data(), &r_data, (*rb)[i]->get_drawable_data(), 
        ( *rb )[i]->get_drawable_id()*buffer_size, 0 );
    }

    GPU::create_and_update_descriptor_sets( k_engine->get_engine_data(), &r_data, rb );

    if( m_render_type != rSKYDOME )
      GPU::create_and_fill_command_buffer( k_engine->get_engine_data(), &r_data, &( *rb )[0], m_render_bin_objects );
  } else {

  }


  if( m_render_type == rBASIC || m_render_type == rSKYDOME || m_render_type == rTEXTURE ) {
    GPU::create_root_signature( &r_data );

#ifdef __VULKAN__
    GPU::create_srv_view_heap( k_engine->get_engine_data(), &r_data, 2 );
    GPU::create_graphics_pipeline( k_engine->get_engine_data(), &r_data, m_render_type );
#elif __DIRECTX12__
    GPU::load_and_compile_shaders( &r_data, m_render_type );
    GPU::create_pipeline_state_object( &r_data );
#endif

  } else if( m_render_type == rPOST ) {
    GPU::create_srv_view_heap( k_engine->get_engine_data(), &r_data, 2 );
    GPU::add_post_textures_to_srv( k_engine->get_engine_data(), &r_data );
    GPU::create_post_root_signature( &r_data );

#ifdef __VULKAN__
    GPU::create_graphics_pipeline( k_engine->get_engine_data(), &r_data, m_render_type );
#elif __DIRECTX12__
    GPU::load_and_compile_shaders( &r_data, m_render_type );
    GPU::create_pipeline_state_object( &r_data );
#endif

    GPU::create_command_signature( k_engine->get_engine_data(), &r_data );
  }
}

void Renderer::update() {
  if( m_render_type != rPOST ) {

    m_render_manager->update( 0.016f );

    std::vector<Drawable*>* rb = m_render_manager->get_active_render_bin();
    Camera* c = k_engine->get_camera();
    float4x4 view = c->get_view();
    float4x4 proj = c->get_projection();


    auto comp = [] ( Drawable* a, Drawable* b ) {
      return a->get_drawable_id() < b->get_drawable_id();
    };

    //Quicker to sort the rb to remove cache misses
    std::sort( rb->begin(), rb->end(), comp );

    for( int32_t i = 0; i < rb->size(); ++i ) {
      int32_t id = ( *rb )[i]->get_drawable_id();

      float4x4 model = ( *rb )[i]->get_model();
      m_instance_buffer[id].mvp = model * view * proj;
      m_instance_buffer[id].model = model;

#if __DIRECTX12__
      m_instance_buffer[id].mvp.transpose();
#elif __VULKAN__
      m_instance_buffer[id].model.transpose();
#endif

      m_instance_buffer[id].normal_matrix = float4x4::inverse( model );

      m_instance_buffer[id].d_texture_id = ( *rb )[i]->get_texture()->get_id( tDIFFUSE );
      m_instance_buffer[id].n_texture_id = ( *rb )[i]->get_texture()->get_id( tNORMAL );
      m_instance_buffer[id].s_texture_id = ( *rb )[i]->get_texture()->get_id( tSPECULAR );

#if __VULKAN__ //Temporary
      ( *rb )[i]->get_drawable_data()->m_uniform_buffer = m_instance_buffer[id];
#endif
    }

    GPU::update_instance_buffer_object( k_engine->get_engine_data(), &r_data, &m_instance_buffer[0] );//~0.5ms
    GPU::update_decriptor_sets( k_engine->get_engine_data(), &r_data, rb );
  }
}

Renderer::~Renderer() {
  delete m_render_manager;
  m_render_manager = nullptr;
}
