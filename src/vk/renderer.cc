#include <memory>
#include "vulkan/vulkandebug.h"
#include "core/vk/context.hh"
#include "core/vk/renderer.hh"
#include "core/engine.hh"
#include "core/drawable.hh"
#include "core/engine_settings.hh"

namespace kretash {

  vkRenderer::vkRenderer() {

    if( m_pipeline != VK_NULL_HANDLE ) {
      vkContext* m_context = dynamic_cast< vkContext* >( k_engine->get_context() );
      assert( m_context->m_device != VK_NULL_HANDLE && "BAD REFERENCE" );
      vkDestroyPipeline( m_context->m_device, m_pipeline, nullptr );
      m_pipeline = VK_NULL_HANDLE;
    }

    m_binding_descriptions.clear();
    m_binding_descriptions.shrink_to_fit();
    m_attribute_descriptions.clear();
    m_attribute_descriptions.shrink_to_fit();
  }
  vkRenderer::~vkRenderer() {

    if( m_pipeline != VK_NULL_HANDLE ) {
      vkContext* m_context = dynamic_cast< vkContext* >( k_engine->get_context() );
      assert( m_context->m_device != VK_NULL_HANDLE && "BAD REFERENCE" );
      vkDestroyPipeline( m_context->m_device, m_pipeline, nullptr );
      m_pipeline = VK_NULL_HANDLE;
    }

    m_binding_descriptions.clear();
    m_binding_descriptions.shrink_to_fit();
    m_attribute_descriptions.clear();
    m_attribute_descriptions.shrink_to_fit();
  
  }

  void vkRenderer::create_instance_buffer_objects( std::vector<Drawable*>* d ) {

    vkContext* m_context = dynamic_cast< vkContext* >( k_engine->get_context() );

    VkResult vkr;
    VkDescriptorSetAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = m_context->m_descriptor_pool;
    alloc_info.descriptorSetCount = 1;
    alloc_info.pSetLayouts = &m_context->m_instance_descriptor_set_layout;

    for( int32_t i = 0; i < d->size(); ++i ) {
      vkDrawable* drawable = dynamic_cast<vkDrawable*>( d->at( i )->get_drawable() );
      vkr = vkAllocateDescriptorSets( m_context->m_device, &alloc_info, &drawable->m_descriptor_set );
      vkassert( vkr );
    }

    std::vector<VkWriteDescriptorSet> desc;

    for( int32_t i = 0; i < d->size(); ++i ) {
      vkDrawable* drawable = dynamic_cast<vkDrawable*>( d->at( i )->get_drawable() );
      vkDescriptorBuffer* buff = dynamic_cast<vkDescriptorBuffer*>( d->at( i )->get_buffer() );

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

      vkr = vkCreateBuffer( m_context->m_device, &buffer_info, nullptr, &buff->m_buffer );
      vkassert( vkr );

      vkGetBufferMemoryRequirements( m_context->m_device, buff->m_buffer, &mem_reqs );
      alloc_info.allocationSize = mem_reqs.size;

      m_context->_get_memory_type( mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &alloc_info.memoryTypeIndex );

      vkr = vkAllocateMemory( m_context->m_device, &alloc_info, nullptr, &buff->m_memory  );
      vkassert( vkr );

      vkr = vkBindBufferMemory( m_context->m_device, buff->m_buffer, buff->m_memory, 0 );
      vkassert( vkr );

      buff->m_descriptor.buffer = buff->m_buffer;
      buff->m_descriptor.offset = 0;
      buff->m_descriptor.range = sizeof( instance_buffer );

      uint8_t* data = nullptr;
      vkr = vkMapMemory( m_context->m_device, buff->m_memory, 0, sizeof( instance_buffer ),
        0, ( void** ) &data );
      vkassert( vkr );

      memset( data, 0, sizeof( instance_buffer ) );
      memcpy( data, d->at(i)->get_instance_buffer(), sizeof( instance_buffer ) );

      vkUnmapMemory( m_context->m_device, buff->m_memory );

      VkWriteDescriptorSet write_descriptor_set = {};
      write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      write_descriptor_set.dstSet = drawable->m_descriptor_set;
      write_descriptor_set.descriptorCount = 1;
      write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      write_descriptor_set.pBufferInfo = &buff->m_descriptor;
      write_descriptor_set.dstBinding = 0;

      desc.push_back( write_descriptor_set );

    }

    vkUpdateDescriptorSets( m_context->m_device, ( uint32_t ) desc.size(), &desc[0], 0, nullptr );

  }

