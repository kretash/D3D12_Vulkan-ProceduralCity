#include <cassert>

#include "core/building.hh"
#include "core/GPU_pool.hh"
#include "core/tools.hh"
#include "noise/OpenSimplexNoise.hh"

Building::Building( bool placeholder ) :
  m_empty( true ),
  m_noise( 0.0f ),
  m_num_floors( 0 ),
  m_num_sides( 0 ),
  m_instances( 0 ),
  m_angle_s( 0 ),
  m_side_neibours( 0 ),
  m_base_size( 0.0f ),
  m_instance_decrement( 0.0f ),
  m_ground_height( 0.0f ),
  m_ground_size_offset( 0.0f ),
  m_ground_multiplier( 0.0f ),
  m_spacers_size( 0.0f ),
  m_side_neibours_size( 0.0f ) {

  if( !placeholder ) {
    this->m_geometry[0] = std::make_shared<Geometry>( k_engine->get_GPU_pool()->get_placeholder_building() );
    this->m_geometry[1] = std::make_shared<Geometry>( k_engine->get_GPU_pool()->get_placeholder_building() );
    this->m_geometry[2] = std::make_shared<Geometry>( k_engine->get_GPU_pool()->get_placeholder_building() );
  }

  m_building_generator_LOD0 = std::auto_ptr<BuildingGen>( new BuildingGen );
  m_building_generator_LOD1 = std::auto_ptr<BuildingGen>( new BuildingGen );
  m_building_generator_LOD2 = std::auto_ptr<BuildingGen>( new BuildingGen );
  m_noise_handle = std::auto_ptr<OpenSimplexNoise>( new OpenSimplexNoise );

  m_ready_to_process = true;
}

void Building::prepare( float seed_x, float seed_y ) {
  if( !m_empty ) {
    k_engine->get_GPU_pool()->remove( get_geometry( 0 ) );
    k_engine->get_GPU_pool()->remove( get_geometry( 1 ) );
    k_engine->get_GPU_pool()->remove( get_geometry( 2 ) );
    m_empty = true;
  }

  _init_noise( seed_x, seed_y );
}

void Building::generate() {
  _generate_classic_building();
}

void Building::upload_and_clean() {
  m_empty = false;

  m_building_generator_LOD0->finish_and_upload();
  m_geometry[0] = std::make_shared<Geometry>( dynamic_cast< Geometry* >( m_building_generator_LOD0.get() ) );
  assert( m_geometry[0] != nullptr && "CAST TO GEOMETRY FAILED" );

  m_building_generator_LOD1->finish_and_upload();
  m_geometry[1] = std::make_shared<Geometry>( dynamic_cast< Geometry* >( m_building_generator_LOD1.get() ) );
  assert( m_geometry[1] != nullptr && "CAST TO GEOMETRY FAILED" );

  m_building_generator_LOD2->finish_and_upload();
  m_geometry[2] = std::make_shared<Geometry>( dynamic_cast< Geometry* >( m_building_generator_LOD2.get() ) );
  assert( m_geometry[2] != nullptr && "CAST TO GEOMETRY FAILED" );

}

void Building::generate_placeholder() {
  building_settings bs = {};
  bs.init_s( 4, 1, 30.0f, float3( 0.0f, 0.0f, 0.0f ) );
  bs.init_r( eSingle, 0, 0, 13.0f, m_main_texture_set );
  m_building_generator_LOD0->generate( bs );

  m_building_generator_LOD0->finish_and_upload();

  m_geometry[0] = nullptr;
  m_geometry[0] = std::make_shared<Geometry>( dynamic_cast< Geometry* >( m_building_generator_LOD0.get() ) );
  assert( m_geometry[0] != nullptr && "CAST TO GEOMETRY FAILED" );
  m_geometry[1] = m_geometry[0];
  m_geometry[2] = m_geometry[0];

  k_engine->get_GPU_pool()->set_placeholder_building( m_geometry[0].get() );
}

void Building::clear() {
  if( !m_empty ) {
    k_engine->get_GPU_pool()->remove( get_geometry( 0 ) );
    k_engine->get_GPU_pool()->remove( get_geometry( 1 ) );
    k_engine->get_GPU_pool()->remove( get_geometry( 2 ) );
    m_empty = true;
  }
}

