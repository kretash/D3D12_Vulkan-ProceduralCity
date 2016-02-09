#include "core/drawable.hh"
#include "core/engine.hh"
#include "core/tools.hh"
#include <cassert>

Drawable::Drawable() {
  drawable_id = k_engine->new_id();
  m_position = float3( 0.0f, 0.0f, 0.0f );
  m_scale = float3( 1.0f, 1.0f, 1.0f );

  //LOD not supported by default
  m_geo_lod = 0;
  m_has_lod = 0;
  m_in_frustum = false;
  m_texture = std::make_shared<Texture>();
}

void Drawable::set_position( float x, float y, float z ) {
  m_position.x = x;
  m_position.y = y;
  m_position.z = z;
}

void Drawable::set_position( float3 pos ) {
  m_position = pos;
}

void Drawable::scale( float x, float y, float z ) {
  m_scale.x = x;
  m_scale.y = y;
  m_scale.z = z;
}

void Drawable::rotate( float x, float y, float z ) {
  m_rotation.x = x;
  m_rotation.y = y;
  m_rotation.z = z;
}

void Drawable::init( Geometry* g ) {
  m_geometry[0] = std::make_shared<Geometry>( g );
}

void Drawable::prepare() {
  assert( m_geometry[0] != nullptr && "NULL GEOMETRY" );

  if( m_geometry[1] != nullptr ) { m_has_lod = 1; }
  if( m_geometry[2] != nullptr ) { m_has_lod = 2; }

  m_geo_lod = 0;
}

void Drawable::update(  ) {

}

float4x4 Drawable::get_model() {
  m_model = float4x4( 1.0f );
  //m_model.rotate_x( m_rotation.x );
  m_model.scale( m_scale );
  m_model.rotate_y( m_rotation.y );
  //m_model.rotate_z( m_rotation.z );
  m_model.translate( m_position );
  return m_model;
}

void Drawable::set_lod( int32_t  i ) {
  if( i <= m_has_lod )
    m_geo_lod = i;
}

void Drawable::set_offsets( int32_t  cvb, int32_t  srv ) {
  m_offsets.cbv_offset = cvb;
  m_offsets.srv_offset = srv;
}

void Drawable::set_frustum_size( float radius, float max_height ) {
  m_radius = radius;
  m_max_height = max_height;
}

void Drawable::set_ignore_frustum() {
  m_radius = -0.1f;
  m_max_height = -0.1f;
}

Drawable::~Drawable() {
}