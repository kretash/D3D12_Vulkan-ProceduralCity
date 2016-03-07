#include "core/k_vulkan.hh"

#ifdef __VULKAN__

#include "vulkan/vulkan.h"
#include "vulkan/vulkantools.h"
#include "core/GPU_pool.hh"
#include "core/types.hh"
#include "core/engine.hh"
#include "core/vk_pool.hh"

namespace vk {

#define USE_POOL 1
  const VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;

  void reset_texture_command_list( engine_data* e ) {

    VkResult vkr = vkResetCommandBuffer( e->m_texture_command_buffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT );
    vkassert( vkr );

    VkCommandBufferBeginInfo command_buffer_begin_info = {};
    command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    vkr = vkBeginCommandBuffer( e->m_texture_command_buffer, &command_buffer_begin_info );
    vkassert( vkr );
  }

  void create_texture( engine_data* e, texture_data* t, void* data, int32_t width, int32_t height, int32_t channels ) {
    VkResult vkr = {};

    t->m_height = height;
    t->m_width = width;
    t->m_mip_levels = 1;


    // mappable image // buffer
    VkImageCreateInfo image_create_info = {};
    image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.pNext = nullptr;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = format;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_LINEAR;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_create_info.flags = 0;
    image_create_info.extent = { t->m_width, t->m_height, 1 };

    vkr = vkCreateImage( e->m_device, &image_create_info, nullptr, &t->m_mappable_image );
    vkassert( vkr );
    VKPool* h_pool = k_engine->get_vk_host_pool();

    VkMemoryAllocateInfo mem_alloc = {};
    mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mem_alloc.pNext = nullptr;
    mem_alloc.allocationSize = 0;
    mem_alloc.memoryTypeIndex = 0;

    VkMemoryRequirements mem_req = {};
    vkGetImageMemoryRequirements( e->m_device, t->m_mappable_image, &mem_req );
    _get_memory_type( e, mem_req.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &mem_alloc.memoryTypeIndex );

    t->m_mappable_mem_block = h_pool->get_mem( mem_req.size );//careful here

    vkr = vkBindImageMemory( e->m_device, t->m_mappable_image, e->m_host_pool_memory, t->m_mappable_mem_block.m_start );
    vkassert( vkr );

    VkImageSubresource sub_resource = {};
    sub_resource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    VkSubresourceLayout sub_res_layout = {};
    void* v_data = nullptr;
    vkGetImageSubresourceLayout( e->m_device, t->m_mappable_image, &sub_resource, &sub_res_layout );

    vkr = vkMapMemory( e->m_device, e->m_host_pool_memory, t->m_mappable_mem_block.m_start, mem_req.size, 0, &v_data );
    vkassert( vkr );

    memcpy( v_data, data, t->m_width*t->m_height*sizeof( int32_t ) );

    vkUnmapMemory( e->m_device, e->m_host_pool_memory );

    vkTools::setImageLayout( e->m_texture_command_buffer, t->m_mappable_image,
      VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL );

    // actual image
    mem_req = {};

    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    image_create_info.mipLevels = t->m_mip_levels;
    image_create_info.extent = { t->m_width, t->m_height, 1 };

    vkr = vkCreateImage( e->m_device, &image_create_info, nullptr, &t->m_image );
    vkassert( vkr );

    vkGetImageMemoryRequirements( e->m_device, t->m_image, &mem_req );
    mem_alloc.allocationSize = mem_req.size;

    VKPool* pool = k_engine->get_vk_device_pool();
    t->m_image_mem_block = pool->get_mem( mem_req.size );

    vkr = vkBindImageMemory( e->m_device, t->m_image, e->m_device_pool_memory, ( uint64_t ) t->m_image_mem_block.m_start );
    vkassert( vkr );

    vkTools::setImageLayout( e->m_texture_command_buffer, t->m_image,
      VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL );

    VkImageCopy image_copy = {};
    image_copy.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    image_copy.srcSubresource.baseArrayLayer = 0;
    image_copy.srcSubresource.mipLevel = 0;
    image_copy.srcSubresource.layerCount = 1;
    image_copy.srcOffset = { 0,0,0 };

    image_copy.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    image_copy.dstSubresource.baseArrayLayer = 0;
    image_copy.dstSubresource.mipLevel = 0;
    image_copy.dstSubresource.layerCount = 1;
    image_copy.dstOffset = { 0,0,0 };

    image_copy.extent.width = t->m_width;
    image_copy.extent.height = t->m_height;
    image_copy.extent.depth = 1;

    vkCmdCopyImage( e->m_texture_command_buffer,
      t->m_mappable_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
      t->m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &image_copy );

    t->m_image_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    vkTools::setImageLayout( e->m_texture_command_buffer, t->m_image,
      VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, t->m_image_layout );

  }

