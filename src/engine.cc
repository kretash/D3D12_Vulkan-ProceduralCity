#include <cassert>
#include <time.h>

#include "core/engine_settings.hh"
#include "core/texture_manager.hh"
#include "core/city_generetaor.hh"
#include "core/vk/context.hh"
#include "core/dx/context.hh"
#include "core/drawable.hh"
#include "core/renderer.hh"
#include "core/GPU_pool.hh"
#include "core/skydome.hh"
#include "core/window.hh"
#include "core/engine.hh"
#include "core/camera.hh"
#include "core/sound.hh"
#include "core/input.hh"
#include "core/world.hh"
#include "core/factory.h"
#include "core/interface.hh"
#include "core/pool.hh"

namespace kretash {

  Engine* Engine::m_instance = nullptr;

  Engine::Engine() :
    m_window( nullptr ),
    m_camera( nullptr ),
    m_world( nullptr ),
    m_is_running( true ),
    m_city( nullptr ),
    m_drawable_id_count( 0 ) {

    for( int i = 0; i < rCOUNT; ++i )
      m_renderers[i] = nullptr;

    m_device_pool_size = ( uint64_t ) 1024 * ( uint64_t ) 512 * ( uint64_t ) 4 * ( uint64_t ) 512;
    m_host_visible_pool_size = ( uint64_t ) 1024 * ( uint64_t ) 512 * ( uint64_t ) 4 * ( uint64_t ) 64;

  }

  void Engine::init() {

    srand( ( uint32_t ) time( nullptr ) );

    m_factory = std::make_shared<Factory>();

    m_factory->make_context( &m_context );

    m_window = std::make_shared<Window>();
    m_camera = std::make_shared<Camera>();
    m_world = std::make_shared<World>();
    m_input = std::make_shared<Input>();
    m_gpu_pool = std::make_shared<GPU_pool>();
    m_texture_manager = std::make_shared<TextureManager>();
    m_interface = std::make_shared<Interface>();

    m_window->init();

    m_context->create_instance();
    m_context->create_factory();
    m_context->create_device();
    m_context->create_swap_chain( m_window.get() );
    m_context->create_command_pool();
    m_context->create_setup_command_buffer();
    m_context->setup_swap_chain( m_window.get() );
    m_context->create_buffer_command_buffer();
    m_context->create_texture_command_buffer();
    m_context->create_render_command_buffer();
    m_context->create_render_pass();
    m_context->create_depth_stencil( m_window.get() );
    m_context->create_framebuffer( m_window.get() );
    m_context->create_pipeline_cache();
    m_context->allocate_device_memory( m_device_pool_size );
    m_context->allocate_host_memory( m_host_visible_pool_size );
    m_context->create_sampler_view_heap();
    m_context->create_fences();

    m_gpu_pool->init();
    m_camera->init();
    m_interface->init();

    m_context->wait_for_setup_completion();

  }

  void Engine::prepare() {

    m_context->create_descriptor_pool();
    m_context->create_descriptor_set_layout();

    m_world->init();
    if( m_renderers[rTEXTURE] != nullptr )  m_texture_manager->prepare();

    for( int type = rBASIC; type < rCOUNT; ++type ) {
      if( m_renderers[type] != nullptr )
        m_renderers[type]->prepare();
    }

    m_sound = std::make_shared<Sound>();
    if( k_engine_settings->get_settings().play_sound )
      m_sound->play_sound( k_engine_settings->get_settings().sound_file );

  }

  void Engine::update() {
    k_engine_settings->start_frame();
    m_interface->new_frame();

    if( m_renderers[rTEXTURE] != nullptr )  m_texture_manager->update();

    m_gpu_pool->update();
    m_input->update();//0.2-0.6
    m_camera->update();
    m_world->update();

    if( m_input->get_key( key::k_F1 ) ) {
      m_input->toggle_focus();
    }

    if( m_input->get_key( e_EXIT ) ) {
      m_is_running = false;
    }

  }

  void Engine::reset_cmd_list() {
    if( m_renderers[rTEXTURE] != nullptr )  m_texture_manager->synch();
    k_engine_settings->start_render();
    m_context->reset_render_command_list( m_window.get() );
  }

  void Engine::clear_color() {
    m_context->clear_color();
  }

  void Engine::clear_depth() {
    m_context->clear_depth();
  }

  void Engine::render_skydome( Skydome* s ) {

    Renderer* r = s->get_renderer();

    m_context->record_commands( r->get_renderer(), m_window.get(),
      &( *r->get_render_bin() )[0], r->get_render_bin_size() );

  }

