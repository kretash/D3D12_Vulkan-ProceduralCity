#include <core/core.hh>
#if __VULKAN__
#include <core/vk_pool.hh>
#include <core/types.hh>
#include <cassert>
#include <algorithm>
#include <iostream>

VKPool::VKPool() {

}

void VKPool::init( uint64_t size, pool_type t ) {
  m_free_memory.push_back( mem_block( 0, size ) );
  m_pool_type = t;
}

mem_block VKPool::get_mem( uint64_t size ) {

  mem_block found = { 0, 0 };

  for( std::vector<mem_block>::iterator i = m_free_memory.begin(); i != m_free_memory.end(); ++i ) {
    if( i->m_size >= size ) {

      mem_block free_mem = *i._Ptr;
      i = m_free_memory.erase( i );

      found.m_start = free_mem.m_start;
      found.m_size = size;
      m_used_memory.push_back( found );

      free_mem.m_start += size;
      free_mem.m_size -= size;
      m_free_memory.push_back( free_mem );

      return found;
    }
  }

  assert( false && "BLOCK NOT FOUND" );
  std::cout << "GET BLOCK NOT FOUND " << m_pool_type << std::endl;
  return found;
}

void VKPool::release( mem_block m ) {

  bool found = false;

  for( std::vector<mem_block>::iterator i = m_used_memory.begin(); i != m_used_memory.end(); i++ ) {
    if( i->m_start == m.m_start ) {

      mem_block used_mem = *i._Ptr;
      m_used_memory.erase( i );
      m_free_memory.push_back( used_mem );
      found = true;
      break;
    }
  }

  assert( found != false && "BLOCK NOT FOUND" );
  if( !found ) std::cout << "RELEASE BLOCK NOT FOUND " << m_pool_type << std::endl;
}

void VKPool::defrag(){
  std::sort( m_free_memory.begin(), m_free_memory.end() );
  std::vector<mem_block>::iterator i = m_free_memory.begin();
  while( i != m_free_memory.end() ) {

    std::vector<mem_block>::iterator next_i = i;
    next_i++;

    if( next_i == m_free_memory.end() )
      break;

    if( ( i->m_start + i->m_size ) == next_i->m_start ) {

      mem_block grouped = {};
      grouped.m_start = i->m_start;
      grouped.m_size = i->m_size + next_i->m_size;
      i = m_free_memory.insert( i, grouped );

      i++;
      i = m_free_memory.erase( i );
      i = m_free_memory.erase( i );
      i--;
    } else {
      i++;
    }
  }
}

VKPool::~VKPool() {

}
#endif