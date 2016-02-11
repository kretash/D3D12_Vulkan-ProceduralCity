#include "core/city_generetaor.hh"
#include "core/engine_settings.hh"
#include "core/texture_manager.hh"
#include "core/renderer.hh"
#include "core/building.hh"
#include "core/GPU_pool.hh"
#include "core/camera.hh"
#include "core/texture.hh"
#include "core/input.hh"
#include "core/tools.hh"
#include <limits>
#include <cassert>

#define building_ite std::vector<building_details>::iterator

CityGenerator::CityGenerator() :
  m_count( 0 ),
  m_grid( 0 ),
  m_half_grid( 0 ),
  m_scale( 0.0f ),
  m_max_radius( 0.0f ) {

  m_exit_threads.store( false );
}

void CityGenerator::_generate_loop() {
  while( !m_exit_threads.load() ) {
    if( m_to_generate_lock.try_lock() ) {
      if( m_to_generate.size() > 0 ) {
        Building* building = m_to_generate[0];
        m_to_generate.erase( m_to_generate.begin() );
        m_to_generate_lock.unlock();

        building->generate();

        assert( building->get_geometry( 0 )->get_indicies_count() != 0 && "EMPTY GEOMETRY" );
        assert( building->get_geometry( 1 )->get_indicies_count() != 0 && "EMPTY GEOMETRY" );
        assert( building->get_geometry( 2 )->get_indicies_count() != 0 && "EMPTY GEOMETRY" );

        m_to_upload_lock.lock();
        m_to_upload.push_back( building );
        m_to_upload_lock.unlock();
      } else {
        m_to_generate_lock.unlock();
        std::this_thread::sleep_for( std::chrono::milliseconds( 4 ) );
      }
    } else {
      std::this_thread::sleep_for( std::chrono::milliseconds( 4 ) );
    }
  }
}

void CityGenerator::generate( std::shared_ptr<Renderer> ren ) {

  m_grid = k_engine_settings->get_settings().grid;

  m_half_grid = m_grid / 2;
  m_scale = 30.0f;
  m_max_radius = m_scale * ( float ) m_half_grid;

  m_renderer = ren;

  m_street_block = std::make_shared<Geometry>();
  m_street_block->load( "street_block.obj" );

  m_placeholder_building = std::make_shared<Building>( true );
  m_placeholder_building->generate_placeholder();
  m_placeholder_building->get_texture()->init_procedural( 0.0f, 0.0f, 0 );

  m_count = 0;
  m_buildigs.resize( m_grid*m_grid );
  m_street_block_D.resize( m_grid*m_grid );

  for( int32_t i = 0; i < m_grid; i++ ) {
    for( int32_t e = 0; e < m_grid; e++ ) {

      m_buildigs[m_count] = std::make_shared<Building>();

      m_buildigs[m_count]->set_position( ( m_half_grid - e )*m_scale, 0.0f, ( m_half_grid - i )*m_scale );
      m_building_positions[m_count] = float3( ( m_half_grid - e )*m_scale, 0.0f, ( m_half_grid - i )*m_scale );

      building_details bd = {};
      bd.building_i = m_count;
      bd.position.x = ( m_half_grid - e )*m_scale;
      bd.position.y = 0.0f;
      bd.position.z = ( m_half_grid - i )*m_scale;
      bd.type = kTOP;
      m_all_buildings.push_back( bd );

      float seed_x = ( float ) ( m_half_grid - e )*m_scale;
      float seed_y = ( float ) ( m_half_grid - i )*m_scale;

      m_buildigs[m_count]->prepare( seed_x, seed_y );
      m_buildigs[m_count]->get_texture()->init_procedural( seed_x, seed_y, 0 );

      m_renderer->add_child( m_buildigs[m_count].get() );


      m_street_block_D[m_count] = std::make_shared<Drawable>();
      m_street_block_D[m_count]->set_frustum_size( 35.0f, 30.0f );
      m_street_block_D[m_count]->set_position( ( m_half_grid - e )*m_scale, 0.0f, ( m_half_grid - i )*m_scale );
      m_street_block_D[m_count]->init( m_street_block.get() );

      m_street_block_D[m_count]->get_texture()->load( tDIFFUSE, "street_block_d.png" );
      m_street_block_D[m_count]->get_texture()->load( tNORMAL, "street_block_n.png" );
      m_street_block_D[m_count]->get_texture()->load( tSPECULAR, "street_block_s.png" );

      m_renderer->add_child( m_street_block_D[m_count].get() );

      ++m_count;
    }
  }

  for( int i = 0; i < 5; ++i )
    m_threads.push_back( std::thread( &CityGenerator::_generate_loop, this ) );

  m_count = 0;
  m_to_generate_lock.lock();
  for( int32_t i = 0; i < m_grid; i++ ) {
    for( int32_t e = 0; e < m_grid; e++ ) {
      m_buildigs[m_count]->set_ready_to_process( false );
      m_to_generate.push_back( m_buildigs[m_count].get() );
      ++m_count;
    }
  }
  m_to_generate_lock.unlock();

  for( int32_t i = 0; i < m_grid; i++ ) {
    m_outline_positions.push_back(
      building_details( float3( ( m_half_grid - i )*m_scale, 0.0f, ( m_half_grid + 1 )*m_scale ), kTOP ) );
  }

  for( int32_t e = 0; e < m_grid; e++ ) {
    m_outline_positions.push_back(
      building_details( float3( ( m_half_grid + 1 )*m_scale, 0.0f, ( m_half_grid - e )*m_scale ), kLEFT ) );
  }

  for( int32_t i = 0; i < m_grid; i++ ) {
    m_outline_positions.push_back(
      building_details( float3( ( m_half_grid - m_grid )*m_scale, 0.0f, ( m_half_grid - i )*m_scale ), kRIGHT ) );
  }

  for( int32_t i = 0; i < m_grid; i++ ) {
    m_outline_positions.push_back(
      building_details( float3( ( m_half_grid - i )*m_scale, 0.0f, ( m_half_grid - m_grid )*m_scale ), kBOT ) );
  }

  m_outline_positions[0].carry = kLEFT;
  m_outline_positions[m_grid - 1].carry = kRIGHT;

  m_outline_positions[m_grid].carry = kTOP;
  m_outline_positions[2 * m_grid - 1].carry = kBOT;

  m_outline_positions[2 * m_grid].carry = kTOP;
  m_outline_positions[3 * m_grid - 1].carry = kBOT;

  m_outline_positions[3 * m_grid].carry = kLEFT;
  m_outline_positions[4 * m_grid - 1].carry = kRIGHT;

}

