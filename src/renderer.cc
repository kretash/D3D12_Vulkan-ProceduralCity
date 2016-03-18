#include "core/engine_settings.hh"
#include "core/renderer.hh"
#include "core/drawable.hh"
#include "core/texture.hh"
#include "core/window.hh"
#include "core/camera.hh"
#include "core/tools.hh"
#include "core/world.hh"
#include "core/xx/renderer.hh"
#include "core/xx/context.hh"
#include "core/factory.h"
#include "core/texture_manager.hh"
#include <cassert>
#include <algorithm>

namespace kretash {

  Renderer::Renderer() {
    m_render_bin_objects = 0;
    m_cbv_srv_offset = 0;
    m_render_manager = std::make_shared<RenderManager>();

  }

  void Renderer::create( render_type t ) {

    Factory* factory = k_engine->get_factory();
    factory->make_renderer( &m_renderer );

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

    assert( m_render_type != rPOST && "TIME TO FIX THIS" );
    if( m_render_bin_objects == 0 ) return;

    xxContext* m_context = k_engine->get_context();
    std::vector<Drawable*>* rb = m_render_manager->get_render_bin();

    m_instance_buffer.resize( k_engine->get_total_drawables() );
    Camera* c = k_engine->get_camera();
    float4x4 view = c->get_view();
    float4x4 proj = c->get_projection();

    m_renderer->create_instance_buffer_objects( rb );

    uint32_t w = sizeof( instance_buffer );
    const uint32_t buffer_size = sizeof( instance_buffer ) + 255 & ~255;
    assert( w == buffer_size && "THIS IS MADNESS" );

    bool vulkan = k_engine_settings->get_settings().m_api == kVulkan;

    for( int32_t i = 0; i < m_render_bin_objects; ++i ) {

      int32_t id = ( *rb )[i]->get_drawable_id();

      m_instance_buffer[id].mvp = ( *rb )[i]->get_model() * view * proj;

      m_instance_buffer[id].normal_matrix = float4x4::inverse( ( *rb )[i]->get_model() );
      if( vulkan ) m_instance_buffer[id].normal_matrix.transpose();

      m_instance_buffer[id].d_texture_id = ( *rb )[i]->get_texture()->get_id( tDIFFUSE );
      m_instance_buffer[id].n_texture_id = ( *rb )[i]->get_texture()->get_id( tNORMAL );
      m_instance_buffer[id].s_texture_id = ( *rb )[i]->get_texture()->get_id( tSPECULAR );
      ( *rb )[i]->set_instance_buffer( &m_instance_buffer[id] );

    }

    m_renderer->create_root_signature();
    m_renderer->create_graphics_pipeline( m_render_type );

    if( m_render_type != rSKYDOME ) {
      m_context->create_indirect_command_signature( m_renderer.get() );
      m_context->create_indirect_command_buffer( m_renderer.get(), &( *rb )[0], m_render_bin_objects );
    }
  }

  void Renderer::reload(){
    prepare();
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

      bool vulkan = k_engine_settings->get_settings().m_api == kVulkan;

      for( int32_t i = 0; i < rb->size(); ++i ) {
        int32_t id = ( *rb )[i]->get_drawable_id();

        float4x4 model = ( *rb )[i]->get_model();
        m_instance_buffer[id].mvp = model * view * proj;
        m_instance_buffer[id].model = model;

        if( vulkan ) m_instance_buffer[id].model.transpose();
        else m_instance_buffer[id].mvp.transpose();

        m_instance_buffer[id].normal_matrix = float4x4::inverse( model );
        if( vulkan ) m_instance_buffer[id].normal_matrix.transpose();

        m_instance_buffer[id].d_texture_id = ( *rb )[i]->get_texture()->get_id( tDIFFUSE );
        m_instance_buffer[id].n_texture_id = ( *rb )[i]->get_texture()->get_id( tNORMAL );
        m_instance_buffer[id].s_texture_id = ( *rb )[i]->get_texture()->get_id( tSPECULAR );

        ( *rb )[i]->set_instance_buffer( &m_instance_buffer[id] );

      }

      m_renderer->update_instance_buffer_objects( rb, &m_instance_buffer );

      //GPU::update_instance_buffer_object( k_engine->get_engine_data(), &r_data, &m_instance_buffer[0] );//~0.5ms
      //GPU::update_decriptor_sets( k_engine->get_engine_data(), &r_data, rb );
    }
  }

  Renderer::~Renderer() {
    m_render_manager = nullptr;
  }
}