#include "core/k_vulkan.hh"

#ifdef __VULKAN__

#include "core/engine.hh"
#include "core/drawable.hh"
#include "vulkan/vulkantools.h"
using namespace kretash;

namespace vk {

  void init_descriptor_pool_and_layout( engine_data* e ) {
    VkDescriptorSetLayoutBinding layout_binding[2] = {};
    layout_binding[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layout_binding[0].descriptorCount = 1;
    layout_binding[0].stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
    layout_binding[0].pImmutableSamplers = nullptr;
    layout_binding[0].binding = 0;

    layout_binding[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layout_binding[1].descriptorCount = 1;
    layout_binding[1].stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
    layout_binding[1].pImmutableSamplers = nullptr;
    layout_binding[1].binding = 0;

    VkDescriptorSetLayoutCreateInfo descriptor_layout = {};
    descriptor_layout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptor_layout.pNext = nullptr;
    descriptor_layout.bindingCount = 1;
    descriptor_layout.pBindings = &layout_binding[0];

    VkResult vkr = vkCreateDescriptorSetLayout( e->m_device, &descriptor_layout, nullptr, &e->m_descriptor_set_layout );
    vkassert( vkr );

    descriptor_layout.pBindings = &layout_binding[1];
    vkr = vkCreateDescriptorSetLayout( e->m_device, &descriptor_layout, nullptr, &e->m_descriptor_set_layout2 );
    vkassert( vkr );

    VkDescriptorSetLayout* dsl[2] = { &e->m_descriptor_set_layout, &e->m_descriptor_set_layout2 };

    VkPipelineLayoutCreateInfo pipeline_layout_create = {};
    pipeline_layout_create.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_create.pNext = nullptr;
    pipeline_layout_create.setLayoutCount = 2;
    pipeline_layout_create.pSetLayouts = dsl[0];

    vkr = vkCreatePipelineLayout( e->m_device, &pipeline_layout_create, nullptr, &e->m_pipeline_layout );
    vkassert( vkr );

    VkDescriptorPoolSize type_counts[1];
    type_counts[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    type_counts[0].descriptorCount = 2;

    VkDescriptorPoolCreateInfo descriptor_pool_info = {};
    descriptor_pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptor_pool_info.pNext = nullptr;
    descriptor_pool_info.poolSizeCount = 1;
    descriptor_pool_info.pPoolSizes = type_counts;
    descriptor_pool_info.maxSets = k_engine->get_total_drawables() + 16;

    vkr = vkCreateDescriptorPool( e->m_device, &descriptor_pool_info, nullptr, &e->m_descriptor_pool );
    vkassert( vkr );

  }

  void create_constant_buffer_object( engine_data* e, constant_buffer_data* cbd, constant_buffer cb ) {
    {
      VkDescriptorSetAllocateInfo alloc_info = {};
      alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
      alloc_info.descriptorPool = e->m_descriptor_pool;
      alloc_info.descriptorSetCount = 1;
      alloc_info.pSetLayouts = &e->m_descriptor_set_layout;

      VkResult vkr = vkAllocateDescriptorSets( e->m_device, &alloc_info, &cbd->m_descriptor_set );
      vkassert( vkr );
    }

    VkMemoryRequirements mem_reqs = {};
    VkMemoryAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.pNext = nullptr;
    alloc_info.allocationSize = 0;
    alloc_info.memoryTypeIndex = 0;

    VkBufferCreateInfo buffer_info = {};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = sizeof( constant_buffer );
    buffer_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

    VkResult vkr = vkCreateBuffer( e->m_device, &buffer_info, nullptr, &cbd->m_buffer );
    vkassert( vkr );

    vkGetBufferMemoryRequirements( e->m_device, cbd->m_buffer, &mem_reqs );
    alloc_info.allocationSize = mem_reqs.size;

    _get_memory_type( e, mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &alloc_info.memoryTypeIndex );

    vkr = vkAllocateMemory( e->m_device, &alloc_info, nullptr, &( cbd->m_memory ) );
    vkassert( vkr );

    vkr = vkBindBufferMemory( e->m_device, cbd->m_buffer, cbd->m_memory, 0 );
    vkassert( vkr );

    cbd->m_descriptor.buffer = cbd->m_buffer;
    cbd->m_descriptor.offset = 0;
    cbd->m_descriptor.range = sizeof( constant_buffer );

    VkWriteDescriptorSet write_descriptor_set = {};
    write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write_descriptor_set.dstSet = cbd->m_descriptor_set;
    write_descriptor_set.descriptorCount = 1;
    write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    write_descriptor_set.pBufferInfo = &cbd->m_descriptor;
    write_descriptor_set.dstBinding = 0;

    vkUpdateDescriptorSets( e->m_device, 1, &write_descriptor_set, 0, nullptr );
  }

  void update_constant_buffer_object( engine_data* e, constant_buffer_data* cbd, constant_buffer cb ) {
    uint8_t* data = nullptr;

    VkResult vkr = vkMapMemory( e->m_device, cbd->m_memory, 0, sizeof( constant_buffer ),
      0, ( void** ) &data );
    vkassert( vkr );

    memcpy( data, &cb, sizeof( constant_buffer ) );

    vkUnmapMemory( e->m_device, cbd->m_memory );
  }

  void create_instance_buffer_object( engine_data* e, renderer_data* r, instance_buffer* ub ) {

  }

  void create_instance_buffer_view( engine_data* e, renderer_data* r, drawable_data* d, uint64_t buffer_offset, int32_t cbv_offset ) {

  }

  void update_instance_buffer_object( engine_data* e, renderer_data* r, instance_buffer* ub ) {

  }

  void create_and_update_descriptor_sets( engine_data* e, renderer_data* r, std::vector<Drawable*>* draw ) {
    VkResult vkr;
    VkDescriptorSetAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = e->m_descriptor_pool;
    alloc_info.descriptorSetCount = 1;
    alloc_info.pSetLayouts = &e->m_descriptor_set_layout;

    for( int32_t i = 0; i < draw->size(); ++i ) {
      vkr = vkAllocateDescriptorSets( e->m_device, &alloc_info, &draw->at( i )->get_drawable_data()->m_descriptor_set );
      vkassert( vkr );
    }

    std::vector<VkWriteDescriptorSet> desc;

    for( int32_t i = 0; i < draw->size(); ++i ) {
      drawable_data* d = ( draw->at( i ) )->get_drawable_data();

      VkMemoryRequirements mem_reqs = {};
      VkMemoryAllocateInfo alloc_info = {};
      alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
      alloc_info.pNext = nullptr;
      alloc_info.allocationSize = 0;
      alloc_info.memoryTypeIndex = 0;

      VkBufferCreateInfo buffer_info = {};
      buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
      buffer_info.size = sizeof( instance_buffer );
      buffer_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

      vkr = vkCreateBuffer( e->m_device, &buffer_info, nullptr, &d->m_uniform_buffer_data.m_buffer );
      vkassert( vkr );

      vkGetBufferMemoryRequirements( e->m_device, d->m_uniform_buffer_data.m_buffer, &mem_reqs );
      alloc_info.allocationSize = mem_reqs.size;

      _get_memory_type( e, mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &alloc_info.memoryTypeIndex );

      vkr = vkAllocateMemory( e->m_device, &alloc_info, nullptr, &( d->m_uniform_buffer_data.m_memory ) );
      vkassert( vkr );

      vkr = vkBindBufferMemory( e->m_device, d->m_uniform_buffer_data.m_buffer, d->m_uniform_buffer_data.m_memory, 0 );
      vkassert( vkr );

      d->m_uniform_buffer_data.m_descriptor.buffer = d->m_uniform_buffer_data.m_buffer;
      d->m_uniform_buffer_data.m_descriptor.offset = 0;
      d->m_uniform_buffer_data.m_descriptor.range = sizeof( instance_buffer );

      uint8_t* data = nullptr;
      vkr = vkMapMemory( e->m_device, d->m_uniform_buffer_data.m_memory, 0, sizeof( instance_buffer ),
        0, ( void** ) &data );
      vkassert( vkr );

      memset( data, 0, sizeof( instance_buffer ) );
      memcpy( data, &d->m_uniform_buffer, sizeof( instance_buffer ) );

      vkUnmapMemory( e->m_device, d->m_uniform_buffer_data.m_memory );

      VkWriteDescriptorSet write_descriptor_set = {};
      write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      write_descriptor_set.dstSet = d->m_descriptor_set;
      write_descriptor_set.descriptorCount = 1;
      write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      write_descriptor_set.pBufferInfo = &d->m_uniform_buffer_data.m_descriptor;
      write_descriptor_set.dstBinding = 0;

      desc.push_back( write_descriptor_set );
    }

    vkUpdateDescriptorSets( e->m_device, ( uint32_t ) desc.size(), &desc[0], 0, nullptr );

  }
  //slow atm
  void update_decriptor_sets( engine_data* e, renderer_data* r, std::vector<Drawable*>* draw ) {
    for( int32_t i = 0; i < draw->size(); ++i ) {
      drawable_data* d = ( draw->at( i ) )->get_drawable_data();

      uint8_t* data = nullptr;
      VkResult vkr = vkMapMemory( e->m_device, d->m_uniform_buffer_data.m_memory, 0, sizeof( instance_buffer ),
        0, ( void** ) &data );
      vkassert( vkr );

      memset( data, 0, sizeof( instance_buffer ) );
      memcpy( data, &d->m_uniform_buffer, sizeof( instance_buffer ) );

      vkUnmapMemory( e->m_device, d->m_uniform_buffer_data.m_memory );

    }
  }
}

#endif