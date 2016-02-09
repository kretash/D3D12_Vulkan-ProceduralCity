#include "core/engine_settings.hh"
#include "core/GPU_pool.hh"
#include "core/building.hh"
#include <algorithm>
#include <thread>
#include <iostream>
#include <cassert>

#define VERTEX_BUFFER_AVERAGE (uint32_t)250000
#define INDEX_BUFFER_AVERAGE (uint32_t)30000
#define BUFFER_SIZE_INFLATE (uint32_t)35000000

GPU_pool::GPU_pool() :
  m_vertex_pointer( 0 ),
  m_index_pointer( 0 ),
  m_instances( 0 ),
  m_max_vertex_buffer( 0 ),
  m_max_index_buffer( 0 ),
  m_placeholder_building( nullptr ) {

  m_removing_geometry.store( false );
  g_data = std::make_shared<geometry_data>();
  m_remove_threads.resize( 1 );

}

void GPU_pool::init() {
  int building_num = k_engine_settings->get_settings().grid * k_engine_settings->get_settings().grid;
  m_max_vertex_buffer = BUFFER_SIZE_INFLATE + VERTEX_BUFFER_AVERAGE * building_num;
  m_max_index_buffer = BUFFER_SIZE_INFLATE + INDEX_BUFFER_AVERAGE * building_num;

  GPU::create_empty_vertex_buffer( g_data.get(), m_max_vertex_buffer );
  GPU::create_empty_index_buffer( g_data.get(), m_max_index_buffer );

  // ------- squeeze a quad in the buffer
  const float size = 1.0f;
  const float e = 0.0f;
  float quad[] =
  {
     -size, -size, e, e, e, e, 0.0f, 1.0f, e, e, e, e, e, e,
     -size, size, e, e, e, e, 0.0f, 0.0f,  e, e, e, e, e, e,
    size, -size, e, e, e, e, 1.0f, 1.0f, e, e, e, e, e, e,
    size, size, e, e, e, e, 1.0f, 0.0f, e, e, e, e, e, e
  };
  queue q = {};
  q.v_block = mem_block( 0, sizeof( quad ) );
  q.v_data = &quad[0];
  m_upload_queue.push_back( q );
  GPU::upload_queue_into_vertex_buffer( g_data.get(), &m_upload_queue );
  m_upload_queue.clear();
  // -------------

  m_V_free_memory.push_back( mem_block( sizeof( quad ), m_max_vertex_buffer ) );
  m_I_free_memory.push_back( mem_block( 0, m_max_index_buffer ) );
}

void GPU_pool::set_placeholder_building( Geometry* b ) {
  m_placeholder_building = b;
}

void GPU_pool::queue_geometry( Geometry* b, float* v_data, uint32_t v_count, uint32_t* e_data, uint32_t e_count ) {
  if( m_removing_geometry.load() == true ) {
    std::cout << "thread sync failed.\n";
    m_remove_threads[0].join();
  }

  _save( b, v_data, v_count, e_data, e_count );
}

//0.1188
void GPU_pool::update() {

  GPU::upload_queue_into_vertex_buffer( g_data.get(), &m_upload_queue );
  GPU::upload_queue_into_index_buffer( g_data.get(), &m_upload_queue );

  //0.06
  while( m_upload_queue.size() != 0 ) {

    queue delete_me = m_upload_queue[0];
    m_upload_queue.erase( m_upload_queue.begin() );

    delete[] delete_me.v_data;
    delete[] delete_me.i_data;
  }

}

void GPU_pool::_save( Geometry* b, float* v_data, uint32_t v_count, uint32_t* e_data, uint32_t e_count ) {
  mem_block v_mem = { 0, 0 };
  mem_block i_mem = { 0, 0 };

  size_t v_size = v_count * sizeof( float );

  for( std::vector<mem_block>::iterator i = m_V_free_memory.begin(); i != m_V_free_memory.end(); ++i ) {
    if( i->m_size >= v_size ) {

      mem_block free_mem = *i._Ptr;
      i = m_V_free_memory.erase( i );

      v_mem.m_start = free_mem.m_start;
      v_mem.m_size = v_size;
      m_V_used_memory.push_back( v_mem );

      free_mem.m_start += v_size;
      free_mem.m_size -= v_size;
      m_V_free_memory.push_back( free_mem );
      break;
    }
  }

  assert( v_mem.m_size != 0 && "BLOCK NOT FOUND" );
  if( v_mem.m_size == 0 ) { std::cout << "full GPU v pool\n"; return; }
  b->set_vertex_offset( static_cast< uint32_t >( v_mem.m_start ) / sizeof( float ) );

  size_t e_size = e_count * sizeof( uint32_t );

  for( std::vector<mem_block>::iterator i = m_I_free_memory.begin(); i != m_I_free_memory.end(); ++i ) {
    if( i->m_size >= e_size ) {

      mem_block free_mem = *i._Ptr;
      i = m_I_free_memory.erase( i );

      i_mem.m_start = free_mem.m_start;
      i_mem.m_size = e_size;
      m_I_used_memory.push_back( i_mem );

      free_mem.m_start += e_size;
      free_mem.m_size -= e_size;
      m_I_free_memory.push_back( free_mem );
      break;
    }
  }

  assert( i_mem.m_size != 0 && "BLOCK NOT FOUND" );
  if( v_mem.m_size == 0 ) { std::cout << "full GPU i pool\n"; return; }
  b->set_index_offset( static_cast< int >( i_mem.m_start ) / sizeof( uint32_t ) );
  ++m_instances;

  //push to the queue
  m_upload_queue.push_back( queue( v_mem, v_data, i_mem, e_data ) );
}

