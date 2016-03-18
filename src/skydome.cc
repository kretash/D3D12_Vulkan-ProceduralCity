#include "core/geometry.hh"
#include "core/texture.hh"
#include "core/renderer.hh"
#include "core/skydome.hh"
#include "core/camera.hh"

namespace kretash {

  Skydome::Skydome() {

  }

  void Skydome::init() {
    m_geometry = std::make_shared<Geometry>();
    m_drawable = std::make_shared<Drawable>();
    m_renderer = std::make_shared<Renderer>();

    m_geometry->load( "skydome.obj" );

    m_drawable->set_ignore_frustum();
    m_drawable->init( m_geometry.get() );

    m_renderer->create( rSKYDOME );
    m_renderer->add_child( m_drawable.get() );
  }

  void Skydome::update() {
    Camera* camera = k_engine->get_camera();
    m_drawable->set_position( float3( camera->get_position().x, 0.0f, camera->get_position().z ) );
    m_renderer->update();
  }

  Skydome::~Skydome() {

  }
}