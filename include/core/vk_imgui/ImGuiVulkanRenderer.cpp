#include "core/core.hh"
#ifdef NOPE
#include "ImGuiVulkanRenderer.h"
#include "core/k_vulkan.hh"
#include "core/engine.hh"
#include "core/window.hh"
#include "core/input.hh"

namespace kretash {
  ImGuiVulkanRenderer::~ImGuiVulkanRenderer() {
    // Must wait to make sure that the objects can be safely destroyed

    VkDevice device = k_engine->get_engine_data()->m_device;

    // We need to check if these objects exist, or else we'll crash
    if( font_sampler ) {
      vkDestroySampler( device, font_sampler, nullptr );
    }

    if( font_memory ) {
      vkFreeMemory( device, font_memory, nullptr );
    }

    if( font_image_view ) {
      vkDestroyImageView( device, font_image_view, nullptr );
    }

    if( font_image ) {
      vkDestroyImage( device, font_image, nullptr );
    }

    if( pipeline ) {
      vkDestroyPipeline( device, pipeline, nullptr );
    }

    if( pipeline_cache ) {
      vkDestroyPipelineCache( device, pipeline_cache, nullptr );
    }

    if( pipeline_layout ) {
      vkDestroyPipelineLayout( device, pipeline_layout, nullptr );
    }

    if( descriptor_set_layout ) {
      vkDestroyDescriptorSetLayout( device, descriptor_set_layout, nullptr );
    }

    if( descriptor_set ) {
      vkFreeDescriptorSets( device, descriptor_pool, 1, &descriptor_set );
    }

    if( descriptor_pool ) {
      vkDestroyDescriptorPool( device, descriptor_pool, nullptr );
    }

    if( vertex_shader ) {
      vkDestroyShaderModule( device, vertex_shader, nullptr );
    }

    if( fragment_shader ) {
      vkDestroyShaderModule( device, fragment_shader, nullptr );
    }
  }

  VkShaderModule ImGuiVulkanRenderer::load_shader_GLSL( std::string file_name ) {
    std::ifstream stream( file_name, std::ios::binary );

    stream.seekg( 0, std::ios::end );
    uint64_t size = stream.tellg();
    stream.seekg( 0, std::ios::beg );

    char* shader_code = ( char* ) malloc( size );

    stream.read( shader_code, size );

    VkShaderModuleCreateInfo shader_module_info = {};
    shader_module_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shader_module_info.codeSize = size;
    shader_module_info.pCode = ( uint32_t* ) shader_code;

    VkShaderModule shader_module;
    VkResult vkr;

    VkDevice device = k_engine->get_engine_data()->m_device;

    vkr = vkCreateShaderModule( device, &shader_module_info, nullptr, &shader_module );
    vkassert( vkr );

    free( shader_code );

    return shader_module;
  }

  void ImGuiVulkanRenderer::new_frame() {
    ImGuiIO& io = ImGui::GetIO();

    Window* window = k_engine->get_window();
    Input* input = k_engine->get_input();

    io.DisplaySize.x = static_cast< float >( k_engine->get_window()->get_width() );
    io.DisplaySize.y = static_cast< float >( k_engine->get_window()->get_height() );
    io.DeltaTime = 1.0f;
    io.KeyCtrl = ( GetKeyState( VK_CONTROL ) & 0x8000 ) != 0;
    io.KeyShift = ( GetKeyState( VK_SHIFT ) & 0x8000 ) != 0;
    io.KeyAlt = ( GetKeyState( VK_MENU ) & 0x8000 ) != 0;
    io.MousePos = ImVec2( input->get_cursor().m_x, input->get_cursor().m_y );
    io.MouseDown[0] = input->get_key( k_LEFT_MOUSE_BTN );
    io.MouseDown[1] = input->get_key( k_MID_MOUSE_BTN );
    io.MouseDown[2] = input->get_key( k_RIGHT_MOUSE_BTN );
    SetCursor( io.MouseDrawCursor ? nullptr : LoadCursor( nullptr, IDC_ARROW ) );

    ImGui::NewFrame();
  }