void GPU_pool::remove( Geometry* b ) {

  assert( b->get_vertex_offset() != m_placeholder_building->get_vertex_offset() ||
    b->get_indicies_offset() != m_placeholder_building->get_indicies_offset()
    && "BUILDING ALREADY DELETED" );


  m_pool_mutex.lock();

  m_remove_queue.push_back(
    remove_queue( b->get_vertex_offset() * sizeof( float ),
      b->get_indicies_offset() * sizeof( uint32_t ) ) );

  m_pool_mutex.unlock();

  b->set_vertex_offset( m_placeholder_building->get_vertex_offset() );
  b->set_index_offset( m_placeholder_building->get_indicies_offset() );
  b->set_indicies_count( m_placeholder_building->get_indicies_count() );
}

void GPU_pool::_remove( remove_queue remove_me ) {

  int32_t rV_start = remove_me.v_mem;
  int32_t rI_start = remove_me.i_mem;
  int32_t removed = 0;

  for( std::vector<mem_block>::iterator i = m_V_used_memory.begin(); i != m_V_used_memory.end(); i++ ) {
    if( i->m_start == rV_start ) {

      mem_block used_mem = *i._Ptr;
      m_V_used_memory.erase( i );
      m_V_free_memory.push_back( used_mem );
      removed++;
      break;
    }
  }


  for( std::vector<mem_block>::iterator i = m_I_used_memory.begin(); i != m_I_used_memory.end(); i++ ) {
    if( i->m_start == rI_start ) {

      mem_block used_mem = *i._Ptr;
      m_I_used_memory.erase( i );
      m_I_free_memory.push_back( used_mem );
      removed++;
      break;
    }
  }

  assert( removed == 2 && "REMOVE COUNT WRONG" );
}

void GPU_pool::_defrag_vectors() {
  {
    std::sort( m_V_free_memory.begin(), m_V_free_memory.end() );
    std::vector<mem_block>::iterator i = m_V_free_memory.begin();
    while( i != m_V_free_memory.end() ) {

      std::vector<mem_block>::iterator next_i = i;
      next_i++;

      if( next_i == m_V_free_memory.end() )
        break;

      if( ( i->m_start + i->m_size ) == next_i->m_start ) {

        mem_block grouped = {};
        grouped.m_start = i->m_start;
        grouped.m_size = i->m_size + next_i->m_size;
        i = m_V_free_memory.insert( i, grouped );

        i++;
        i = m_V_free_memory.erase( i );
        i = m_V_free_memory.erase( i );
        i--;
      } else {
        i++;
      }
    }
  }
  {
    std::sort( m_I_free_memory.begin(), m_I_free_memory.end() );
    std::vector<mem_block>::iterator i = m_I_free_memory.begin();
    while( i != m_I_free_memory.end() ) {

      std::vector<mem_block>::iterator next_i = i;
      next_i++;

      if( next_i == m_I_free_memory.end() )
        break;

      if( ( i->m_start + i->m_size ) == next_i->m_start ) {

        mem_block grouped = {};
        grouped.m_start = i->m_start;
        grouped.m_size = i->m_size + next_i->m_size;
        i = m_I_free_memory.insert( i, grouped );

        i++;
        i = m_I_free_memory.erase( i );
        i = m_I_free_memory.erase( i );
        i--;
      } else {
        i++;
      }
    }
  }
}

void GPU_pool::start_remove_thread() {
  if( m_remove_threads[0].get_id() != std::thread::id() )
    m_remove_threads[0].join();

  m_remove_threads[0] = std::thread( &GPU_pool::_remove_thread, this );
}

void GPU_pool::_remove_thread() {
  m_removing_geometry.store( true );
  m_pool_mutex.lock();

  while( m_remove_queue.size() != 0 ) {

    remove_queue remove_me = m_remove_queue[0];
    m_remove_queue.erase( m_remove_queue.begin() );

    _remove( remove_me );

  }

  _defrag_vectors();

  m_pool_mutex.unlock();
  m_removing_geometry.store( false );
}

void GPU_pool::_debug_log() {
  if( ( uint32_t ) ( m_vertex_pointer * sizeof( float ) ) >= m_max_vertex_buffer ) {
    std::cout << "Vertex buffer overflow at " << m_instances << " instance" << std::endl;
    std::cout << m_vertex_pointer * sizeof( float ) << " out of " << m_max_vertex_buffer <<
      " by " << ( m_max_vertex_buffer ) -( m_vertex_pointer * sizeof( float ) ) << std::endl;
  }

  if( ( uint32_t ) ( m_index_pointer * sizeof( uint32_t ) ) >= m_max_index_buffer ) {
    std::cout << "Index buffer overflow at " << m_instances << " instance" << std::endl;
    std::cout << m_index_pointer * sizeof( uint32_t ) << " out of " << m_max_index_buffer <<
      " by " << m_max_index_buffer - ( m_index_pointer * sizeof( uint32_t ) ) << std::endl;
  }

  int32_t total_instances = k_engine_settings->get_settings().grid * k_engine_settings->get_settings().grid;
  int32_t LODs = 3;

  if( m_instances >= ( total_instances*LODs ) - 3 ) {
    std::cout << "Vertex buffer at " << m_instances << " instances" << std::endl;
    std::cout << m_vertex_pointer << " out of " << m_max_vertex_buffer << std::endl;
    std::cout << "Index buffer at " << m_instances << " instances" << std::endl;
    std::cout << m_index_pointer << " out of " << m_max_index_buffer << std::endl;
  }
}

GPU_pool::~GPU_pool() {

  for( int i = 0; i < m_remove_threads.size(); ++i ) {
    m_remove_threads[i].join();
  }

  m_remove_threads.clear();
}