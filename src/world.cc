#include "core/world.hh"
#include "core/tools.hh"
#include "core/engine.hh"
#include "core/camera.hh"

World::World() {

}

void World::init() {
  m_buffer.light_pos = float3( 0.5, -0.5f, 0.0f );
  GPU::create_constant_buffer_object( &m_buffer_data, m_buffer );
  GPU::update_constant_buffer_object( &m_buffer_data, m_buffer );
}

void World::update() {

  static float time = 1.5f;
  time -= 0.00055f;

  float sky_color = cosf( time ) + 0.3f;
  sky_color *= 3.07692307692308f;
  sky_color = tools::clamp( sky_color, 0.0f, 4.0f );
  m_buffer.sky_color = sky_color;

  const float3 all_horizon_color[5] =
  {
    float3( 69.0f / 256.f, 56.0f / 256.f, 47.0f / 256.f ),    // Night
    float3( 239.0f / 256.f, 96.0f / 256.f, 40.0f / 256.f ),   // Sunset
    float3( 127.0f / 256.f, 190.0f / 256.f, 233.0f / 256.f ), // Day 1
    float3( 137.0f / 256.f, 199.0f / 256.f, 236.0f / 256.f ), // Day 2
    float3( 147.0f / 256.f, 210.0f / 256.f, 243.0f / 256.f )  // Day 3
  };

  int32_t current = 0;
  int32_t next = 0;
  float blend = 0.0f;

  int32_t i = ( int32_t ) sky_color;
  current = i;
  next = i + 1;
  next = tools::clamp( next, 0, 4 );
  blend = sky_color - ( float ) i;

  m_buffer.fog_color = tools::lerp( all_horizon_color[current], all_horizon_color[next], blend );

  m_buffer.ambient_intensity = 0.1f + tools::clamp( sky_color - 1.0f, 0.0f, 3.0f ) / 10.0f;
  
  if( sky_color == 0.0f ) {
    m_buffer.light_pos = float3( 0.0f, 0.0f, 0.0f );
    m_buffer.sun_light_intensity = float3( 0.0f, 0.0f, 0.0f );
  } else {
    float3 pos = float3( sinf( time ), cosf( time ), 0.5f );
    m_buffer.light_pos = pos;

    m_buffer.sun_light_intensity = m_buffer.fog_color * 0.3f + 
      float3( 1.0f, 1.0f, 1.0f ) *m_buffer.ambient_intensity;
  }

  Camera* c = k_engine->get_camera();
  m_buffer.eye_view = c->get_position();
  m_buffer.view = c->get_view();

  GPU::update_constant_buffer_object( &m_buffer_data, m_buffer );
}

World::~World() {

}