void Building::_generate_classic_building() {

  m_num_floors = _get_p_rand( 2, 6, 77.0f );
  m_iteration_group = static_cast< groups >( _get_p_rand( 1, 3, 66.0f ) );

  if( m_iteration_group == eSingle ) {
    m_num_sides = _get_p_rand( 4, 6, 55.0f );
  } else if( m_iteration_group == eDouble ) {
    m_num_sides = _get_p_rand( 3, 6, 55.0f ) * 2;
    m_side_neibours_size = 35.0f / ( float ) m_num_sides;
  } else if( m_iteration_group == eTriple ) {
    m_num_sides = _get_p_rand( 3, 6, 55.0f ) * 3;
    m_side_neibours_size = 37.5f / ( float ) m_num_sides;
  } else if( m_iteration_group == eQuad ) {
    m_num_sides = _get_p_rand( 4, 6, 55.0f ) * 4;
    m_side_neibours_size = 40.0f / ( float ) m_num_sides;
  } else if( m_iteration_group == ePenta ) {
    m_num_sides = _get_p_rand( 4, 6, 55.0f ) * 5;
    m_side_neibours_size = 42.5f / ( float ) m_num_sides;
  }

  if( m_num_sides <= 6 ) {
    m_side_neibours_size = 5.0f * ( powf( 1.0f / ( m_num_sides - 0.05f ), 2.0f ) / powf( 1.0f / m_num_sides, 2.0f ) );
    m_side_neibours = _get_p_rand( 1, 2, 5.2f );
  } else {
    m_side_neibours_size = 0.0f;
    m_side_neibours = 0;
  }
  m_main_texture_set = eLEFT;
  m_roof_texture_set = eRIGHT;

  m_base_size = ( 3.0f * 16.0f ) / ( float ) m_num_sides;
  m_instances = _get_p_rand( 1, 3, 44.0f );
  m_instance_decrement = _get_p_rand( 0.75, 0.9f, 5.5f );
  m_ground_size_offset = _get_p_rand( 0.15f, 0.40f, 4.4f );
  m_ground_size_offset = -m_ground_size_offset;
  m_spacers_size = _get_p_rand( 0.1f, 0.15f, 3.3f );
  m_ground_height = 5.0f;

  m_angle_s = new int[m_instances + 1];
  for( int i = 0; i < m_instances + 1; ++i ) {
    m_angle_s[i] = _get_p_rand( 0, 5, 44.0f );
  }

  int side_s = _get_p_rand( 0, 5, 44.0f );
  float current_height = 0.0f;

  building_settings bs = {};
  bs.init_s( m_num_sides, 1, m_ground_height, float3( 0.0f, current_height, 0.0f ) );
  bs.init_r( m_iteration_group, m_angle_s[0], side_s, m_base_size + m_ground_size_offset, m_main_texture_set );
  m_building_generator_LOD0->generate( bs );
  current_height += 5.0f;

  for( int i = 0; i < m_instances; ++i ) {
    // Main
    float d = pow( m_instance_decrement, i );

    int current_floors = m_num_floors - 2 * i;
    current_floors = max( 1, current_floors );

    bs.init_s( m_num_sides, current_floors, 3.0f, float3( 0.0f, current_height, 0.0f ) );
    bs.init_r( m_iteration_group, m_angle_s[i + 1], side_s, d*m_base_size, m_main_texture_set );
    m_building_generator_LOD0->generate( bs );

    bs.n_vertical_uv = 0.05f;
    //Spacers
    for( int e = 0; e < current_floors; ++e ) {
      bs.init_s( m_num_sides, 1, 0.3f, float3( 0.0f, e * 3.0f + current_height, 0.0f ) );
      bs.init_r( m_iteration_group, m_angle_s[i + 1], side_s, d*( m_base_size + m_spacers_size ), m_roof_texture_set );  
      m_building_generator_LOD0->generate( bs );
    }
    bs.n_vertical_uv = 1.0f;

    //Neighbors
    if( i == 0 ) {
      if( m_side_neibours > 0 ) {
        bs.init_s( m_num_sides, current_floors, 3.0f, float3( d*m_side_neibours_size, current_height - 0.1f, 0.0f ) );
        bs.init_r( m_iteration_group, m_angle_s[i + 1], side_s, d*m_side_neibours_size, m_main_texture_set );
        m_building_generator_LOD0->generate( bs );

        bs.n_vertical_uv = 0.05f;
        for( int e = 0; e < current_floors; ++e ) {
          bs.init_s( m_num_sides, 1, 0.3f, float3( d*m_side_neibours_size, e * 3.0f + current_height, 0.0f ) );
          bs.init_r( m_iteration_group, m_angle_s[i + 1], side_s, d*( m_side_neibours_size + m_spacers_size ), m_roof_texture_set );
          m_building_generator_LOD0->generate( bs );
        }
        bs.n_vertical_uv = 1.0f;

        bs.init_s( m_num_sides, current_floors, 3.0f, float3( -d*m_side_neibours_size, current_height - 0.1f, 0.0f ) );
        bs.init_r( m_iteration_group, m_angle_s[i + 1], side_s, d*m_side_neibours_size, m_main_texture_set );
        m_building_generator_LOD0->generate( bs );

        bs.n_vertical_uv = 0.05f;
        for( int e = 0; e < current_floors; ++e ) {
          bs.init_s( m_num_sides, 1, 0.3f, float3( -d*m_side_neibours_size, e * 3.0f + current_height, 0.0f ) );
          bs.init_r( m_iteration_group, m_angle_s[i + 1], side_s, d*( m_side_neibours_size + m_spacers_size ), m_roof_texture_set );
          m_building_generator_LOD0->generate( bs );
        }
        bs.n_vertical_uv = 1.0f;
      }
      if( m_side_neibours > 1 ) {
        bs.init_s( m_num_sides, current_floors, 3.0f, float3( 0.0f, current_height - 0.1f, d*m_side_neibours_size ) );
        bs.init_r( m_iteration_group, m_angle_s[i + 1], side_s, d*m_side_neibours_size, m_main_texture_set );
        m_building_generator_LOD0->generate( bs );

        bs.n_vertical_uv = 0.05f;
        for( int e = 0; e < current_floors; ++e ) {
          bs.init_s( m_num_sides, 1, 0.3f, float3( 0.0f, e * 3.0f + current_height - 0.1f, d*m_side_neibours_size ) );
          bs.init_r( m_iteration_group, m_angle_s[i + 1], side_s, d*( m_side_neibours_size + m_spacers_size ), m_roof_texture_set );
          m_building_generator_LOD0->generate( bs );
        }
        bs.n_vertical_uv = 1.0f;

        bs.init_s( m_num_sides, current_floors, 3.0f, float3( 0.0f, current_height - 0.1f, -d*m_side_neibours_size ) );
        bs.init_r( m_iteration_group, m_angle_s[i + 1], side_s, d*m_side_neibours_size, m_main_texture_set );
        m_building_generator_LOD0->generate( bs );

        bs.n_vertical_uv = 0.05f;
        for( int e = 0; e < current_floors; ++e ) {
          bs.init_s( m_num_sides, 1, 0.3f, float3( 0.0f, e * 3.0f + current_height - 0.1f, -d*m_side_neibours_size ) );
          bs.init_r( m_iteration_group, m_angle_s[i + 1], side_s, d*( m_side_neibours_size + m_spacers_size ), m_roof_texture_set );
          m_building_generator_LOD0->generate( bs );
        }
        bs.n_vertical_uv = 1.0f;
      }
    }

    current_height += 3.0f * current_floors;
  }

  float d = pow( m_instance_decrement, m_instances - 1 );

  bs.init_s( m_num_sides, 1, 0.3f, float3( 0.0f, current_height, 0.0f ) );
  bs.init_r( m_iteration_group, m_angle_s[0], side_s, d * m_base_size * 0.9f, m_roof_texture_set );
  m_building_generator_LOD0->generate( bs );

  current_height += 0.3f;
  m_max_height = current_height+10.0f;
  m_radius = m_building_generator_LOD0->get_radius();

  current_height = 0.0f;

  bs.init_s( m_num_sides, 1, m_ground_height, float3( 0.0f, current_height, 0.0f ) );
  bs.init_r( m_iteration_group, m_angle_s[0], side_s, ( m_base_size + m_ground_size_offset ), m_main_texture_set );
  m_building_generator_LOD1->generate( bs );
  current_height += 5.0f;

  for( int i = 0; i < m_instances; ++i ) {
    // Main
    float d = pow( m_instance_decrement, i );

    int current_floors = m_num_floors - 2 * i;
    current_floors = max( 1, current_floors );

    bs.init_s( m_num_sides, current_floors, 3.0f, float3( 0.0f, current_height, 0.0f ) );
    bs.init_r( m_iteration_group, m_angle_s[i + 1], side_s, d*m_base_size, m_main_texture_set );
    m_building_generator_LOD1->generate( bs );

    current_height += 3.0f * current_floors;
  }

  // LOD 2
  bs.init_s( 4, 1, current_height, float3( 0.0f, 0.0f, 0.0f ) );
  bs.init_r( eSingle, 0, side_s, 13.0f, m_main_texture_set );
  m_building_generator_LOD2->generate( bs );

  delete[] m_angle_s;
}

void Building::_generate_modern_building() {

}

void Building::_generate_factory() {

}

void Building::_init_noise( float seed_x, float seed_y ) {
  float soft = 2.0f;
  float x = ( float ) seed_x / soft;
  float y = ( float ) seed_y / soft;
  float z = 1.0f;

  m_noise = m_noise_handle->eval( x, y, z );
  m_noise += 1.0f;
  m_noise /= 2.0f;
}

int32_t Building::_get_p_rand( int32_t min, int32_t max, float sample ) {
  return min + ( ( ( int ) ( m_noise * sample ) ) % max );
}

float Building::_get_p_rand( float min, float max, float sample ) {

  float tmp_noise = m_noise * sample;
  float range = max - min;
  assert( ( sample / range ) < 100 && "SAMPLE SIZE TOO BIG" );

  for( int i = 0; i < 100; ++i ) {
    if( tmp_noise < range )
      break;

    tmp_noise -= range;
  }

  assert( tools::clamp( tmp_noise, 0.0f, range ) == tmp_noise && "SHIET" );

  return min + tmp_noise;
}

Building::~Building() {
}