void CityGenerator::update() {
  std::cout.precision( 5 );

#ifdef _DEBUG
  uint32_t max_movements = 50;
#else
  uint32_t max_movements = 250;
#endif

  static int count = 0;

  if( m_move_operations.size() == 0 && count == 1 ) { //0.1 -> 0.5, spikes to 1.7
    _prepare_vectors();
    _generate_move_buildings();
  }


  if( count == 0 ) { //0.01 norm and spikes to 0.13 max
    _apply_move_buildings( max_movements );
    k_engine->get_GPU_pool()->start_remove_thread();
  }


  if( count == 2 ) {
    if( m_to_upload_lock.try_lock() ) {
      bool locked = true;

      for( uint32_t i = 0; i < max_movements; ++i ) {
        if( m_to_upload.size() == 0 ) {
          locked = false;
          m_to_upload_lock.unlock();
          break;
        }

        Building* building = m_to_upload[0];
        m_to_upload.erase( m_to_upload.begin() );

        building->upload_and_clean();
        building->set_ready_to_process( true );
      }
      if( locked ) m_to_upload_lock.unlock();
    }
  }

  ++count;
  count = count % 3;
}

void CityGenerator::_prepare_vectors() {

  float3 position = k_engine->get_camera()->get_position();
  float3 camera( position.x, 0.0f, position.z );

  {
    for( building_ite i = m_outline_positions.begin(); i != m_outline_positions.end(); ++i ) {
      float3 border( i->position.x, 0.0f, i->position.z );
      i->distance = float3::lenght( camera - border );
    }
    std::sort( m_outline_positions.begin(), m_outline_positions.end() );
  }

  if( m_outline_positions[0].distance >= m_max_radius )
    return;

  {
    for( building_ite i = m_all_buildings.begin(); i != m_all_buildings.end(); ++i ) {
      float3 building( i->position.x, 0.0f, i->position.z );
      i->distance = float3::lenght( camera - building );
    }
    std::sort( m_all_buildings.begin(), m_all_buildings.end() );
  }

}

