#include "vulkan/vulkandebug.h"
#include "core/vk/context.hh"
#include "core/vk/geometry.hh"
#include "core/engine.hh"
#include "core/types.hh"

namespace kretash {

  /* This will create an empty vertex buffer in Vulkan and D3D12 */
  void vkGeometry::create_empty_vertex_buffer( uint64_t size ) {

    vkContext* m_context = dynamic_cast<vkContext*>( k_engine->get_context() );

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

    VkResult vkr = vkCreateBuffer( m_context->m_device, &buffer_info, nullptr, &m_v_buf );
    vkassert( vkr );

    vkGetBufferMemoryRequirements( m_context->m_device, m_v_buf, &mem_reqs );
    mem_alloc.allocationSize = mem_reqs.size;
    m_context->_get_memory_type( mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &mem_alloc.memoryTypeIndex );
    vkr = vkAllocateMemory( m_context->m_device, &mem_alloc, nullptr, &m_v_mem );
    vkassert( vkr );
    vkr = vkMapMemory( m_context->m_device, m_v_mem, 0, mem_alloc.allocationSize, 0, &data );
    vkassert( vkr );
    memset( data, 0, size );
    vkUnmapMemory( m_context->m_device, m_v_mem );
    vkr = vkBindBufferMemory( m_context->m_device, m_v_buf, m_v_mem, 0 );
    vkassert( vkr );

  }

  /* This will create an empty index buffer in Vulkan and D3D12 */
  void vkGeometry::create_empty_index_buffer( uint64_t size ) {

    vkContext* m_context = dynamic_cast<vkContext*>( k_engine->get_context() );

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

    VkResult vkr = vkCreateBuffer( m_context->m_device, &index_buffer_info, nullptr, &m_i_buf );
    vkassert( vkr );

    vkGetBufferMemoryRequirements( m_context->m_device, m_i_buf, &mem_reqs );
    mem_alloc.allocationSize = mem_reqs.size;
    m_context->_get_memory_type( mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &mem_alloc.memoryTypeIndex );
    vkr = vkAllocateMemory( m_context->m_device, &mem_alloc, nullptr, &m_i_mem );
    vkassert( vkr );

    vkr = vkMapMemory( m_context->m_device, m_i_mem, 0, size, 0, &data );
    vkassert( vkr );
    memset( data, 0, size );
    vkUnmapMemory( m_context->m_device, m_i_mem );
    vkr = vkBindBufferMemory( m_context->m_device, m_i_buf, m_i_mem, 0 );
    vkassert( vkr );

  }

  /* This will upload into an vertex buffer in Vulkan and D3D12 */
  void vkGeometry::upload_into_vertex_buffer( uint64_t offset, float* array_data, uint64_t size ) {

    vkContext* m_context = dynamic_cast<vkContext*>( k_engine->get_context() );

    void* data = nullptr;
    VkResult vkr = vkMapMemory( m_context->m_device, m_v_mem, offset, size, 0, &data );
    vkassert( vkr );
    memcpy( data, array_data, size );
    vkUnmapMemory( m_context->m_device, m_v_mem );

  }

  /* This will queue an upload into an vertex buffer in Vulkan and D3D12 */
  void vkGeometry::upload_queue_into_vertex_buffer( std::vector<kretash::queue>* queue ) {

    vkContext* m_context = dynamic_cast<vkContext*>( k_engine->get_context() );

    void* data = nullptr;
    VkResult vkr = vkMapMemory( m_context->m_device, m_v_mem, 0, size_v, 0, &data );
    vkassert( vkr );
    for( int i = 0; i < queue->size(); ++i ) {
      uint8_t* data_offset = ( uint8_t* ) data + ( ( *queue )[i].v_block.m_start );
      memcpy( data_offset, ( *queue )[i].v_data, ( *queue )[i].v_block.m_size );
    }
    vkUnmapMemory( m_context->m_device, m_v_mem );

  }