  void create_shader_resource_view( engine_data* e, renderer_data* r, texture_data* t, int32_t offset ) {

    VkSamplerCreateInfo sampler = {};
    sampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler.pNext = nullptr;
    sampler.magFilter = VK_FILTER_LINEAR;
    sampler.minFilter = VK_FILTER_LINEAR;
    sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler.mipLodBias = 0.0f;
    sampler.compareOp = VK_COMPARE_OP_NEVER;
    sampler.minLod = 0.0f;
    sampler.maxLod = 0.0f;
    sampler.maxAnisotropy = 8;
    sampler.anisotropyEnable = VK_TRUE;
    sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

    VkResult vkr = vkCreateSampler( e->m_device, &sampler, nullptr, &t->m_sampler );
    vkassert( vkr );

    VkImageViewCreateInfo view = {};
    view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view.pNext = nullptr;
    view.flags = 0;
    view.image = t->m_image;
    view.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view.format = format;
    view.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
    view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    view.subresourceRange.baseMipLevel = 0;
    view.subresourceRange.baseArrayLayer = 0;
    view.subresourceRange.layerCount = 1;
    view.subresourceRange.levelCount = 1;

    vkr = vkCreateImageView( e->m_device, &view, nullptr, &t->m_view );
    vkassert( vkr );

    VkDescriptorImageInfo image_descriptor = vkTools::initializers::descriptorImageInfo(
      t->m_sampler, t->m_view, VK_IMAGE_LAYOUT_GENERAL );

    VkWriteDescriptorSet write_descriptor_set_image = {};
    write_descriptor_set_image.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write_descriptor_set_image.dstSet = e->m_constant_descriptor_set;
    write_descriptor_set_image.descriptorCount = 1;
    write_descriptor_set_image.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write_descriptor_set_image.pImageInfo = &image_descriptor;
    write_descriptor_set_image.dstBinding = 1;
    write_descriptor_set_image.dstArrayElement = offset;

    vkUpdateDescriptorSets( e->m_device, 1, &write_descriptor_set_image, 0, nullptr );

  }

  void compute_texture_upload( engine_data* e ) {

    VkResult vkr = vkEndCommandBuffer( e->m_texture_command_buffer );
    vkassert( vkr );

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &e->m_texture_command_buffer;

    vkr = vkQueueSubmit( e->m_texture_queue, 1, &submit_info, VK_NULL_HANDLE );
    vkassert( vkr );

  }

  void wait_for_texture_upload( engine_data* e ) {

    VkResult vkr = vkQueueWaitIdle( e->m_texture_queue );
    vkassert( vkr );

  }

  void clear_texture_upload( engine_data* e, texture_data* t ) {

    if( t->m_mappable_image != VK_NULL_HANDLE ) {
      vkDestroyImage( e->m_device, t->m_mappable_image, nullptr );
      t->m_mappable_image = VK_NULL_HANDLE;
    }

    k_engine->get_vk_host_pool()->release( t->m_mappable_mem_block );
  }

  void clear_texture( engine_data* e, texture_data* t ) {
    if( t->m_view != VK_NULL_HANDLE ) {

      vkDestroyImageView( e->m_device, t->m_view, nullptr );
      vkDestroySampler( e->m_device, t->m_sampler, nullptr );
      vkDestroyImage( e->m_device, t->m_image, nullptr );
      k_engine->get_vk_device_pool()->release( t->m_image_mem_block );

      t->m_view = VK_NULL_HANDLE;
      t->m_sampler = VK_NULL_HANDLE;
      t->m_image = VK_NULL_HANDLE;

    }
  }

  void clear_descriptor_set( engine_data* e, texture_data* pt, uint32_t offset ) {

    VkDescriptorImageInfo image_descriptor = vkTools::initializers::descriptorImageInfo(
      pt->m_sampler, pt->m_view, VK_IMAGE_LAYOUT_GENERAL );

    VkWriteDescriptorSet write_descriptor_set_image = {};
    write_descriptor_set_image.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write_descriptor_set_image.dstSet = e->m_constant_descriptor_set;
    write_descriptor_set_image.descriptorCount = 1;
    write_descriptor_set_image.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write_descriptor_set_image.pImageInfo = &image_descriptor;
    write_descriptor_set_image.dstBinding = 1;
    write_descriptor_set_image.dstArrayElement = offset;

    vkUpdateDescriptorSets( e->m_device, 1, &write_descriptor_set_image, 0, nullptr );
  }

}

#endif