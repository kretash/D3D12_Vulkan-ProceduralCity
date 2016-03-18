#include "core/vk/texture.hh"
#include "vulkan/vulkantools.h"
#include "core/vk/context.hh"
#include "core/engine.hh"
#include "core/pool.hh"

namespace kretash {

  //TODO
  const VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;

  /* Creates a texture in Vulkan and D3D12 */
  void vkTexture::create_texture( void* data, int32_t width, int32_t height, int32_t channels ) {

    VkResult vkr = {};
    vkContext* m_context = dynamic_cast<vkContext*>( k_engine->get_context() );

    m_height = height;
    m_width = width;
    m_mip_levels = 1;

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
    image_create_info.extent = { m_width, m_height, 1 };

    vkr = vkCreateImage( m_context->m_device, &image_create_info, nullptr, &m_mappable_image );
    vkassert( vkr );

    VkMemoryAllocateInfo mem_alloc = {};
    mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mem_alloc.pNext = nullptr;
    mem_alloc.allocationSize = 0;
    mem_alloc.memoryTypeIndex = 0;

    VkMemoryRequirements mem_req = {};
    vkGetImageMemoryRequirements( m_context->m_device, m_mappable_image, &mem_req );
    m_context->_get_memory_type( mem_req.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &mem_alloc.memoryTypeIndex );

    m_mappable_mem_block = m_context->m_vk_host_pool->get_mem( mem_req.size );

    vkr = vkBindImageMemory( m_context->m_device, m_mappable_image, m_context->m_host_pool_memory, 
      m_mappable_mem_block.m_start );
    vkassert( vkr );

    VkImageSubresource sub_resource = {};
    sub_resource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    VkSubresourceLayout sub_res_layout = {};
    void* v_data = nullptr;
    vkGetImageSubresourceLayout( m_context->m_device, m_mappable_image, &sub_resource, &sub_res_layout );

    vkr = vkMapMemory( m_context->m_device, m_context->m_host_pool_memory, m_mappable_mem_block.m_start,
      mem_req.size, 0, &v_data );
    vkassert( vkr );

    memcpy( v_data, data, m_width*m_height*sizeof( int32_t ) );

    vkUnmapMemory( m_context->m_device, m_context->m_host_pool_memory );

    vkTools::setImageLayout( m_context->m_texture_command_buffer, m_mappable_image,
      VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL );

    // actual image
    mem_req = {};

    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    image_create_info.mipLevels = m_mip_levels;
    image_create_info.extent = { m_width, m_height, 1 };

    vkr = vkCreateImage( m_context->m_device, &image_create_info, nullptr, &m_image );
    vkassert( vkr );

    vkGetImageMemoryRequirements( m_context->m_device, m_image, &mem_req );
    mem_alloc.allocationSize = mem_req.size;

    m_image_mem_block = m_context->m_vk_device_pool->get_mem( mem_req.size );

    vkr = vkBindImageMemory( m_context->m_device, m_image, m_context->m_device_pool_memory, m_image_mem_block.m_start );
    vkassert( vkr );

    vkTools::setImageLayout( m_context->m_texture_command_buffer, m_image,
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

    image_copy.extent.width = m_width;
    image_copy.extent.height = m_height;
    image_copy.extent.depth = 1;

    vkCmdCopyImage( m_context->m_texture_command_buffer,
      m_mappable_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
      m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &image_copy );

    m_image_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    vkTools::setImageLayout( m_context->m_texture_command_buffer, m_image,
      VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, m_image_layout );
  }

  /* Creates a texture view in Vulkan and D3D12 */
  void vkTexture::create_shader_resource_view( xxRenderer* r, int32_t offset ) {

    vkContext* m_context = dynamic_cast<vkContext*>( k_engine->get_context() );

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
    sampler.maxAnisotropy = 16;
    sampler.anisotropyEnable = VK_TRUE;
    sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

    VkResult vkr = vkCreateSampler( m_context->m_device, &sampler, nullptr, &m_sampler );
    vkassert( vkr );

    VkImageViewCreateInfo view = {};
    view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view.pNext = nullptr;
    view.flags = 0;
    view.image = m_image;
    view.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view.format = format;
    view.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
    view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    view.subresourceRange.baseMipLevel = 0;
    view.subresourceRange.baseArrayLayer = 0;
    view.subresourceRange.layerCount = 1;
    view.subresourceRange.levelCount = 1;

    vkr = vkCreateImageView( m_context->m_device, &view, nullptr, &m_view );
    vkassert( vkr );

    VkDescriptorImageInfo image_descriptor = vkTools::initializers::descriptorImageInfo(
      m_sampler, m_view, VK_IMAGE_LAYOUT_GENERAL );

    VkWriteDescriptorSet write_descriptor_set_image = {};
    write_descriptor_set_image.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write_descriptor_set_image.dstSet = m_context->m_constant_descriptor_set;
    write_descriptor_set_image.descriptorCount = 1;
    write_descriptor_set_image.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write_descriptor_set_image.pImageInfo = &image_descriptor;
    write_descriptor_set_image.dstBinding = 1;
    write_descriptor_set_image.dstArrayElement = offset;

    vkUpdateDescriptorSets( m_context->m_device, 1, &write_descriptor_set_image, 0, nullptr );
  }

  /* Clears the view resources in Vulkan and D3D12 */
  void vkTexture::clear_texture_upload() {

    vkContext* m_context = dynamic_cast<vkContext*>( k_engine->get_context() );

    if( m_mappable_image != VK_NULL_HANDLE ) {
      vkDestroyImage( m_context->m_device, m_mappable_image, nullptr );
      m_mappable_image = VK_NULL_HANDLE;
      m_context->m_vk_host_pool->release( m_mappable_mem_block );
    }


  }

  /* Clears the whole texture in Vulkan and D3D12 */
  void vkTexture::clear_texture() {

    vkContext* m_context = dynamic_cast<vkContext*>( k_engine->get_context() );

    if( m_view != VK_NULL_HANDLE ) {

      vkDestroyImageView( m_context->m_device, m_view, nullptr );
      vkDestroySampler( m_context->m_device, m_sampler, nullptr );
      vkDestroyImage( m_context->m_device, m_image, nullptr );
      m_context->m_vk_device_pool->release( m_image_mem_block );

      m_view = VK_NULL_HANDLE;
      m_sampler = VK_NULL_HANDLE;
      m_image = VK_NULL_HANDLE;

    }

  }

  /* Removes the texture from the descriptor set in Vulkan and D3D12 */
  void vkTexture::clear_descriptor_set( xxTexture* other, int32_t offset ) {

    vkContext* m_context = dynamic_cast<vkContext*>( k_engine->get_context() );
    vkTexture* t = dynamic_cast<vkTexture*>( other );

    VkDescriptorImageInfo image_descriptor = vkTools::initializers::descriptorImageInfo(
      t->m_sampler, t->m_view, VK_IMAGE_LAYOUT_GENERAL );

    VkWriteDescriptorSet write_descriptor_set_image = {};
    write_descriptor_set_image.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write_descriptor_set_image.dstSet = m_context->m_constant_descriptor_set;
    write_descriptor_set_image.descriptorCount = 1;
    write_descriptor_set_image.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write_descriptor_set_image.pImageInfo = &image_descriptor;
    write_descriptor_set_image.dstBinding = 1;
    write_descriptor_set_image.dstArrayElement = offset;

    vkUpdateDescriptorSets( m_context->m_device, 1, &write_descriptor_set_image, 0, nullptr );

  }


  vkTexture::vkTexture() {
    m_image_layout = VK_IMAGE_LAYOUT_UNDEFINED;
    m_mappable_mem_block = {};
    m_image_mem_block = {};
    m_width = 0;
    m_height = 0;
    m_mip_levels = 0;
    
    if( m_sampler != VK_NULL_HANDLE ) {
      vkContext* m_context = dynamic_cast<vkContext*>( k_engine->get_context() );
      vkDestroySampler( m_context->m_device, m_sampler, nullptr );
      m_sampler = VK_NULL_HANDLE;
    }

    if( m_image != VK_NULL_HANDLE ) {
      vkContext* m_context = dynamic_cast<vkContext*>( k_engine->get_context() );
      vkDestroyImage( m_context->m_device, m_image, nullptr );
      m_image = VK_NULL_HANDLE;
    }

    if( m_mappable_image != VK_NULL_HANDLE ) {
      vkContext* m_context = dynamic_cast<vkContext*>( k_engine->get_context() );
      vkDestroyImage( m_context->m_device, m_mappable_image, nullptr );
      m_mappable_image = VK_NULL_HANDLE;
    }

    if( m_view != VK_NULL_HANDLE ) {
      vkContext* m_context = dynamic_cast<vkContext*>( k_engine->get_context() );
      vkDestroyImageView( m_context->m_device, m_view, nullptr );
      m_view = VK_NULL_HANDLE;
    }
    
  }
  vkTexture::~vkTexture() {

    if( m_sampler != VK_NULL_HANDLE ) {
      vkContext* m_context = dynamic_cast<vkContext*>( k_engine->get_context() );
      vkDestroySampler( m_context->m_device, m_sampler, nullptr );
      m_sampler = VK_NULL_HANDLE;
    }

    if( m_image != VK_NULL_HANDLE ) {
      vkContext* m_context = dynamic_cast<vkContext*>( k_engine->get_context() );
      vkDestroyImage( m_context->m_device, m_image, nullptr );
      m_image = VK_NULL_HANDLE;
    }

    if( m_mappable_image != VK_NULL_HANDLE ) {
      vkContext* m_context = dynamic_cast<vkContext*>( k_engine->get_context() );
      vkDestroyImage( m_context->m_device, m_mappable_image, nullptr );
      m_mappable_image = VK_NULL_HANDLE;
    }

    if( m_view != VK_NULL_HANDLE ) {
      vkContext* m_context = dynamic_cast<vkContext*>( k_engine->get_context() );
      vkDestroyImageView( m_context->m_device, m_view, nullptr );
      m_view = VK_NULL_HANDLE;
    }

  }

}