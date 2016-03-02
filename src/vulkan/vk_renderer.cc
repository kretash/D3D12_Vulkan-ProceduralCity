#include "core/k_vulkan.hh"

#ifdef __VULKAN__

#include <vulkan/vulkan.h>
#include <vulkan/vulkandebug.h>
#include <vulkan/vulkantools.h>

using namespace kretash;

namespace vk {

  void create_srv_view_heap( engine_data* e, renderer_data* r, int32_t size ) {

  }

  void create_root_signature( renderer_data* r ) {

    r->m_binding_descriptions.resize( 1 );
    r->m_binding_descriptions[0].binding = VERTEX_BUFFER_BIND_ID;
    r->m_binding_descriptions[0].stride = sizeof( float ) * m_stride;
    r->m_binding_descriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    r->m_attribute_descriptions.resize( 5 );
    r->m_attribute_descriptions[0].binding = VERTEX_BUFFER_BIND_ID;
    r->m_attribute_descriptions[0].location = 0;
    r->m_attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    r->m_attribute_descriptions[0].offset = 0;

    r->m_attribute_descriptions[1].binding = VERTEX_BUFFER_BIND_ID;
    r->m_attribute_descriptions[1].location = 1;
    r->m_attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    r->m_attribute_descriptions[1].offset = sizeof( float ) * 3;

    r->m_attribute_descriptions[2].binding = VERTEX_BUFFER_BIND_ID;
    r->m_attribute_descriptions[2].location = 2;
    r->m_attribute_descriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    r->m_attribute_descriptions[2].offset = sizeof( float ) * 6;

    r->m_attribute_descriptions[3].binding = VERTEX_BUFFER_BIND_ID;
    r->m_attribute_descriptions[3].location = 3;
    r->m_attribute_descriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
    r->m_attribute_descriptions[3].offset = sizeof( float ) * 8;

    r->m_attribute_descriptions[4].binding = VERTEX_BUFFER_BIND_ID;
    r->m_attribute_descriptions[4].location = 4;
    r->m_attribute_descriptions[4].format = VK_FORMAT_R32G32B32_SFLOAT;
    r->m_attribute_descriptions[4].offset = sizeof( float ) * 11;

    r->m_vi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    r->m_vi.pNext = nullptr;
    r->m_vi.vertexBindingDescriptionCount = ( uint32_t ) r->m_binding_descriptions.size();
    r->m_vi.pVertexBindingDescriptions = r->m_binding_descriptions.data();
    r->m_vi.vertexAttributeDescriptionCount = ( uint32_t ) r->m_attribute_descriptions.size();
    r->m_vi.pVertexAttributeDescriptions = r->m_attribute_descriptions.data();
  }

  VkPipelineShaderStageCreateInfo _load_shaders( engine_data* e, std::string filename, VkShaderStageFlagBits flags ) {
    VkPipelineShaderStageCreateInfo shader_stage = {};
    shader_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader_stage.stage = flags;
    filename = "../assets/shaders/" + filename;
    shader_stage.module = vkTools::loadShader( filename.c_str(), e->m_device, flags );
    shader_stage.pName = "main";

    assert( shader_stage.module != nullptr );
    e->m_shader_modules.push_back( shader_stage.module );
    return shader_stage;
  }

  void create_graphics_pipeline( engine_data* e, renderer_data* r, render_type s ) {

    VkGraphicsPipelineCreateInfo pipeline_create_info = {};
    pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_create_info.layout = e->m_pipeline_layout;
    pipeline_create_info.renderPass = e->m_render_pass;

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
    if( s == rBASIC ) {
      shader_stages[0] = _load_shaders( e, "basic.vert.spv", VK_SHADER_STAGE_VERTEX_BIT );
      shader_stages[1] = _load_shaders( e, "basic.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT );
    } else if( s == rTEXTURE ) {
      shader_stages[0] = _load_shaders( e, "texture.vert.spv", VK_SHADER_STAGE_VERTEX_BIT );
      shader_stages[1] = _load_shaders( e, "texture.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT );
    } else if( s == rSKYDOME ) {
      shader_stages[0] = _load_shaders( e, "skydome.vert.spv", VK_SHADER_STAGE_VERTEX_BIT );
      shader_stages[1] = _load_shaders( e, "skydome.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT );
    } else if( s == rPOST ) {
      shader_stages[0] = _load_shaders( e, "post.vert.spv", VK_SHADER_STAGE_VERTEX_BIT );
      shader_stages[1] = _load_shaders( e, "post.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT );
    } else {
      assert( false && "UNRECOGNIZED RENDERER");
    }

    pipeline_create_info.stageCount = 2;
    pipeline_create_info.pVertexInputState = &r->m_vi;
    pipeline_create_info.pInputAssemblyState = &input_assembly_info;
    pipeline_create_info.pRasterizationState = &rasterization_state;
    pipeline_create_info.pColorBlendState = &color_blend_state;
    pipeline_create_info.pMultisampleState = &multisample_state;
    pipeline_create_info.pViewportState = &viewport_state;
    pipeline_create_info.pDepthStencilState = &depth_stencil_state;
    pipeline_create_info.pStages = shader_stages;
    pipeline_create_info.renderPass = e->m_render_pass;
    pipeline_create_info.pDynamicState = &dynamic_state;

    VkResult vkr = vkCreateGraphicsPipelines( e->m_device, e->m_pipeline_cache, 1, &pipeline_create_info,
      nullptr, &r->m_pipeline );
    vkassert( vkr );

  }

}

#endif