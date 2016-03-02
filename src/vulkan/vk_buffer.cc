#include "core/k_vulkan.hh"
#include "vulkan/vulkan.h"
#include "vulkan/vulkantools.h"
#include "core/GPU_pool.hh"
#include "core/types.hh"
#ifdef __VULKAN__

using namespace kretash;

namespace vk {

  //remove this when chaos is over
  static uint32_t size_v = 0;
  static uint32_t size_i = 0;

  void create_empty_vertex_buffer( engine_data* e, geometry_data* g, uint32_t size ) {
    size_v = size;

    VkMemoryAllocateInfo mem_alloc = {};
    mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mem_alloc.pNext = nullptr;
    mem_alloc.allocationSize = 0;
    mem_alloc.memoryTypeIndex = 0;

    VkMemoryRequirements mem_reqs = {};
    void* data = nullptr;

    VkBufferCreateInfo buffer_info = {};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.pNext = nullptr;
    buffer_info.size = size;
    buffer_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    buffer_info.flags = 0;

    VkResult vkr = vkCreateBuffer( e->m_device, &buffer_info, nullptr, &g->m_v_buf );
    vkassert( vkr );

    vkGetBufferMemoryRequirements( e->m_device, g->m_v_buf, &mem_reqs );
    mem_alloc.allocationSize = mem_reqs.size;
    _get_memory_type( e, mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &mem_alloc.memoryTypeIndex );
    vkr = vkAllocateMemory( e->m_device, &mem_alloc, nullptr, &g->m_v_mem );
    vkassert( vkr );
    vkr = vkMapMemory( e->m_device, g->m_v_mem, 0, mem_alloc.allocationSize, 0, &data );
    vkassert( vkr );
    memset( data, 0, size );
    vkUnmapMemory( e->m_device, g->m_v_mem );
    vkr = vkBindBufferMemory( e->m_device, g->m_v_buf, g->m_v_mem, 0 );
    vkassert( vkr );
  }

  void create_empty_index_buffer( engine_data* e, geometry_data* g, uint32_t size ) {
    size_i = size;

    VkMemoryAllocateInfo mem_alloc = {};
    mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mem_alloc.pNext = nullptr;
    mem_alloc.allocationSize = 0;
    mem_alloc.memoryTypeIndex = 0;

    VkMemoryRequirements mem_reqs = {};
    void* data = nullptr;

    VkBufferCreateInfo index_buffer_info = {};
    index_buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    index_buffer_info.pNext = nullptr;
    index_buffer_info.size = size;
    index_buffer_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    index_buffer_info.flags = 0;

    VkResult vkr = vkCreateBuffer( e->m_device, &index_buffer_info, nullptr, &g->m_i_buf );
    vkassert( vkr );

    vkGetBufferMemoryRequirements( e->m_device, g->m_i_buf, &mem_reqs );
    mem_alloc.allocationSize = mem_reqs.size;
    _get_memory_type( e, mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &mem_alloc.memoryTypeIndex );
    vkr = vkAllocateMemory( e->m_device, &mem_alloc, nullptr, &g->m_i_mem );
    vkassert( vkr );

    vkr = vkMapMemory( e->m_device, g->m_i_mem, 0, size, 0, &data );
    vkassert( vkr );
    memset( data, 0, size );
    vkUnmapMemory( e->m_device, g->m_i_mem );
    vkr = vkBindBufferMemory( e->m_device, g->m_i_buf, g->m_i_mem, 0 );
    vkassert( vkr );
  }

  void upload_into_vertex_buffer( engine_data* e, geometry_data* g, uint32_t offset, float* array_data, uint32_t size ) {
    void* data = nullptr;
    VkResult vkr = vkMapMemory( e->m_device, g->m_v_mem, offset, size, 0, &data );
    vkassert( vkr );
    memcpy( data, array_data, size );
    vkUnmapMemory( e->m_device, g->m_v_mem );
  }

  void upload_queue_into_vertex_buffer( engine_data* e, geometry_data* g, std::vector<queue>* queue ) {
    void* data = nullptr;
    VkResult vkr = vkMapMemory( e->m_device, g->m_v_mem, 0, size_v, 0, &data );
    vkassert( vkr );
    for( int i = 0; i < queue->size(); ++i ) {
      uint8_t* data_offset = ( uint8_t* ) data + ( ( *queue )[i].v_block.m_start );
      memcpy( data_offset, ( *queue )[i].v_data, ( *queue )[i].v_block.m_size );
    }
    vkUnmapMemory( e->m_device, g->m_v_mem );

  }

  void upload_into_index_buffer( engine_data* e, geometry_data* g, uint32_t offset, uint32_t* elements_data, uint32_t size ) {
    void* data = nullptr;
    VkResult vkr = vkMapMemory( e->m_device, g->m_i_mem, offset, size, 0, &data );
    vkassert( vkr );
    memcpy( data, 0, size );
    vkUnmapMemory( e->m_device, g->m_i_mem );

  }

  void upload_queue_into_index_buffer( engine_data* e, geometry_data* g, std::vector<queue>* queue ) {
    void* data = nullptr;
    VkResult vkr = vkMapMemory( e->m_device, g->m_i_mem, 0, size_i, 0, &data );
    vkassert( vkr );

    for( int i = 0; i < queue->size(); ++i ) {
      uint8_t* data_offset = ( uint8_t* ) data + ( ( *queue )[i].i_block.m_start );
      memcpy( data_offset, ( *queue )[i].i_data, ( *queue )[i].i_block.m_size );
    }

    vkUnmapMemory( e->m_device, g->m_i_mem );

  }
}

#endif