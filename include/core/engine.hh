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
#include <memory>
#include <vector>

#define k_engine (kretash::Engine::get_instance())

namespace kretash {

  class                             Drawable;
  class                             Geometry;
  class                             Window;
  class                             Camera;
  class                             World;
  class                             Renderer;
  class                             Skydome;
  class                             GPU_pool;
  class                             TextureManager;
  class                             Sound;
  class                             Input;
  class                             Interface;
  class                             Factory;
  class                             CityGenerator;
  class                             xxContext;

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
    void                            reload();
    void                            quit();

    void                            reset_cmd_list();
    void                            clear_color();
    void                            clear_depth();
    void                            render_skydome( Skydome* s );
    void                            render( Renderer* r );
    void                            render_post( Renderer* r );
    void                            execute_and_swap();

    xxContext*                      get_context();
    Window*                         get_window();
    Camera*                         get_camera();
    World*                          get_world();
    GPU_pool*                       get_GPU_pool();
    TextureManager*                 get_texture_manager();
    Sound*                          get_sound();
    Input*                          get_input();
    Factory*                        get_factory();
    Interface*                      get_interface();
    int32_t                         get_total_drawables();
    int32_t                         new_id();
    int64_t                         get_device_pool_size() { return m_device_pool_size; }
    int64_t                         get_host_visible_pool_size() { return m_host_visible_pool_size; }

    void                            set_renderer( Renderer* r );
    void                            save_geometry( Geometry* g );
    void                            save_city( CityGenerator* c );
    bool                            has_renderer( render_type t );
    Renderer*                       get_renderer( render_type t );

  private:
    Engine();
    ~Engine();

    static Engine*                  m_instance;

    std::shared_ptr<xxContext>      m_context;
    std::shared_ptr<Window>         m_window;
    std::shared_ptr<Camera>         m_camera;
    std::shared_ptr<World>          m_world;
    std::shared_ptr<GPU_pool>       m_gpu_pool;
    std::shared_ptr<TextureManager> m_texture_manager;
    std::shared_ptr<Sound>          m_sound;
    std::shared_ptr<Input>          m_input;
    std::shared_ptr<Interface>      m_interface;
    std::shared_ptr<Factory>        m_factory;
    Renderer*                       m_renderers[rCOUNT];
    std::vector<Geometry*>          m_geometries;
    CityGenerator*                  m_city;

    bool                            m_is_running;
    int32_t                         m_drawable_id_count;
    int64_t                         m_device_pool_size;
    int64_t                         m_host_visible_pool_size;
  };
}