  /* This will create the instance buffers object view in D3D12 and do nothing in Vulkan*/
  void vkRenderer::create_instance_buffer_object_view( Drawable* d, uint32_t offset ) {

  }

  void vkRenderer::update_instance_buffer_objects( std::vector<Drawable*>* d, std::vector<instance_buffer>* ib ) {

    vkContext* m_context = dynamic_cast< vkContext* >( k_engine->get_context() );

    for( int32_t i = 0; i < d->size(); ++i ) {

      vkDescriptorBuffer* buff = dynamic_cast<vkDescriptorBuffer*>( d->at( i )->get_buffer() );

      uint8_t* data = nullptr;
      VkResult vkr = vkMapMemory( m_context->m_device, buff->m_memory, 0, sizeof( instance_buffer ),
        0, ( void** ) &data );
      vkassert( vkr );

      memset( data, 0, sizeof( instance_buffer ) );
      memcpy( data, d->at( i )->get_instance_buffer(), sizeof( instance_buffer ) );

      vkUnmapMemory( m_context->m_device, buff->m_memory );

    }
  }


  /* This will create the root signature in Vulkan and D3D12 */
  void vkRenderer::create_root_signature() {

    vkContext* m_context = dynamic_cast< vkContext* >( k_engine->get_context() );

    m_binding_descriptions.resize( 1 );
    m_binding_descriptions[0].binding = 0;
    m_binding_descriptions[0].stride = sizeof( float ) * m_context->m_stride;
    m_binding_descriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    m_attribute_descriptions.resize( 5 );
    m_attribute_descriptions[0].binding = 0;
    m_attribute_descriptions[0].location = 0;
    m_attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    m_attribute_descriptions[0].offset = 0;

    m_attribute_descriptions[1].binding = 0;
    m_attribute_descriptions[1].location = 1;
    m_attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    m_attribute_descriptions[1].offset = sizeof( float ) * 3;

    m_attribute_descriptions[2].binding = 0;
    m_attribute_descriptions[2].location = 2;
    m_attribute_descriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    m_attribute_descriptions[2].offset = sizeof( float ) * 6;

    m_attribute_descriptions[3].binding = 0;
    m_attribute_descriptions[3].location = 3;
    m_attribute_descriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
    m_attribute_descriptions[3].offset = sizeof( float ) * 8;

    m_attribute_descriptions[4].binding = 0;
    m_attribute_descriptions[4].location = 4;
    m_attribute_descriptions[4].format = VK_FORMAT_R32G32B32_SFLOAT;
    m_attribute_descriptions[4].offset = sizeof( float ) * 11;

    m_vi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    m_vi.pNext = nullptr;
    m_vi.vertexBindingDescriptionCount = ( uint32_t ) m_binding_descriptions.size();
    m_vi.pVertexBindingDescriptions = m_binding_descriptions.data();
    m_vi.vertexAttributeDescriptionCount = ( uint32_t ) m_attribute_descriptions.size();
    m_vi.pVertexAttributeDescriptions = m_attribute_descriptions.data();

  }

