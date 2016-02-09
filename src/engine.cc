#include <cassert>
#include <time.h>

#include "core/engine_settings.hh"
#include "core/texture_manager.hh"
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

Engine* Engine::m_instance = nullptr;

Engine::Engine() :
  m_window( nullptr ),
  m_camera( nullptr ),
  m_world( nullptr ),
  m_is_running( true ),
  m_drawable_id_count( 0 ) {

  for( int i = 0; i < rCOUNT; ++i )
    m_renderers[i] = nullptr;

}

void Engine::init() {

  srand( ( uint32_t ) time( nullptr ) );

#ifdef _DEBUG
  GPU::enable_debug_layer( &m_e_data );
#endif

  m_window = std::make_shared<Window>();
  m_window->init();

  GPU::create_device( &m_e_data );
  GPU::create_factory( &m_e_data );
  GPU::create_command_queue( &m_e_data );
  GPU::create_swap_chain( &m_e_data, m_window.get() );
  GPU::create_command_allocator( &m_e_data );
  GPU::create_command_list( &m_e_data );

  m_texture_manager = std::make_shared<TextureManager>();
  m_gpu_pool = std::make_shared<GPU_pool>();
  m_camera = std::make_shared<Camera>();
  m_world = std::make_shared<World>();
  m_input = std::make_shared<Input>();

  m_gpu_pool->init();
  m_camera->init();
  m_world->init();

  GPU::create_render_target_view_heap( &m_e_data );
  GPU::create_render_target_view( &m_e_data, k_engine->get_window() );
  GPU::create_depth_stencil_view_heap( &m_e_data );
  GPU::create_depth_stencil_view( &m_e_data, k_engine->get_window() );

  GPU::create_fences( &m_e_data );
  GPU::wait_for_previous_frame( &m_e_data );
}

void Engine::prepare() {

  for( int type = rBASIC; type < rCOUNT; ++type ) {
    if( m_renderers[type] != nullptr )
      m_renderers[type]->prepare();
  }

  m_sound = std::make_shared<Sound>();
  if( k_engine_settings->get_settings().play_sound )
    m_sound->play_sound( k_engine_settings->get_settings().sound_file );

  m_texture_manager->prepare();

}

void Engine::update() {

  m_texture_manager->update();
  m_input->update();
  m_gpu_pool->update();
  m_camera->update();
  m_world->update();

  if( m_input->get_key( k_SPACE ) ) {
    m_input->toggle_focus();
  }

  if( m_input->get_key( e_EXIT ) ) {
    m_is_running = false;
  }

}

void Engine::reset_cmd_list() {
  GPU::reset_render_command_list( &m_e_data, m_window.get() );
}

void Engine::clear_color(  ) {
  GPU::clear_rtv( &m_e_data, m_window.get() );
}

void Engine::clear_depth(  ) {
  GPU::clear_dsv( &m_e_data, m_window.get() );
}

void Engine::render_skydome( Skydome* s ) {

  Renderer* r = s->get_renderer();

  GPU::populate_command_list( &m_e_data, r->get_render_data(), m_window.get(),
    r->get_render_bin(), r->get_render_bin_size() );

}

void Engine::render( Renderer* r ) {

  int32_t render_bin_size = r->get_render_bin_size();

  m_texture_manager->synch();

  if( render_bin_size > 0 ) {

    GPU::update_command_buffer( &m_e_data, r->get_render_data(),
      r->get_render_bin(), render_bin_size );

    GPU::populate_indirect_command_list( &m_e_data, r->get_render_data(), m_window.get(),
      r->get_render_bin(), render_bin_size );

  }
}

void Engine::render_post( Renderer* r ) {

    GPU::post_render( &m_e_data, r->get_render_data(), m_window.get() );

}

void Engine::execute_and_swap( Renderer* r ) {

  GPU::execute_command_lists( &m_e_data, r->get_render_data() );
  GPU::present_swap_chain( &m_e_data );
  GPU::wait_for_previous_frame( &m_e_data );

}

bool Engine::is_running() {
  if( !m_is_running ) {
    shutdown();
    return false;
  }
  return m_is_running;
}

void Engine::shutdown(){
    EngineSettings::shutdown();
    Engine::delele_instance();
}

// Engine System Getters
Window*         Engine::get_window() { return m_window.get(); }
Camera*         Engine::get_camera() { return m_camera.get(); }
World*          Engine::get_world() { return m_world.get(); }
GPU_pool*       Engine::get_GPU_pool() { return m_gpu_pool.get(); }
TextureManager* Engine::get_texture_manager() { return m_texture_manager.get(); }
Sound*          Engine::get_sound() { return m_sound.get(); }
Input*          Engine::get_input() { return m_input.get(); }


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

engine_data* Engine::get_engine_data() {
  return &m_e_data;
}
Engine::~Engine() {}