  void Engine::render( Renderer* r ) {

    if( r->get_render_bin_size() == 0 ) return;

    m_context->update_indirect_command_buffer( r->get_renderer(),
      &( *r->get_render_bin() )[0], r->get_render_bin_size() );

    m_context->record_indirect_commands( r->get_renderer(), m_window.get(),
      &( *r->get_render_bin() )[0], r->get_render_bin_size() );

  }

  void Engine::render_post( Renderer* r ) {

  }

  void Engine::execute_and_swap() {

    m_gpu_pool->synch();
    m_interface->render();

    m_context->execute_render_command_list();
    m_context->present_swap_chain();
    m_context->wait_render_completition();

    k_engine_settings->end_frame();

  }

  bool Engine::is_running() {

    if( !m_is_running ){
      m_texture_manager = nullptr;
    
    }

    return m_is_running;
  }

  void Engine::quit() {
    m_is_running = false;
  }

  void Engine::save_geometry( Geometry* g ) {
    m_geometries.push_back( g );
  }

  void Engine::save_city( CityGenerator* c ) {
    m_city = c;
  }

  void Engine::reload() {
    //close the command list
    m_context->execute_render_command_list();
    m_context->wait_render_completition();

    m_gpu_pool = nullptr;
    m_factory->reload();

    m_window->init();

    m_context->create_instance();
    m_context->create_factory();
    m_context->create_device();
    m_context->create_swap_chain( m_window.get() );
    m_context->create_command_pool();
    m_context->create_setup_command_buffer();
    m_context->setup_swap_chain( m_window.get() );
    m_context->create_buffer_command_buffer();
    m_context->create_texture_command_buffer();
    m_context->create_render_command_buffer();
    m_context->create_render_pass();
    m_context->create_depth_stencil( m_window.get() );
    m_context->create_framebuffer( m_window.get() );
    m_context->create_pipeline_cache();
    m_context->allocate_device_memory( m_device_pool_size );
    m_context->allocate_host_memory( m_host_visible_pool_size );
    m_context->create_sampler_view_heap();
    m_context->create_fences();

    m_gpu_pool = std::make_shared<GPU_pool>();
    m_gpu_pool->init();
    m_interface->init();

    m_context->create_descriptor_pool();
    m_context->create_descriptor_set_layout();

    m_world->init();
    m_camera->api_swap();

    for( int type = rBASIC; type < rCOUNT; ++type ) {
      if( m_renderers[type] != nullptr )
        m_renderers[type]->reload();
    }

    for( int i = 0; i < m_geometries.size(); ++i ) {
      m_geometries[i]->reload();
    }

    if( m_city != nullptr )  m_city->regenerate();

    //m_texture_manager = std::make_shared<TextureManager>();
    //if( m_renderers[rTEXTURE] != nullptr )  m_texture_manager->prepare();
    if( m_renderers[rTEXTURE] != nullptr )  m_texture_manager->regenerate();

    m_context->reset_render_command_list( m_window.get() );
  }

  void Engine::shutdown() {
    m_interface = nullptr;
    m_window = nullptr;
    m_camera = nullptr;
    m_world = nullptr;
    m_gpu_pool = nullptr;
    m_sound = nullptr;
    m_input = nullptr;
    m_factory = nullptr;
    m_context = nullptr;
    EngineSettings::shutdown();
    Engine::delele_instance();
  }

  // Engine System Getters
  xxContext*      Engine::get_context() { return m_context.get(); }
  Window*         Engine::get_window() { return m_window.get(); }
  Camera*         Engine::get_camera() { return m_camera.get(); }
  World*          Engine::get_world() { return m_world.get(); }
  GPU_pool*       Engine::get_GPU_pool() { return m_gpu_pool.get(); }
  TextureManager* Engine::get_texture_manager() { return m_texture_manager.get(); }
  Sound*          Engine::get_sound() { return m_sound.get(); }
  Input*          Engine::get_input() { return m_input.get(); }
  Factory*        Engine::get_factory() { return m_factory.get(); }
  Interface*      Engine::get_interface() { return m_interface.get(); }

  void Engine::set_renderer( Renderer* r ) {
    assert( m_renderers[r->get_renderer_type()] == nullptr && "ONLY ONE RENDERER TYPE ALLOWED" );
    m_renderers[r->get_renderer_type()] = r;
  }

  bool Engine::has_renderer( render_type t ) {
    return m_renderers[t] != nullptr;
  }

  Renderer* Engine::get_renderer( render_type t ) {
    return m_renderers[t];
  }

  int32_t Engine::new_id() {
    int32_t  id = m_drawable_id_count;
    m_drawable_id_count++;
    return id;
  }

  int32_t Engine::get_total_drawables() {
    return m_drawable_id_count;
  }

  Engine::~Engine() {

  }
}