  /* This will create the graphics pipeline in Vulkan and D3D12 */
  void vkRenderer::create_graphics_pipeline( render_type rt ) {

    vkContext* m_context = dynamic_cast< vkContext* >( k_engine->get_context() );

    VkGraphicsPipelineCreateInfo pipeline_create_info = {};
    pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_create_info.layout = m_context->m_pipeline_layout;
    pipeline_create_info.renderPass = m_context->m_render_pass;

    VkPipelineInputAssemblyStateCreateInfo input_assembly_info = {};
    input_assembly_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkPipelineRasterizationStateCreateInfo rasterization_state = {};
    rasterization_state.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterization_state.polygonMode = VK_POLYGON_MODE_FILL;
    rasterization_state.cullMode = VK_CULL_MODE_NONE;
    rasterization_state.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterization_state.depthClampEnable = VK_FALSE;
    rasterization_state.rasterizerDiscardEnable = VK_FALSE;
    rasterization_state.depthBiasEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo color_blend_state = {};
    color_blend_state.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    VkPipelineColorBlendAttachmentState blend_state[1] = {};
    blend_state[0].colorWriteMask = 0xf;
    blend_state[0].blendEnable = VK_FALSE;
    color_blend_state.attachmentCount = 1;
    color_blend_state.pAttachments = blend_state;

    VkPipelineViewportStateCreateInfo viewport_state = {};
    viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state.viewportCount = 1;
    viewport_state.scissorCount = 1;

    VkPipelineDynamicStateCreateInfo dynamic_state = {};
    std::vector<VkDynamicState> dynamic_state_enables;
    dynamic_state_enables.push_back( VK_DYNAMIC_STATE_VIEWPORT );
    dynamic_state_enables.push_back( VK_DYNAMIC_STATE_SCISSOR );
    dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state.pDynamicStates = dynamic_state_enables.data();
    dynamic_state.dynamicStateCount = ( uint32_t ) dynamic_state_enables.size();

    VkPipelineDepthStencilStateCreateInfo depth_stencil_state = {};
    depth_stencil_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth_stencil_state.depthTestEnable = VK_TRUE;
    depth_stencil_state.depthWriteEnable = VK_TRUE;
    depth_stencil_state.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    depth_stencil_state.depthBoundsTestEnable = VK_FALSE;
    depth_stencil_state.back.failOp = VK_STENCIL_OP_KEEP;
    depth_stencil_state.back.passOp = VK_STENCIL_OP_KEEP;
    depth_stencil_state.back.compareOp = VK_COMPARE_OP_ALWAYS;
    depth_stencil_state.stencilTestEnable = VK_FALSE;
    depth_stencil_state.front = depth_stencil_state.back;

    VkPipelineMultisampleStateCreateInfo multisample_state = {};
    multisample_state.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisample_state.pSampleMask = nullptr;
    multisample_state.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineShaderStageCreateInfo shader_stages[2] = { {},{} };
    if( rt == rBASIC ) {
      shader_stages[0] = _load_shaders( "basic.vert.spv", VK_SHADER_STAGE_VERTEX_BIT );
      shader_stages[1] = _load_shaders( "basic.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT );
    } else if( rt == rTEXTURE ) {
      shader_stages[0] = _load_shaders( "texture.vert.spv", VK_SHADER_STAGE_VERTEX_BIT );
      shader_stages[1] = _load_shaders( "texture.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT );
    } else if( rt == rSKYDOME ) {
      shader_stages[0] = _load_shaders( "skydome.vert.spv", VK_SHADER_STAGE_VERTEX_BIT );
      shader_stages[1] = _load_shaders( "skydome.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT );
    } else if( rt == rPOST ) {
      shader_stages[0] = _load_shaders( "post.vert.spv", VK_SHADER_STAGE_VERTEX_BIT );
      shader_stages[1] = _load_shaders( "post.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT );
    } else {
      assert( false && "UNRECOGNIZED RENDERER" );
    }

    pipeline_create_info.stageCount = 2;
    pipeline_create_info.pVertexInputState = &m_vi;
    pipeline_create_info.pInputAssemblyState = &input_assembly_info;
    pipeline_create_info.pRasterizationState = &rasterization_state;
    pipeline_create_info.pColorBlendState = &color_blend_state;
    pipeline_create_info.pMultisampleState = &multisample_state;
    pipeline_create_info.pViewportState = &viewport_state;
    pipeline_create_info.pDepthStencilState = &depth_stencil_state;
    pipeline_create_info.pStages = shader_stages;
    pipeline_create_info.renderPass = m_context->m_render_pass;
    pipeline_create_info.pDynamicState = &dynamic_state;

    if( m_pipeline != VK_NULL_HANDLE )
      vkDestroyPipeline( m_context->m_device, m_pipeline, nullptr );

    VkResult vkr = vkCreateGraphicsPipelines( m_context->m_device, m_context->m_pipeline_cache, 1, &pipeline_create_info,
      nullptr, &m_pipeline );
    vkassert( vkr );

  }

  VkPipelineShaderStageCreateInfo vkRenderer::_load_shaders( std::string filename, VkShaderStageFlagBits flags ) {

    vkContext* m_context = dynamic_cast< vkContext* >( k_engine->get_context() );

    VkPipelineShaderStageCreateInfo shader_stage = {};
    shader_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader_stage.stage = flags;

    filename = SPATH + filename;
    shader_stage.module = vkTools::loadShader( filename.c_str(), m_context->m_device, flags );
    shader_stage.pName = "main";

    assert( shader_stage.module != nullptr );
    m_context->m_shader_modules.push_back( shader_stage.module );

    return shader_stage;
  }

}