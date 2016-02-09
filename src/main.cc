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

//#include <vld.h> 

#include "core/core.hh"
#include "core/engine.hh"
#include "core/window.hh"
#include "core/geometry.hh"
#include "core/renderer.hh"
#include "core/drawable.hh"
#include "core/skydome.hh"
#include "core/tools.hh"
#include "core/building.hh"
#include "core/texture.hh"
#include "core/engine_settings.hh"
#include "core/city_generetaor.hh"
#include "core/texture_manager.hh"

#define TEST_OBJ 0

int main( int argc, char **argv ) {

  k_engine->init();

  std::shared_ptr<Renderer> ren = std::make_shared<Renderer>();
  ren->create( rTEXTURE );

  std::shared_ptr<Renderer> post = std::make_shared<Renderer>();
  post->create( rPOST );
  
  std::shared_ptr<CityGenerator> c_gen = std::make_shared<CityGenerator>();
  c_gen->generate( ren );

  std::shared_ptr<Skydome> sky = std::make_shared<Skydome>();
  sky->init();

#if TEST_OBJ
  std::shared_ptr<Geometry> geometry = std::make_shared<Geometry>();
  geometry->load( "plane.obj" );
  std::shared_ptr<Drawable> drawable = std::make_shared<Drawable>();
  drawable->init( geometry.get() );
  drawable->set_ignore_frustum();
  drawable->get_texture()->load( tDIFFUSE, "chair_d.png" );
  drawable->get_texture()->load( tNORMAL, "chair_n.png" );
  drawable->get_texture()->load( tSPECULAR, "chair_s.png" );
  ren->add_child( drawable.get() );

  std::shared_ptr<Geometry> teapot = std::make_shared<Geometry>();
  teapot->load( "buddha.obj" );
  std::shared_ptr<Drawable> tea = std::make_shared<Drawable>();
  tea->init( teapot.get() );
  tea->set_ignore_frustum();
  tea->get_texture()->load( tDIFFUSE, "marble_d.png" );
  tea->get_texture()->load( tNORMAL, "marble_n.png" );
  tea->get_texture()->load( tSPECULAR, "marble_s.png" );
  tea->set_position( 15.0f, 0.0f, 15.0f );
  ren->add_child( tea.get() );

  float rotation = 0.0f;
#endif

  k_engine->prepare();
  k_engine->get_window()->capture_mouse();
  EngineSettings* es = k_engine_settings;

  while( k_engine->is_running() ) {
    es->start_frame();

#if TEST_OBJ
    tea->rotate( 0.0f, rotation, 0.0f );
    rotation += 0.01f;
#endif

    k_engine->update();
    ren->update();
    sky->update();
    c_gen->update();

    es->start_render();

    k_engine->reset_cmd_list();
    k_engine->clear_color();
    k_engine->clear_depth();

    k_engine->render_skydome( sky.get() );
    k_engine->render( ren.get() );
    k_engine->render_post( post.get() );

    k_engine->execute_and_swap( ren.get() );

    es->end_frame();
  }

  return 0;
}
