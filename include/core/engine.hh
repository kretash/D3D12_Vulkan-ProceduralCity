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
#include "base.hh"
#include "types.hh"
#include "k_graphics.hh"

#define k_engine (Engine::get_instance())

class                             Drawable;
class                             Window;
class                             Camera;
class                             World;
class                             Renderer;
class                             Skydome;
class                             GPU_pool;
class                             TextureManager;
class                             Sound;
class                             Input;
#if __VULKAN__
class                             VKPool;
#endif

class                             Engine : public Base {
public:
  static Engine* get_instance() {
    if( m_instance == nullptr )
      m_instance = new Engine();
    return m_instance;
  }

  static void delele_instance() {
    delete m_instance;
    m_instance = nullptr;
  }

  void                            init();
  void                            prepare();
  void                            update();
  void                            shutdown();

  bool                            is_running();

  void                            reset_cmd_list();
  void                            clear_color( );
  void                            clear_depth( );  
  void                            render_skydome( Skydome* s );
  void                            render( Renderer* r );
  void                            render_post( Renderer* r );
  void                            execute_and_swap( Renderer* r );

  Window*                         get_window();
  Camera*                         get_camera();
  World*                          get_world();
  GPU_pool*                       get_GPU_pool();
  TextureManager*                 get_texture_manager();
  Sound*                          get_sound();
  Input*                          get_input();
  engine_data*                    get_engine_data();
  int32_t                         get_total_drawables();
  int32_t                         new_id();

#if __VULKAN__
  VKPool*                         get_vk_device_pool();
  VKPool*                         get_vk_host_pool();
#endif

  void                            set_renderer( Renderer* r );
  bool                            has_renderer( render_type t );
  Renderer*                       get_renderer( render_type t );

private:
  Engine();
  ~Engine();

  static Engine*                  m_instance;

  engine_data                     m_e_data;
  std::shared_ptr<Window>         m_window;
  std::shared_ptr<Camera>         m_camera;
  std::shared_ptr<World>          m_world;
  std::shared_ptr<GPU_pool>       m_gpu_pool;
  std::shared_ptr<TextureManager> m_texture_manager;
  std::shared_ptr<Sound>          m_sound;
  std::shared_ptr<Input>          m_input;
#if __VULKAN__
  std::shared_ptr<VKPool>         m_vk_device_pool;
  std::shared_ptr<VKPool>         m_vk_host_pool;
#endif
  Renderer*                       m_renderers[rCOUNT];

  bool                            m_is_running;
  int32_t                         m_drawable_id_count;
};