void CityGenerator::_generate_move_buildings() {

  if( m_outline_positions[0].distance >= m_max_radius )
    return;

  int32_t  outline_count = 0;
  int32_t  building_moves = 0;
  int32_t  outline_moves = 0;

  outline_type my_type = m_outline_positions[0].type;

  float3 my_building = {};
  float3 difference = {};
  if( my_type == kTOP ) {
    my_building = float3( 0.0f, 0.0f, -( m_scale * m_grid ) );
    difference = float3( 0.0f, 0.0f, ( m_scale ) );
  } else if( my_type == kBOT ) {
    my_building = float3( 0.0f, 0.0f, ( m_scale * m_grid ) );
    difference = float3( 0.0f, 0.0f, -( m_scale ) );
  } else if( my_type == kRIGHT ) {
    my_building = float3( ( m_scale * m_grid ), 0.0f, 0.0f );
    difference = float3( -( m_scale ), 0.0f, 0.0f );
  } else if( my_type == kLEFT ) {
    my_building = float3( -( m_scale * m_grid ), 0.0f, 0.0f );
    difference = float3( ( m_scale ), 0.0f, 0.0f );
  }

  for( building_ite i = m_outline_positions.begin(); i != m_outline_positions.end(); ++i ) {

    if( i->type == my_type ) {

      float3 position = i->position + my_building;

      {
        _apply_carry_to_outline( *i );
        i->position = i->position + difference;
        ++outline_moves;
      }

      int32_t  building_count = 0;
      for( building_ite e = m_all_buildings.begin(); e != m_all_buildings.end(); ++e ) {

        if( e->position == position ) {

          move_operation move_me = {};
          move_me.type = kBUILDING_MOVE;
          move_me.building_i = e->building_i;
          move_me.end_position = e->position - my_building;
          m_move_operations.push_back( move_me );

          e->position = move_me.end_position;
          ++building_moves;

          break;
        }
        ++building_count;
      }

    } else if( i->type == _oposite( my_type ) ) {
      i->position = i->position + difference;
      ++outline_moves;
    }

    ++outline_count;
  }
  assert( ( 2 * building_moves ) == outline_moves && "SIZE DIFFERENCE" );
}

void CityGenerator::_apply_move_buildings( uint32_t count ) {
  if( m_to_generate_lock.try_lock() == false )
    return;

  for( uint32_t i = 0; i < count; ++i ) {
    if( m_move_operations.size() == 0 )
      break;

    move_operation move_me = m_move_operations[0];

    if( m_buildigs[move_me.building_i]->is_ready_to_process() == false )
      break;

    m_move_operations.erase( m_move_operations.begin() );

    m_buildigs[move_me.building_i]->set_ready_to_process( false );
    m_buildigs[move_me.building_i]->set_position( move_me.end_position );
    m_buildigs[move_me.building_i]->prepare( move_me.end_position.x, move_me.end_position.z );
    m_street_block_D[move_me.building_i]->set_position( move_me.end_position );

    m_to_generate.push_back( m_buildigs[move_me.building_i].get() );

  }

  m_to_generate_lock.unlock();
}

void CityGenerator::_apply_carry_to_outline( building_details outline ) {
  if( outline.carry != 0 ) {
    for( std::vector<building_details>::iterator all_outlines = m_outline_positions.begin(); all_outlines != m_outline_positions.end(); ++all_outlines ) {
      if( all_outlines->type == outline.carry ) {


        if( outline.type == kTOP ) {
          all_outlines->position.z += m_scale;

          if( ( all_outlines - 1 )->type == outline.carry )
            all_outlines->position.x = ( all_outlines - 1 )->position.x;

        } else if( outline.type == kBOT ) {
          all_outlines->position.z -= m_scale;

          if( ( all_outlines + 1 )->type == outline.carry )
            all_outlines->position.x = ( all_outlines + 1 )->position.x;

        } else if( outline.type == kRIGHT ) {
          all_outlines->position.x -= m_scale;

          if( ( all_outlines + 1 )->type == outline.carry )
            all_outlines->position.z = ( all_outlines + 1 )->position.z;

        } else if( outline.type == kLEFT ) {
          all_outlines->position.x += m_scale;

          if( ( all_outlines - 1 )->type == outline.carry )
            all_outlines->position.z = ( all_outlines - 1 )->position.z;
        }

      }
    }
  }
}

outline_type CityGenerator::_oposite( outline_type s ) {
  if( s == kTOP ) return kBOT;
  if( s == kBOT ) return kTOP;
  if( s == kLEFT ) return kRIGHT;
  if( s == kRIGHT ) return kLEFT;
  return kNONE;
}

CityGenerator::~CityGenerator() {

  m_exit_threads.store( true );

  for( int i = 0; i < m_threads.size(); ++i )
    m_threads[i].join();

  m_buildigs.clear();
  m_buildigs.shrink_to_fit();
  m_street_block_D.clear();
  m_street_block_D.shrink_to_fit();

  m_placeholder_building.reset();
  m_street_block.reset();
  m_texture.reset();
  m_texture.reset();
}