  void ImGuiVulkanRenderer::imgui_render( ImDrawData* draw_data ) {
    ImGuiVulkanRenderer& renderer = *( ImGuiVulkanRenderer* ) ImGui::GetIO().UserData;
    ImGuiIO& io = ImGui::GetIO();

    uint32_t width = static_cast< uint32_t >( io.DisplaySize.x );
    uint32_t height = static_cast< uint32_t >( io.DisplaySize.y );
    draw_data->ScaleClipRects( io.DisplayFramebufferScale );

    VkResult vkr;

    VkSemaphoreCreateInfo semaphore_info = {};
    semaphore_info.pNext = nullptr;
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkSemaphore semaphore;

    engine_data* e = k_engine->get_engine_data();
    VkDevice device = k_engine->get_engine_data()->m_device;
    VkSwapchainKHR swapchain = k_engine->get_engine_data()->m_swap_chain.swapChain;

    vkr = vkCreateSemaphore( device, &semaphore_info, nullptr, &semaphore );
    vkassert( vkr );

    VkViewport viewport = {};
    viewport.width = io.DisplaySize.x;
    viewport.height = io.DisplaySize.y;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    int32_t cb = 0;
    vkCmdSetViewport( e->m_draw_command_buffers[cb], 0, 1, &viewport );

    // Projection matrix
    const float ortho_projection[4][4] =
    {
      { 2.0f / io.DisplaySize.x,  0.0f, 0.0f, 0.0f },
      { 0.0f, 2.0f / -io.DisplaySize.y, 0.0f, 0.0f },
      { 0.0f, 0.0f, -1.0f, 0.0f },
      { -1.0f, 1.0f,  0.0f, 1.0f },
    };

    vkCmdBindDescriptorSets( e->m_draw_command_buffers[cb], VK_PIPELINE_BIND_POINT_GRAPHICS, renderer.pipeline_layout, 0, 1, &renderer.descriptor_set, 0, nullptr );
    vkCmdPushConstants( e->m_draw_command_buffers[cb], renderer.pipeline_layout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof( float ) * 16, &ortho_projection );
    vkCmdBindPipeline( e->m_draw_command_buffers[cb], VK_PIPELINE_BIND_POINT_GRAPHICS, renderer.pipeline );

    std::vector<VkDeviceMemory> buffer_memory( draw_data->CmdListsCount );
    std::vector<VkBuffer> render_buffer( draw_data->CmdListsCount );

    for( int32_t i = 0; i < draw_data->CmdListsCount; i++ ) {
      ImDrawList* draw_list = draw_data->CmdLists[i];

      VkBufferCreateInfo render_buffer_info = {};
      render_buffer_info.pNext = nullptr;
      render_buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
      render_buffer_info.size = draw_list->IdxBuffer.size() * sizeof( ImDrawIdx ) + draw_list->VtxBuffer.size() * sizeof( ImDrawVert );
      render_buffer_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

      vkr = vkCreateBuffer( device, &render_buffer_info, nullptr, &render_buffer[i] );
      vkassert( vkr );

      VkMemoryRequirements buffer_memory_requirements;
      vkGetBufferMemoryRequirements( device, render_buffer[i], &buffer_memory_requirements );

      VkMemoryAllocateInfo memory_allocation_info = {};
      memory_allocation_info.pNext = nullptr;
      memory_allocation_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
      memory_allocation_info.allocationSize = buffer_memory_requirements.size;

      vk::_get_memory_type( e, buffer_memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &memory_allocation_info.memoryTypeIndex );

      vkr = vkAllocateMemory( device, &memory_allocation_info, nullptr, &buffer_memory[i] );
      vkassert( vkr );

      void* data;

      vkr = vkMapMemory( device, buffer_memory[i], 0, memory_allocation_info.allocationSize, 0, &data );
      vkassert( vkr );

      memcpy( data, &draw_list->VtxBuffer.front(), draw_list->VtxBuffer.size() * sizeof( ImDrawVert ) );
      memcpy( ( uint8_t* ) data + draw_list->VtxBuffer.size() * sizeof( ImDrawVert ), &draw_list->IdxBuffer.front(), draw_list->IdxBuffer.size() * sizeof( ImDrawIdx ) );

      vkUnmapMemory( device, buffer_memory[i] );

      vkr = vkBindBufferMemory( device, render_buffer[i], buffer_memory[i], 0 );
      vkassert( vkr );

      uint64_t offset = 0;
      vkCmdBindVertexBuffers( e->m_draw_command_buffers[cb], 0, 1, &render_buffer[i], &offset );
      vkCmdBindIndexBuffer( e->m_draw_command_buffers[cb], render_buffer[i], draw_list->VtxBuffer.size() * sizeof( ImDrawVert ), VK_INDEX_TYPE_UINT16 );

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

          vkCmdSetScissor( e->m_draw_command_buffers[cb], 0, 1, &scissor );
          vkCmdDrawIndexed( e->m_draw_command_buffers[cb], draw_cmd->ElemCount, 1, index_offset, 0, 0 );
        }

        index_offset += draw_cmd->ElemCount;
      }
    }

  }

  bool ImGuiVulkanRenderer::prepare_vulkan( uint8_t device_num ) {

    engine_data* e = k_engine->get_engine_data();
    VkInstance instance = k_engine->get_engine_data()->m_instance;
    VkDevice device = k_engine->get_engine_data()->m_device;


    // Set the ImGui size values
    ImGuiIO io = ImGui::GetIO();
    io.DisplaySize.x = static_cast< float >( k_engine->get_window()->get_width() );
    io.DisplaySize.y = static_cast< float >( k_engine->get_window()->get_height() );

    // Create a render pass
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

    VkResult vkr = vkCreateRenderPass( device, &render_pass_info, nullptr, &render_pass );
    vkassert( vkr );

    // Create a descriptor set layout
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

    vkr = vkCreateDescriptorSetLayout( device, &descriptor_set_layout_info, nullptr, &descriptor_set_layout );
    vkassert( vkr );

    // Create the pipeline layout
    VkPushConstantRange push_constant_range;
    push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    push_constant_range.size = sizeof( float ) * 16;

    VkPipelineLayoutCreateInfo pipeline_layout_info = {};
    pipeline_layout_info.pNext = nullptr;
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = 1;
    pipeline_layout_info.pSetLayouts = &descriptor_set_layout;
    pipeline_layout_info.pushConstantRangeCount = 1;
    pipeline_layout_info.pPushConstantRanges = &push_constant_range;

    vkr = vkCreatePipelineLayout( device, &pipeline_layout_info, nullptr, &pipeline_layout );
    vkassert( vkr );

    // Prepare vertex input bindings
    vertex_input_binding.binding = 0;
    vertex_input_binding.stride = sizeof( ImDrawVert );
    vertex_input_binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    // Prepare vertex input attributes
    vertex_input_attribute[0].location = 0;
    vertex_input_attribute[0].format = VK_FORMAT_R32G32_SFLOAT;
    vertex_input_attribute[0].offset = offsetof( ImDrawVert, pos );

    vertex_input_attribute[1].location = 1;
    vertex_input_attribute[1].format = VK_FORMAT_R32G32_SFLOAT;
    vertex_input_attribute[1].offset = offsetof( ImDrawVert, uv );

    vertex_input_attribute[2].location = 2;
    vertex_input_attribute[2].format = VK_FORMAT_R8G8B8A8_UNORM;
    vertex_input_attribute[2].offset = offsetof( ImDrawVert, col );

    // Create a rendering pipeline
    // Vertex input info
    VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
    vertex_input_info.pNext = nullptr;
    vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_info.vertexAttributeDescriptionCount = 3;
    vertex_input_info.pVertexAttributeDescriptions = vertex_input_attribute;
    vertex_input_info.vertexBindingDescriptionCount = 1;
    vertex_input_info.pVertexBindingDescriptions = &vertex_input_binding;

    // Vertex input assembly
    VkPipelineInputAssemblyStateCreateInfo input_assembly_info = {};
    input_assembly_info.pNext = nullptr;
    input_assembly_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    // Viewport info
    VkPipelineViewportStateCreateInfo viewport_info = {};
    viewport_info.pNext = nullptr;
    viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_info.viewportCount = 1;
    viewport_info.scissorCount = 1;

    // Rasterization info
    VkPipelineRasterizationStateCreateInfo rasterization_info = {};
    rasterization_info.pNext = nullptr;
    rasterization_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterization_info.polygonMode = VK_POLYGON_MODE_FILL;
    rasterization_info.cullMode = VK_CULL_MODE_NONE;
    rasterization_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterization_info.depthClampEnable = VK_FALSE;
    rasterization_info.rasterizerDiscardEnable = VK_FALSE;
    rasterization_info.depthBiasEnable = VK_FALSE;

    // Multisampling info
    VkPipelineMultisampleStateCreateInfo multisample_info = {};
    multisample_info.pNext = nullptr;
    multisample_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisample_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    // Depth stencil info
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

    // Color blending info
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

    // Dynamic states
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

    // Shaders
    vertex_shader = load_shader_GLSL( vertex_shader_path );
    fragment_shader = load_shader_GLSL( fragment_shader_path );

    VkPipelineShaderStageCreateInfo shader_info[2] = {};
    shader_info[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader_info[0].module = vertex_shader;
    shader_info[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shader_info[0].pName = "main";

    shader_info[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader_info[1].module = fragment_shader;
    shader_info[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shader_info[1].pName = "main";

    // Create the pipeline cache
    VkPipelineCacheCreateInfo pipeline_cache_info = {};
    pipeline_cache_info.pNext = nullptr;
    pipeline_cache_info.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

    vkr = vkCreatePipelineCache( device, &pipeline_cache_info, nullptr, &pipeline_cache );
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
    pipeline_info.renderPass = render_pass;
    pipeline_info.layout = pipeline_layout;

    vkr = vkCreateGraphicsPipelines( device, pipeline_cache, 1, &pipeline_info, nullptr, &pipeline );
    vkassert( vkr );

    // Setup a descriptor pool
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

    vkr = vkCreateDescriptorPool( device, &descriptor_pool_info, nullptr, &descriptor_pool );
    vkassert( vkr );

    // Setup up the descriptor set
    VkDescriptorSetAllocateInfo descriptor_set_info = {};
    descriptor_set_info.pNext = nullptr;
    descriptor_set_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptor_set_info.descriptorPool = descriptor_pool;
    descriptor_set_info.descriptorSetCount = 1;
    descriptor_set_info.pSetLayouts = &descriptor_set_layout;

    vkr = vkAllocateDescriptorSets( device, &descriptor_set_info, &descriptor_set );
    vkassert( vkr );

    uint8_t* pixels;
    int32_t width, height;

    io.Fonts->GetTexDataAsRGBA32( &pixels, &width, &height );

    // Prepare font texture
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

    vkr = vkCreateImage( device, &image_info, nullptr, &font_image );
    vkassert( vkr );

    VkMemoryRequirements memory_requirements;
    vkGetImageMemoryRequirements( device, font_image, &memory_requirements );

    VkMemoryAllocateInfo memory_allocation_info = {};
    memory_allocation_info.pNext = nullptr;
    memory_allocation_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memory_allocation_info.allocationSize = memory_requirements.size;

    vk::_get_memory_type( e, memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
      &memory_allocation_info.memoryTypeIndex );

    vkr = vkAllocateMemory( device, &memory_allocation_info, nullptr, &font_memory );
    vkassert( vkr );

    vkr = vkBindImageMemory( device, font_image, font_memory, 0 );
    vkassert( vkr );

    VkImageViewCreateInfo image_view_info;
    image_view_info.pNext = nullptr;
    image_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    image_view_info.image = font_image;
    image_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_view_info.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
    image_view_info.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

    vkr = vkCreateImageView( device, &image_view_info, nullptr, &font_image_view );
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

    vkr = vkCreateSampler( device, &sampler_info, nullptr, &font_sampler );
    vkassert( vkr );

    // Update descriptors
    VkDescriptorImageInfo descriptor_image_info = {};
    descriptor_image_info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    descriptor_image_info.sampler = font_sampler;
    descriptor_image_info.imageView = font_image_view;

    VkWriteDescriptorSet write_descriptor_set = {};
    write_descriptor_set.pNext = nullptr;
    write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write_descriptor_set.dstSet = descriptor_set;
    write_descriptor_set.descriptorCount = 1;
    write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write_descriptor_set.pImageInfo = &descriptor_image_info;

    vkUpdateDescriptorSets( device, 1, &write_descriptor_set, 0, nullptr );

    // Upload the image to the GPU
    VkImageSubresource image_subresource = {};
    image_subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    VkSubresourceLayout subresource_layout = {};
    vkGetImageSubresourceLayout( device, font_image, &image_subresource, &subresource_layout );

    void* data;

    vkr = vkMapMemory( device, font_memory, 0, subresource_layout.size, 0, &data );
    vkassert( vkr );

    memcpy( data, pixels, subresource_layout.size );

    vkUnmapMemory( device, font_memory );

    return true;
  }

  bool ImGuiVulkanRenderer::initialize( void* handle, void* instance ) {
    // Set some basic ImGui info
    ImGuiIO& io = ImGui::GetIO();
    io.ImeWindowHandle = handle;
    io.RenderDrawListsFn = imgui_render;
    io.UserData = this;

    // Set some internal values
    window_handle = handle;
    window_instance = instance;
    vertex_shader_path = "../assets/shaders/imgui.vert.spv";
    fragment_shader_path = "../assets/shaders/imgui.frag.spv";

    bool err = prepare_vulkan( 0 );
    assert( err );

    return true;
  }
}
#endif