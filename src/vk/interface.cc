#include "core/vk/interface.hh"
#include "core/vk/context.hh"
#include "core/engine.hh"
#include "core/window.hh"
#include "core/input.hh"

namespace kretash {

  /* Initialize the interface in Vulkan and D3D12 */
  void vkInterface::init( Window* w ) {

    ImGuiIO& io = ImGui::GetIO();
    io.ImeWindowHandle = w->get_window_handle();
    io.RenderDrawListsFn = _render;
    io.UserData = this;

    vkContext* m_context = dynamic_cast< vkContext* >( k_engine->get_context() );
    VkInstance m_instance = m_context->m_instance;
    VkDevice m_device = m_context->m_device;

    io.DisplaySize.x = static_cast< float >( k_engine->get_window()->get_width() );
    io.DisplaySize.y = static_cast< float >( k_engine->get_window()->get_height() );

    VkAttachmentDescription attachement_description = {};
    attachement_description.format = VK_FORMAT_R8G8B8A8_UNORM;
    attachement_description.samples = VK_SAMPLE_COUNT_1_BIT;
    attachement_description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachement_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachement_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachement_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachement_description.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachement_description.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference attachment_reference = {};
    attachment_reference.attachment = 0;
    attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass_description = {};
    subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass_description.colorAttachmentCount = 1;
    subpass_description.pColorAttachments = &attachment_reference;

    VkRenderPassCreateInfo render_pass_info = {};
    render_pass_info.pNext = nullptr;
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subpass_description;
    render_pass_info.attachmentCount = 1;
    render_pass_info.pAttachments = &attachement_description;

    VkResult vkr = vkCreateRenderPass( m_device, &render_pass_info, nullptr, &m_render_pass );
    vkassert( vkr );

    VkDescriptorSetLayoutBinding descriptor_set_layout_binding = {};
    descriptor_set_layout_binding.binding = 0;
    descriptor_set_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_set_layout_binding.descriptorCount = 1;
    descriptor_set_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo descriptor_set_layout_info = {};
    descriptor_set_layout_info.pNext = nullptr;
    descriptor_set_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptor_set_layout_info.bindingCount = 1;
    descriptor_set_layout_info.pBindings = &descriptor_set_layout_binding;

    vkr = vkCreateDescriptorSetLayout( m_device, &descriptor_set_layout_info, nullptr, &m_descriptor_set_layout );
    vkassert( vkr );

    VkPushConstantRange push_constant_range;
    push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    push_constant_range.size = sizeof( float ) * 16;

    VkPipelineLayoutCreateInfo pipeline_layout_info = {};
    pipeline_layout_info.pNext = nullptr;
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = 1;
    pipeline_layout_info.pSetLayouts = &m_descriptor_set_layout;
    pipeline_layout_info.pushConstantRangeCount = 1;
    pipeline_layout_info.pPushConstantRanges = &push_constant_range;

    vkr = vkCreatePipelineLayout( m_device, &pipeline_layout_info, nullptr, &m_pipeline_layout );
    vkassert( vkr );

    m_vertex_input_binding.binding = 0;
    m_vertex_input_binding.stride = sizeof( ImDrawVert );
    m_vertex_input_binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    m_vertex_input_attribute[0].location = 0;
    m_vertex_input_attribute[0].format = VK_FORMAT_R32G32_SFLOAT;
    m_vertex_input_attribute[0].offset = offsetof( ImDrawVert, pos );

    m_vertex_input_attribute[1].location = 1;
    m_vertex_input_attribute[1].format = VK_FORMAT_R32G32_SFLOAT;
    m_vertex_input_attribute[1].offset = offsetof( ImDrawVert, uv );

    m_vertex_input_attribute[2].location = 2;
    m_vertex_input_attribute[2].format = VK_FORMAT_R8G8B8A8_UNORM;
    m_vertex_input_attribute[2].offset = offsetof( ImDrawVert, col );

    VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
    vertex_input_info.pNext = nullptr;
    vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_info.vertexAttributeDescriptionCount = 3;
    vertex_input_info.pVertexAttributeDescriptions = m_vertex_input_attribute;
    vertex_input_info.vertexBindingDescriptionCount = 1;
    vertex_input_info.pVertexBindingDescriptions = &m_vertex_input_binding;

    VkPipelineInputAssemblyStateCreateInfo input_assembly_info = {};
    input_assembly_info.pNext = nullptr;
    input_assembly_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkPipelineViewportStateCreateInfo viewport_info = {};
    viewport_info.pNext = nullptr;
    viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_info.viewportCount = 1;
    viewport_info.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterization_info = {};
    rasterization_info.pNext = nullptr;
    rasterization_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterization_info.polygonMode = VK_POLYGON_MODE_FILL;
    rasterization_info.cullMode = VK_CULL_MODE_NONE;
    rasterization_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterization_info.depthClampEnable = VK_FALSE;
    rasterization_info.rasterizerDiscardEnable = VK_FALSE;
    rasterization_info.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisample_info = {};
    multisample_info.pNext = nullptr;
    multisample_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisample_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineDepthStencilStateCreateInfo depth_stencil_info = {};
    depth_stencil_info.pNext = nullptr;
    depth_stencil_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth_stencil_info.depthTestEnable = VK_TRUE;
    depth_stencil_info.depthWriteEnable = VK_TRUE;
    depth_stencil_info.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    depth_stencil_info.depthBoundsTestEnable = VK_FALSE;
    depth_stencil_info.back.failOp = VK_STENCIL_OP_KEEP;
    depth_stencil_info.back.passOp = VK_STENCIL_OP_KEEP;
    depth_stencil_info.back.compareOp = VK_COMPARE_OP_ALWAYS;
    depth_stencil_info.stencilTestEnable = VK_FALSE;
    depth_stencil_info.front = depth_stencil_info.back;

    VkPipelineColorBlendAttachmentState color_blend_attachment = {};
    color_blend_attachment.blendEnable = VK_TRUE;
    color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;
    color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
    color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    color_blend_attachment.colorWriteMask = 0xF;

    VkPipelineColorBlendStateCreateInfo color_blend_info = {};
    color_blend_info.pNext = nullptr;
    color_blend_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blend_info.attachmentCount = 1;
    color_blend_info.pAttachments = &color_blend_attachment;

    std::vector<VkDynamicState> dynamic_states =
    {
      VK_DYNAMIC_STATE_VIEWPORT,
      VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamic_info;
    dynamic_info.pNext = nullptr;
    dynamic_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_info.dynamicStateCount = ( uint32_t ) dynamic_states.size();
    dynamic_info.pDynamicStates = dynamic_states.data();

    m_vertex_shader = vkTools::loadShader( "../assets/shaders/imgui.vert.spv",
      m_context->m_device, VK_SHADER_STAGE_VERTEX_BIT );

    m_fragment_shader = vkTools::loadShader( "../assets/shaders/imgui.frag.spv",
      m_context->m_device, VK_SHADER_STAGE_FRAGMENT_BIT );

    VkPipelineShaderStageCreateInfo shader_info[2] = {};
    shader_info[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader_info[0].module = m_vertex_shader;
    shader_info[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shader_info[0].pName = "main";

    shader_info[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader_info[1].module = m_fragment_shader;
    shader_info[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shader_info[1].pName = "main";

    VkPipelineCacheCreateInfo pipeline_cache_info = {};
    pipeline_cache_info.pNext = nullptr;
    pipeline_cache_info.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

    vkr = vkCreatePipelineCache( m_device, &pipeline_cache_info, nullptr, &m_pipeline_cache );
    vkassert( vkr );

    VkGraphicsPipelineCreateInfo pipeline_info = {};
    pipeline_info.pNext = nullptr;
    pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_info.stageCount = 2;
    pipeline_info.pStages = shader_info;
    pipeline_info.pVertexInputState = &vertex_input_info;
    pipeline_info.pInputAssemblyState = &input_assembly_info;
    pipeline_info.pViewportState = &viewport_info;
    pipeline_info.pRasterizationState = &rasterization_info;
    pipeline_info.pMultisampleState = &multisample_info;
    pipeline_info.pDepthStencilState = &depth_stencil_info;
    pipeline_info.pColorBlendState = &color_blend_info;
    pipeline_info.pDynamicState = &dynamic_info;
    pipeline_info.renderPass = m_render_pass;
    pipeline_info.layout = m_pipeline_layout;

    vkr = vkCreateGraphicsPipelines( m_device, m_pipeline_cache, 1, &pipeline_info, nullptr, &m_pipeline );
    vkassert( vkr );

    VkDescriptorPoolSize descriptor_pool_size = {};
    descriptor_pool_size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_pool_size.descriptorCount = 1;

    VkDescriptorPoolCreateInfo descriptor_pool_info = {};
    descriptor_pool_info.pNext = nullptr;
    descriptor_pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptor_pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    descriptor_pool_info.poolSizeCount = 1;
    descriptor_pool_info.pPoolSizes = &descriptor_pool_size;
    descriptor_pool_info.maxSets = 1;

    vkr = vkCreateDescriptorPool( m_device, &descriptor_pool_info, nullptr, &m_descriptor_pool );
    vkassert( vkr );

    VkDescriptorSetAllocateInfo descriptor_set_info = {};
    descriptor_set_info.pNext = nullptr;
    descriptor_set_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptor_set_info.descriptorPool = m_descriptor_pool;
    descriptor_set_info.descriptorSetCount = 1;
    descriptor_set_info.pSetLayouts = &m_descriptor_set_layout;

    vkr = vkAllocateDescriptorSets( m_device, &descriptor_set_info, &m_descriptor_set );
    vkassert( vkr );

    uint8_t* pixels;
    int32_t width, height;

    io.Fonts->GetTexDataAsRGBA32( &pixels, &width, &height );

    VkImageCreateInfo image_info = {};
    image_info.pNext = nullptr;
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_info.extent = { ( uint32_t ) width, ( uint32_t ) height, 1 };
    image_info.mipLevels = 1;
    image_info.arrayLayers = 1;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.tiling = VK_IMAGE_TILING_LINEAR;
    image_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_info.initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    vkr = vkCreateImage( m_device, &image_info, nullptr, &m_font_image );
    vkassert( vkr );

    VkMemoryRequirements memory_requirements;
    vkGetImageMemoryRequirements( m_device, m_font_image, &memory_requirements );

    VkMemoryAllocateInfo memory_allocation_info = {};
    memory_allocation_info.pNext = nullptr;
    memory_allocation_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memory_allocation_info.allocationSize = memory_requirements.size;

    m_context->_get_memory_type( memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
      &memory_allocation_info.memoryTypeIndex );

    vkr = vkAllocateMemory( m_device, &memory_allocation_info, nullptr, &m_font_memory );
    vkassert( vkr );

    vkr = vkBindImageMemory( m_device, m_font_image, m_font_memory, 0 );
    vkassert( vkr );

    VkImageViewCreateInfo image_view_info;
    image_view_info.pNext = nullptr;
    image_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    image_view_info.image = m_font_image;
    image_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_view_info.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
    image_view_info.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

    vkr = vkCreateImageView( m_device, &image_view_info, nullptr, &m_font_image_view );
    vkassert( vkr );

    VkSamplerCreateInfo sampler_info;
    sampler_info.pNext = nullptr;
    sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_info.magFilter = VK_FILTER_NEAREST;
    sampler_info.minFilter = VK_FILTER_NEAREST;
    sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_info.anisotropyEnable = VK_FALSE;
    sampler_info.maxAnisotropy = 1.0f;
    sampler_info.compareOp = VK_COMPARE_OP_NEVER;
    sampler_info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    sampler_info.unnormalizedCoordinates = VK_FALSE;

    vkr = vkCreateSampler( m_device, &sampler_info, nullptr, &m_font_sampler );
    vkassert( vkr );

    VkDescriptorImageInfo descriptor_image_info = {};
    descriptor_image_info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    descriptor_image_info.sampler = m_font_sampler;
    descriptor_image_info.imageView = m_font_image_view;

    VkWriteDescriptorSet write_descriptor_set = {};
    write_descriptor_set.pNext = nullptr;
    write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write_descriptor_set.dstSet = m_descriptor_set;
    write_descriptor_set.descriptorCount = 1;
    write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write_descriptor_set.pImageInfo = &descriptor_image_info;

    vkUpdateDescriptorSets( m_device, 1, &write_descriptor_set, 0, nullptr );

    VkImageSubresource image_subresource = {};
    image_subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    VkSubresourceLayout subresource_layout = {};
    vkGetImageSubresourceLayout( m_device, m_font_image, &image_subresource, &subresource_layout );

    void* data = nullptr;

    vkr = vkMapMemory( m_device, m_font_memory, 0, subresource_layout.size, 0, &data );
    vkassert( vkr );

    memcpy( data, pixels, subresource_layout.size );

    vkUnmapMemory( m_device, m_font_memory );

    {
      VkBufferCreateInfo render_buffer_info = {};
      render_buffer_info.pNext = nullptr;
      render_buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
      render_buffer_info.size = 1024 * 1024 * 8;
      render_buffer_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

      vkr = vkCreateBuffer( m_device, &render_buffer_info, nullptr, &m_render_buffer );
      vkassert( vkr );

      VkMemoryRequirements buffer_memory_requirements;
      vkGetBufferMemoryRequirements( m_device, m_render_buffer, &buffer_memory_requirements );

      VkMemoryAllocateInfo memory_allocation_info = {};
      memory_allocation_info.pNext = nullptr;
      memory_allocation_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
      memory_allocation_info.allocationSize = buffer_memory_requirements.size;

      m_context->_get_memory_type( buffer_memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, 
        &memory_allocation_info.memoryTypeIndex );

      vkr = vkAllocateMemory( m_device, &memory_allocation_info, nullptr, &m_render_buffer_mem );
      vkassert( vkr );

      void* data = nullptr;

      vkr = vkMapMemory( m_device, m_render_buffer_mem, 0, memory_allocation_info.allocationSize, 0, &data );
      vkassert( vkr );

      memset( data, 0, 1024 * 1024 * 8 );

      vkUnmapMemory( m_device, m_render_buffer_mem );

      vkr = vkBindBufferMemory( m_device, m_render_buffer, m_render_buffer_mem, 0 );
      vkassert( vkr );
    }
  }

  /* Render the interface in Vulkan and D3D12 */
  void vkInterface::_render( ImDrawData* draw_data ) {
    vkInterface& renderer = *( vkInterface* ) ImGui::GetIO().UserData;
    ImGuiIO& io = ImGui::GetIO();

    uint32_t width = static_cast< uint32_t >( io.DisplaySize.x );
    uint32_t height = static_cast< uint32_t >( io.DisplaySize.y );
    draw_data->ScaleClipRects( io.DisplayFramebufferScale );

    VkResult vkr;

    vkContext* m_context = dynamic_cast< vkContext* >( k_engine->get_context() );
    VkDevice device = m_context->m_device;

    VkViewport viewport = {};
    viewport.width = io.DisplaySize.x;
    viewport.height = io.DisplaySize.y;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    int32_t cb = 0;
    vkCmdSetViewport( m_context->m_draw_command_buffers[cb], 0, 1, &viewport );

    const float ortho_projection[4][4] =
    {
      { 2.0f / io.DisplaySize.x,  0.0f, 0.0f, 0.0f },
      { 0.0f, 2.0f / -io.DisplaySize.y, 0.0f, 0.0f },
      { 0.0f, 0.0f, -1.0f, 0.0f },
      { -1.0f, 1.0f,  0.0f, 1.0f },
    };

    vkCmdBindDescriptorSets( m_context->m_draw_command_buffers[cb], VK_PIPELINE_BIND_POINT_GRAPHICS, 
      renderer.m_pipeline_layout, 0, 1, &renderer.m_descriptor_set, 0, nullptr );

    vkCmdPushConstants( m_context->m_draw_command_buffers[cb], renderer.m_pipeline_layout, 
      VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof( float ) * 16, &ortho_projection );

    vkCmdBindPipeline( m_context->m_draw_command_buffers[cb], VK_PIPELINE_BIND_POINT_GRAPHICS, renderer.m_pipeline );

    std::vector<uint64_t> v_offsets;
    std::vector<uint64_t> i_offsets;
    uint64_t offset = 0;

    uint8_t* data = nullptr;
    vkr = vkMapMemory( device, renderer.m_render_buffer_mem, 0, 1024 * 1024 * 8, 0, ( void** ) &data );
    vkassert( vkr );

    for( int32_t i = 0; i < draw_data->CmdListsCount; i++ ) {
      ImDrawList* draw_list = draw_data->CmdLists[i];

      uint64_t v_size = draw_list->VtxBuffer.size() * sizeof( ImDrawVert );
      memcpy( data + offset, &draw_list->VtxBuffer.front(), v_size );
      v_offsets.push_back( offset );

      offset += v_size;
      uint64_t i_size = draw_list->IdxBuffer.size() * sizeof( ImDrawIdx );

      memcpy( data + offset, &draw_list->IdxBuffer.front(), i_size );
      i_offsets.push_back( offset );
      offset += i_size;

    }

    vkUnmapMemory( device, renderer.m_render_buffer_mem );

    for( int32_t i = 0; i < draw_data->CmdListsCount; i++ ) {
      ImDrawList* draw_list = draw_data->CmdLists[i];

      vkCmdBindVertexBuffers( m_context->m_draw_command_buffers[cb], 0, 1, &renderer.m_render_buffer, &v_offsets[i] );
      vkCmdBindIndexBuffer( m_context->m_draw_command_buffers[cb], renderer.m_render_buffer, i_offsets[i], VK_INDEX_TYPE_UINT16 );

      uint32_t index_offset = 0;

      for( int32_t j = 0; j < draw_list->CmdBuffer.size(); j++ ) {
        ImDrawCmd* draw_cmd = &draw_list->CmdBuffer[j];

        if( draw_cmd->UserCallback ) {
          draw_cmd->UserCallback( draw_list, draw_cmd );
        } else {
          VkRect2D scissor;
          scissor.offset.x = static_cast< int32_t >( draw_cmd->ClipRect.x );
          scissor.offset.y = static_cast< int32_t >( draw_cmd->ClipRect.y );
          scissor.extent.width = static_cast< int32_t >( draw_cmd->ClipRect.z - draw_cmd->ClipRect.x );
          scissor.extent.height = static_cast< int32_t >( draw_cmd->ClipRect.w - draw_cmd->ClipRect.y );

          vkCmdSetScissor( m_context->m_draw_command_buffers[cb], 0, 1, &scissor );
          vkCmdDrawIndexed( m_context->m_draw_command_buffers[cb], draw_cmd->ElemCount, 1, index_offset, 0, 0 );
        }

        index_offset += draw_cmd->ElemCount;
      }

    }
  }

  vkInterface::vkInterface() {

    vkContext* m_context = dynamic_cast< vkContext* >( k_engine->get_context() );

    if( m_font_sampler != VK_NULL_HANDLE ) {
      vkDestroySampler( m_context->m_device, m_font_sampler, nullptr );
      m_font_sampler = VK_NULL_HANDLE;
    }

    if( m_font_memory != VK_NULL_HANDLE ) {
      vkFreeMemory( m_context->m_device, m_font_memory, nullptr );
      m_font_memory = VK_NULL_HANDLE;
    }

    if( m_render_buffer != VK_NULL_HANDLE ) {
      vkDestroyBuffer( m_context->m_device, m_render_buffer, nullptr );
      m_render_buffer = VK_NULL_HANDLE;
    }

    if( m_render_buffer_mem != VK_NULL_HANDLE ) {
      vkFreeMemory( m_context->m_device, m_render_buffer_mem, nullptr );
      m_render_buffer_mem = VK_NULL_HANDLE;
    }

    if( m_font_image_view != VK_NULL_HANDLE ) {
      vkDestroyImageView( m_context->m_device, m_font_image_view, nullptr );
      m_font_image_view = VK_NULL_HANDLE;
    }

    if( m_font_image != VK_NULL_HANDLE ) {
      vkDestroyImage( m_context->m_device, m_font_image, nullptr );
      m_font_image = VK_NULL_HANDLE;
    }

    if( m_pipeline != VK_NULL_HANDLE ) {
      vkDestroyPipeline( m_context->m_device, m_pipeline, nullptr );
      m_pipeline = VK_NULL_HANDLE;
    }

    if( m_pipeline_cache != VK_NULL_HANDLE ) {
      vkDestroyPipelineCache( m_context->m_device, m_pipeline_cache, nullptr );
      m_pipeline_cache = VK_NULL_HANDLE;
    }

    if( m_pipeline_layout != VK_NULL_HANDLE ) {
      vkDestroyPipelineLayout( m_context->m_device, m_pipeline_layout, nullptr );
      m_pipeline_layout = VK_NULL_HANDLE;
    }

    if( m_descriptor_set_layout != VK_NULL_HANDLE ) {
      vkDestroyDescriptorSetLayout( m_context->m_device, m_descriptor_set_layout, nullptr );
      m_descriptor_set_layout = VK_NULL_HANDLE;
    }

    if( m_descriptor_set != VK_NULL_HANDLE ) {
      vkFreeDescriptorSets( m_context->m_device, m_descriptor_pool, 1, &m_descriptor_set );
      m_descriptor_set = VK_NULL_HANDLE;
    }

    if( m_descriptor_pool != VK_NULL_HANDLE ) {
      vkDestroyDescriptorPool( m_context->m_device, m_descriptor_pool, nullptr );
      m_descriptor_pool = VK_NULL_HANDLE;
    }

    if( m_vertex_shader != VK_NULL_HANDLE ) {
      vkDestroyShaderModule( m_context->m_device, m_vertex_shader, nullptr );
      m_vertex_shader = VK_NULL_HANDLE;
    }

    if( m_fragment_shader != VK_NULL_HANDLE ) {
      vkDestroyShaderModule( m_context->m_device, m_fragment_shader, nullptr );
      m_fragment_shader = VK_NULL_HANDLE;
    }

  }

  vkInterface::~vkInterface() {

    vkContext* m_context = dynamic_cast< vkContext* >( k_engine->get_context() );

    if( m_font_sampler != VK_NULL_HANDLE ) {
      vkDestroySampler( m_context->m_device, m_font_sampler, nullptr );
      m_font_sampler = VK_NULL_HANDLE;
    }

    if( m_font_memory != VK_NULL_HANDLE ) {
      vkFreeMemory( m_context->m_device, m_font_memory, nullptr );
      m_font_memory = VK_NULL_HANDLE;
    }

    if( m_render_buffer != VK_NULL_HANDLE ) {
      vkDestroyBuffer( m_context->m_device, m_render_buffer, nullptr );
      m_render_buffer = VK_NULL_HANDLE;
    }

    if( m_render_buffer_mem != VK_NULL_HANDLE ) {
      vkFreeMemory( m_context->m_device, m_render_buffer_mem, nullptr );
      m_render_buffer_mem = VK_NULL_HANDLE;
    }

    if( m_font_image_view != VK_NULL_HANDLE ) {
      vkDestroyImageView( m_context->m_device, m_font_image_view, nullptr );
      m_font_image_view = VK_NULL_HANDLE;
    }

    if( m_font_image != VK_NULL_HANDLE ) {
      vkDestroyImage( m_context->m_device, m_font_image, nullptr );
      m_font_image = VK_NULL_HANDLE;
    }

    if( m_pipeline != VK_NULL_HANDLE ) {
      vkDestroyPipeline( m_context->m_device, m_pipeline, nullptr );
      m_pipeline = VK_NULL_HANDLE;
    }

    if( m_pipeline_cache != VK_NULL_HANDLE ) {
      vkDestroyPipelineCache( m_context->m_device, m_pipeline_cache, nullptr );
      m_pipeline_cache = VK_NULL_HANDLE;
    }

    if( m_pipeline_layout != VK_NULL_HANDLE ) {
      vkDestroyPipelineLayout( m_context->m_device, m_pipeline_layout, nullptr );
      m_pipeline_layout = VK_NULL_HANDLE;
    }

    if( m_descriptor_set_layout != VK_NULL_HANDLE ) {
      vkDestroyDescriptorSetLayout( m_context->m_device, m_descriptor_set_layout, nullptr );
      m_descriptor_set_layout = VK_NULL_HANDLE;
    }

    if( m_descriptor_set != VK_NULL_HANDLE ) {
      vkFreeDescriptorSets( m_context->m_device, m_descriptor_pool, 1, &m_descriptor_set );
      m_descriptor_set = VK_NULL_HANDLE;
    }

    if( m_descriptor_pool != VK_NULL_HANDLE ) {
      vkDestroyDescriptorPool( m_context->m_device, m_descriptor_pool, nullptr );
      m_descriptor_pool = VK_NULL_HANDLE;
    }

    if( m_vertex_shader != VK_NULL_HANDLE ) {
      vkDestroyShaderModule( m_context->m_device, m_vertex_shader, nullptr );
      m_vertex_shader = VK_NULL_HANDLE;
    }

    if( m_fragment_shader != VK_NULL_HANDLE ) {
      vkDestroyShaderModule( m_context->m_device, m_fragment_shader, nullptr );
      m_fragment_shader = VK_NULL_HANDLE;
    }

    if( m_render_pass != VK_NULL_HANDLE ) {
      vkDestroyRenderPass( m_context->m_device, m_render_pass, nullptr );
      m_render_pass = VK_NULL_HANDLE;
    }

  }

}