  /* This will upload into an index buffer in Vulkan and D3D12 */
  void vkGeometry::upload_into_index_buffer( uint64_t offset, uint32_t* elements_data, uint64_t size ) {

    vkContext* m_context = dynamic_cast<vkContext*>( k_engine->get_context() );

    void* data = nullptr;
    VkResult vkr = vkMapMemory( m_context->m_device, m_i_mem, offset, size, 0, &data );
    vkassert( vkr );
    memcpy( data, 0, size );
    vkUnmapMemory( m_context->m_device, m_i_mem );

  }

  /* This will queue an upload into an index buffer in Vulkan and D3D12 */
  void vkGeometry::upload_queue_into_index_buffer( std::vector<kretash::queue>* queue ) {

    vkContext* m_context = dynamic_cast<vkContext*>( k_engine->get_context() );

    void* data = nullptr;
    VkResult vkr = vkMapMemory( m_context->m_device, m_i_mem, 0, size_i, 0, &data );
    vkassert( vkr );

    for( int i = 0; i < queue->size(); ++i ) {
      uint8_t* data_offset = ( uint8_t* ) data + ( ( *queue )[i].i_block.m_start );
      memcpy( data_offset, ( *queue )[i].i_data, ( *queue )[i].i_block.m_size );
    }

    vkUnmapMemory( m_context->m_device, m_i_mem );

  }

  vkGeometry::vkGeometry() {
    size_v = 0;
    size_i = 0;

    if( m_v_buf != VK_NULL_HANDLE ) {
      vkContext* m_context = dynamic_cast<vkContext*>( k_engine->get_context() );
      vkDestroyBuffer( m_context->m_device, m_v_buf, nullptr );
      m_v_buf = VK_NULL_HANDLE;
    }
    if( m_i_buf != VK_NULL_HANDLE ) {
      vkContext* m_context = dynamic_cast<vkContext*>( k_engine->get_context() );
      vkDestroyBuffer( m_context->m_device, m_i_buf, nullptr );
      m_i_buf = VK_NULL_HANDLE;
    }
    if( m_v_mem != VK_NULL_HANDLE ) {
      vkContext* m_context = dynamic_cast<vkContext*>( k_engine->get_context() );
      vkFreeMemory( m_context->m_device, m_i_mem, nullptr );
      m_v_mem = VK_NULL_HANDLE;
    }
    if( m_i_mem != VK_NULL_HANDLE ) {
      vkContext* m_context = dynamic_cast<vkContext*>( k_engine->get_context() );
      vkFreeMemory( m_context->m_device, m_i_mem, nullptr );
      m_i_mem = VK_NULL_HANDLE;
    }

  }

  vkGeometry::~vkGeometry() {

    if( m_v_buf != VK_NULL_HANDLE ) {
      vkContext* m_context = dynamic_cast<vkContext*>( k_engine->get_context() );
      vkDestroyBuffer( m_context->m_device, m_v_buf, nullptr );
      m_v_buf = VK_NULL_HANDLE;
    }
    if( m_i_buf != VK_NULL_HANDLE ) {
      vkContext* m_context = dynamic_cast<vkContext*>( k_engine->get_context() );
      vkDestroyBuffer( m_context->m_device, m_i_buf, nullptr );
      m_i_buf = VK_NULL_HANDLE;
    }
    if( m_v_mem != VK_NULL_HANDLE ) {
      vkContext* m_context = dynamic_cast<vkContext*>( k_engine->get_context() );
      vkFreeMemory( m_context->m_device, m_v_mem, nullptr );
      m_v_mem = VK_NULL_HANDLE;
    }
    if( m_i_mem != VK_NULL_HANDLE ) {
      vkContext* m_context = dynamic_cast<vkContext*>( k_engine->get_context() );
      vkFreeMemory( m_context->m_device, m_i_mem, nullptr );
      m_i_mem = VK_NULL_HANDLE;
    }

  }
}