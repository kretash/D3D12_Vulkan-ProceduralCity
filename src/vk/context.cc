#include <memory>
#include <array>
#include "vulkan/vulkandebug.h"
#include "core/vk/context.hh"
#include "core/vk/drawable.hh"
#include "core/vk/renderer.hh"
#include "core/vk/geometry.hh"
#include "core/window.hh"
#include "core/engine.hh"
#include "core/drawable.hh"
#include "core/pool.hh"
#include "core/GPU_pool.hh"

namespace kretash {

#if DEBUG
#define ENABLE_VALIDATION 1
#else
#define ENABLE_VALIDATION 0
#endif

  /* This will create the Vulkan Instance and do nothing in D3D12 */
  void vkContext::create_instance() {

    VkApplicationInfo app_info = {};
    app_info.pApplicationName = "Procedural City Rendering With The New Generation Graphics APIs";
    app_info.pEngineName = "Unreal Unity 8";
    app_info.apiVersion = VK_MAKE_VERSION( 1, 0, 4 );

    std::vector<const char*> enabledExtensions = { VK_KHR_SURFACE_EXTENSION_NAME };
    enabledExtensions.push_back( VK_KHR_WIN32_SURFACE_EXTENSION_NAME );

#if ENABLE_VALIDATION
    enabledExtensions.push_back( VK_EXT_DEBUG_REPORT_EXTENSION_NAME );
#endif

    VkInstanceCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    info.pNext = nullptr;
    info.pApplicationInfo = &app_info;
    info.enabledExtensionCount = ( uint32_t ) enabledExtensions.size();
    info.ppEnabledExtensionNames = enabledExtensions.data();
#if ENABLE_VALIDATION
    info.enabledLayerCount = vkDebug::validationLayerCount;
    info.ppEnabledLayerNames = vkDebug::validationLayerNames;
#endif

    VkAllocationCallbacks callbacks = {};

    VkResult vkr = vkCreateInstance( &info, &callbacks, &m_instance );
    vkassert( vkr );

  }

  /* This will create a device, used in both APIs */
  void vkContext::create_device() {

    uint32_t gpu_count = 0;
    VkResult vkr = vkEnumeratePhysicalDevices( m_instance, &gpu_count, &m_physical_device );
    vkassert( vkr );

    m_graphics_queue_index = 0;
    m_transfer_queue_index = 0;
    uint32_t queue_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties( m_physical_device, &queue_count, nullptr );
    assert( queue_count >= 1 );

    std::vector<VkQueueFamilyProperties> queue_props;
    queue_props.resize( queue_count );
    vkGetPhysicalDeviceQueueFamilyProperties( m_physical_device, &queue_count, queue_props.data() );

    for( m_graphics_queue_index = 0; m_graphics_queue_index < queue_count; ++m_graphics_queue_index ) {
      if( queue_props[m_graphics_queue_index].queueFlags & VK_QUEUE_GRAPHICS_BIT )
        break;
    }
    for( m_transfer_queue_index = 0; m_transfer_queue_index < queue_count; ++m_transfer_queue_index ) {
      if( queue_props[m_transfer_queue_index].queueFlags & VK_QUEUE_TRANSFER_BIT )
        break;
    }
    assert( m_graphics_queue_index < queue_count );
    assert( m_transfer_queue_index < queue_count );

    std::array< float, 1> queue_priorities = { 0.0f };
    VkDeviceQueueCreateInfo queue_create_info = {};
    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    queue_create_info.queueFamilyIndex = m_graphics_queue_index;
    queue_create_info.queueCount = 1;
    queue_create_info.pQueuePriorities = queue_priorities.data();

    std::vector<const char*> enabled_extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    VkDeviceCreateInfo device_create_info = {};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.pNext = nullptr;
    device_create_info.queueCreateInfoCount = 1;
    device_create_info.pQueueCreateInfos = &queue_create_info;
    device_create_info.pEnabledFeatures = nullptr;

    if( enabled_extensions.size() > 0 ) {
      device_create_info.enabledExtensionCount = ( uint32_t ) enabled_extensions.size();
      device_create_info.ppEnabledExtensionNames = enabled_extensions.data();
    }
#if ENABLE_VALIDATION
    device_create_info.enabledLayerCount = vkDebug::validationLayerCount;
    device_create_info.ppEnabledLayerNames = vkDebug::validationLayerNames;
#endif

    vkr = vkCreateDevice( m_physical_device, &device_create_info, nullptr, &m_device );

#if ENABLE_VALIDATION
    vkDebug::setupDebugging( m_instance, VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT, NULL );
#endif

  }

  /* This will create a swap chain and get the queue, used in both APIs */
  void vkContext::create_swap_chain( Window* w ) {

    m_device_memory_properties = {};
    vkGetPhysicalDeviceMemoryProperties( m_physical_device, &m_device_memory_properties );

    vkGetDeviceQueue( m_device, m_graphics_queue_index, 0, &m_queue );
    vkGetDeviceQueue( m_device, m_transfer_queue_index, 0, &m_texture_queue );

    std::vector<VkFormat> depth_formats = { VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D16_UNORM_S8_UINT, VK_FORMAT_D16_UNORM };
    bool depth_format_found = false;
    for( auto& format : depth_formats ) {

      VkFormatProperties format_props;
      vkGetPhysicalDeviceFormatProperties( m_physical_device, format, &format_props );

      if( format_props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT ) {
        depth_format_found = true;
        m_depth_format = format;
        break;
      }
    }
    assert( depth_format_found );

    m_swap_chain.init( m_instance, m_physical_device, m_device );
    m_swap_chain.initSwapChain( GetModuleHandle( 0 ), w->get_window_handle() );

  }

  /* This will create a command pool in Vulkan or command Allocator in D3D12 */
  void vkContext::create_command_pool() {

    VkCommandPoolCreateInfo command_pool_info = {};
    command_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    command_pool_info.queueFamilyIndex = m_swap_chain.queueNodeIndex;
    command_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    VkResult vkr = vkCreateCommandPool( m_device, &command_pool_info, nullptr, &m_command_pool );
    vkassert( vkr );

  }

  /* This will create the setup command buffer in Vulkan or command lists in D3D12*/
  void vkContext::create_setup_command_buffer() {

    {// Setup command buffers
      VkCommandBufferAllocateInfo command_buffer_allocate_info =
        vkTools::initializers::commandBufferAllocateInfo( m_command_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1 );

      VkResult vkr = vkAllocateCommandBuffers( m_device, &command_buffer_allocate_info, &m_setup_command_buffer );
      vkassert( vkr );

      VkCommandBufferBeginInfo command_buffer_begin_info = {};
      command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

      vkr = vkBeginCommandBuffer( m_setup_command_buffer, &command_buffer_begin_info );
      vkassert( vkr );
    }

  }

  /* This will set up the swap chain in Vulkan and do nothing in D3D12 */
  void vkContext::setup_swap_chain( Window* w ) {
    uint32_t h = w->get_height();
    uint32_t ww = w->get_width();
    m_swap_chain.setup( m_setup_command_buffer, &ww, &h );
  }

  /* This will create all the command buffers in Vulkan or command lists in D3D12*/
  void vkContext::create_texture_command_buffer() {

    VkCommandBufferAllocateInfo command_buffer_allocate_info =
      vkTools::initializers::commandBufferAllocateInfo( m_command_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1 );

    VkResult vkr = vkAllocateCommandBuffers( m_device, &command_buffer_allocate_info, &m_texture_command_buffer );
    vkassert( vkr );

  }

  /* This will create the render command buffer in Vulkan or command lists in D3D12*/
  void vkContext::create_render_command_buffer() {

    {//render command buffer
      m_draw_command_buffers.resize( m_swap_chain.imageCount );

      VkCommandBufferAllocateInfo command_buffer_allocate_info =
        vkTools::initializers::commandBufferAllocateInfo(
          m_command_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, ( uint32_t ) m_draw_command_buffers.size() );

      VkResult vkr = vkAllocateCommandBuffers( m_device, &command_buffer_allocate_info, m_draw_command_buffers.data() );
      vkassert( vkr );
    }
    {//post present command buffer

      VkCommandBufferAllocateInfo command_buffer_allocate_info =
        vkTools::initializers::commandBufferAllocateInfo( m_command_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1 );

      VkResult vkr = vkAllocateCommandBuffers( m_device, &command_buffer_allocate_info, &m_post_present_command_buffer );
      vkassert( vkr );
    }

  }

  /* This will create a render pass in Vulkan and do nothing in D3D12 */
  void vkContext::create_render_pass() {
    VkAttachmentDescription attachements[2];
    attachements[0].format = VK_FORMAT_B8G8R8A8_UNORM;
    attachements[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachements[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachements[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachements[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachements[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachements[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachements[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    attachements[1].format = m_depth_format;
    attachements[1].samples = VK_SAMPLE_COUNT_1_BIT;
    attachements[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachements[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachements[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachements[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachements[1].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachements[1].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference color_reference = {};
    color_reference.attachment = 0;
    color_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depth_reference = {};
    depth_reference.attachment = 1;
    depth_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.flags = 0;
    subpass.inputAttachmentCount = 0;
    subpass.pInputAttachments = nullptr;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_reference;
    subpass.pResolveAttachments = nullptr;
    subpass.pDepthStencilAttachment = &depth_reference;
    subpass.preserveAttachmentCount = 0;
    subpass.pPreserveAttachments = nullptr;

    VkRenderPassCreateInfo render_pass_info = {};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.pNext = nullptr;
    render_pass_info.attachmentCount = 2;
    render_pass_info.pAttachments = attachements;
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subpass;
    render_pass_info.dependencyCount = 0;
    render_pass_info.pDependencies = nullptr;

    VkResult vkr = vkCreateRenderPass( m_device, &render_pass_info, nullptr, &m_render_pass );
    vkassert( vkr );
  }

  /* This will create a depth stencil texture and a view in both APIs */
  void vkContext::create_depth_stencil( Window* w ) {
    VkImageCreateInfo image = {};
    image.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image.pNext = nullptr;
    image.imageType = VK_IMAGE_TYPE_2D;
    image.format = m_depth_format;
    image.extent = { ( uint32_t ) w->get_width(), ( uint32_t ) w->get_height(), 1 };
    image.mipLevels = 1;
    image.arrayLayers = 1;
    image.samples = VK_SAMPLE_COUNT_1_BIT;
    image.tiling = VK_IMAGE_TILING_OPTIMAL;
    image.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image.flags = 0;

    VkMemoryAllocateInfo mem_alloc = {};
    mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mem_alloc.pNext = nullptr;
    mem_alloc.allocationSize = 0;
    mem_alloc.memoryTypeIndex = 0;

    VkImageViewCreateInfo depth_stencil_view = {};
    depth_stencil_view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    depth_stencil_view.pNext = nullptr;
    depth_stencil_view.viewType = VK_IMAGE_VIEW_TYPE_2D;
    depth_stencil_view.format = m_depth_format;
    depth_stencil_view.flags = 0;
    depth_stencil_view.subresourceRange = {};
    depth_stencil_view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    depth_stencil_view.subresourceRange.baseMipLevel = 0;
    depth_stencil_view.subresourceRange.levelCount = 1;
    depth_stencil_view.subresourceRange.baseArrayLayer = 0;
    depth_stencil_view.subresourceRange.layerCount = 1;

    VkMemoryRequirements mem_req = {};

    VkResult vkr = vkCreateImage( m_device, &image, nullptr, &m_depth_stencil.m_image );
    vkassert( vkr );

    vkGetImageMemoryRequirements( m_device, m_depth_stencil.m_image, &mem_req );
    mem_alloc.allocationSize = mem_req.size;

    _get_memory_type( mem_req.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &mem_alloc.memoryTypeIndex );
    vkr = vkAllocateMemory( m_device, &mem_alloc, nullptr, &m_depth_stencil.m_mem );
    vkassert( vkr );

    vkr = vkBindImageMemory( m_device, m_depth_stencil.m_image, m_depth_stencil.m_mem, 0 );
    vkassert( vkr );
    vkTools::setImageLayout( m_setup_command_buffer, m_depth_stencil.m_image, VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL );

    depth_stencil_view.image = m_depth_stencil.m_image;
    vkr = vkCreateImageView( m_device, &depth_stencil_view, nullptr, &m_depth_stencil.m_view );
    vkassert( vkr );
  }

  /* This will create a framebuffer/render target and a view in both APIs */
  void vkContext::create_framebuffer( Window* w ) {
    VkImageView attachements[2];

    attachements[1] = m_depth_stencil.m_view;

    VkFramebufferCreateInfo frame_buffer_create_info = {};
    frame_buffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    frame_buffer_create_info.pNext = nullptr;
    frame_buffer_create_info.renderPass = m_render_pass;
    frame_buffer_create_info.attachmentCount = 2;
    frame_buffer_create_info.pAttachments = attachements;
    frame_buffer_create_info.width = w->get_width();
    frame_buffer_create_info.height = w->get_height();
    frame_buffer_create_info.layers = 1;

    m_framebuffers.resize( m_swap_chain.imageCount );
    for( uint32_t i = 0; i < m_framebuffers.size(); ++i ) {
      attachements[0] = m_swap_chain.buffers[i].view;
      VkResult vkr = vkCreateFramebuffer( m_device, &frame_buffer_create_info, nullptr, &m_framebuffers[i] );
      vkassert( vkr );
    }
  }

  /* This will create a pipeline cache in Vulkan and do nothing in D3D12 */
  void vkContext::create_pipeline_cache() {
    VkPipelineCacheCreateInfo pipeline_cache_create_info = {};
    pipeline_cache_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    VkResult vkr = vkCreatePipelineCache( m_device, &pipeline_cache_create_info, nullptr, &m_pipeline_cache );
    vkassert( vkr );
  }

  /* This will wait for all setup actions to be completed */
  void vkContext::wait_for_setup_completion() {

    VkResult vkr = vkEndCommandBuffer( m_setup_command_buffer );
    vkassert( vkr );

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_setup_command_buffer;

    vkr = vkQueueSubmit( m_queue, 1, &submit_info, VK_NULL_HANDLE );
    vkassert( vkr );

    vkr = vkQueueWaitIdle( m_queue );
    vkassert( vkr );

    vkFreeCommandBuffers( m_device, m_command_pool, 1, &m_setup_command_buffer );
    m_setup_command_buffer = VK_NULL_HANDLE;

  }

  /* This will allocate memory for texture upload in Vulkan and do nothing in D3D12 */
  void vkContext::allocate_device_memory( uint64_t size ) {

    m_vk_device_pool = nullptr;
    m_vk_device_pool = std::make_shared<Pool>();
    m_vk_device_pool->init( size, kDEVICE_MEMORY );

    VkMemoryAllocateInfo mem_alloc = {};
    mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mem_alloc.pNext = nullptr;
    mem_alloc.allocationSize = size;
    mem_alloc.memoryTypeIndex = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    VkResult vkr = vkAllocateMemory( m_device, &mem_alloc, nullptr, &m_device_pool_memory );
    vkassert( vkr );

  }

  /* This will allocate memory for texture upload in Vulkan and do nothing in D3D12 */
  void vkContext::allocate_host_memory( uint64_t size ) {

    m_vk_host_pool = nullptr;
    m_vk_host_pool = std::make_shared<Pool>();
    m_vk_host_pool->init( size, kHOST_VISIBLE );

    VkMemoryAllocateInfo mem_alloc = {};
    mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mem_alloc.pNext = nullptr;
    mem_alloc.allocationSize = size;
    mem_alloc.memoryTypeIndex = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

    VkResult vkr = vkAllocateMemory( m_device, &mem_alloc, nullptr, &m_host_pool_memory );
    vkassert( vkr );

  }

  /* This will create a descriptor set layout in Vulkan and do nothing in D3D12 */
  void vkContext::create_descriptor_set_layout() {
    VkResult vkr;

    VkDescriptorSetLayoutBinding layout_binding[2] = {};
    layout_binding[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layout_binding[0].descriptorCount = 1;
    layout_binding[0].stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
    layout_binding[0].pImmutableSamplers = nullptr;
    layout_binding[0].binding = 0;

    layout_binding[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    layout_binding[1].descriptorCount = m_max_textures;
    layout_binding[1].stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
    layout_binding[1].pImmutableSamplers = nullptr;
    layout_binding[1].binding = 1;

    VkDescriptorSetLayoutCreateInfo descriptor_layout = {};
    descriptor_layout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptor_layout.pNext = nullptr;
    descriptor_layout.pBindings = &layout_binding[0];

    //Instance buffer
    descriptor_layout.bindingCount = 1;
    vkr = vkCreateDescriptorSetLayout( m_device, &descriptor_layout, nullptr, &m_instance_descriptor_set_layout );
    vkassert( vkr );

    //Scene buffer
    descriptor_layout.bindingCount = 2;
    vkr = vkCreateDescriptorSetLayout( m_device, &descriptor_layout, nullptr, &m_constant_descriptor_set_layout );
    vkassert( vkr );

    VkDescriptorSetLayout* dsl[2] =
    { &m_instance_descriptor_set_layout, &m_constant_descriptor_set_layout };

    VkPipelineLayoutCreateInfo pipeline_layout_create = {};
    pipeline_layout_create.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_create.pNext = nullptr;
    pipeline_layout_create.setLayoutCount = 2;
    pipeline_layout_create.pSetLayouts = dsl[0];

    vkr = vkCreatePipelineLayout( m_device, &pipeline_layout_create, nullptr, &m_pipeline_layout );
    vkassert( vkr );

  }

  /* This will create descriptor pool in Vulkan and do nothing in D3D12 */
  void vkContext::create_descriptor_pool() {

    VkDescriptorPoolSize type_counts[2];
    type_counts[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    type_counts[0].descriptorCount = 1;
    type_counts[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    type_counts[1].descriptorCount = 1;

    VkDescriptorPoolCreateInfo descriptor_pool_info = {};
    descriptor_pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptor_pool_info.pNext = nullptr;
    descriptor_pool_info.poolSizeCount = 2;
    descriptor_pool_info.pPoolSizes = type_counts;
    descriptor_pool_info.maxSets = k_engine->get_total_drawables() + m_max_textures + 16;

    VkResult vkr = vkCreateDescriptorPool( m_device, &descriptor_pool_info, nullptr, &m_descriptor_pool );
    vkassert( vkr );
  }

  /* This will create the constant buffer object in Vulkan and D3D12 */
  void vkContext::create_constant_buffer_object( xxDescriptorBuffer* db, constant_buffer* cb ) {
    {
      VkDescriptorSetAllocateInfo alloc_info = {};
      alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
      alloc_info.descriptorPool = m_descriptor_pool;
      alloc_info.descriptorSetCount = 1;
      alloc_info.pSetLayouts = &m_constant_descriptor_set_layout;

      VkResult vkr = vkAllocateDescriptorSets( m_device, &alloc_info, &m_constant_descriptor_set );
      vkassert( vkr );
    }

    vkDescriptorBuffer* m_buffer = dynamic_cast< vkDescriptorBuffer* >( db );

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

    VkResult vkr = vkCreateBuffer( m_device, &buffer_info, nullptr, &m_buffer->m_buffer );
    vkassert( vkr );

    vkGetBufferMemoryRequirements( m_device, m_buffer->m_buffer, &mem_reqs );
    alloc_info.allocationSize = mem_reqs.size;

    _get_memory_type( mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &alloc_info.memoryTypeIndex );

    vkr = vkAllocateMemory( m_device, &alloc_info, nullptr, &( m_buffer->m_memory ) );
    vkassert( vkr );

    vkr = vkBindBufferMemory( m_device, m_buffer->m_buffer, m_buffer->m_memory, 0 );
    vkassert( vkr );

    m_buffer->m_descriptor.buffer = m_buffer->m_buffer;
    m_buffer->m_descriptor.offset = 0;
    m_buffer->m_descriptor.range = sizeof( constant_buffer );

    VkWriteDescriptorSet write_descriptor_set = {};
    write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write_descriptor_set.dstSet = m_constant_descriptor_set;
    write_descriptor_set.descriptorCount = 1;
    write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    write_descriptor_set.pBufferInfo = &m_buffer->m_descriptor;
    write_descriptor_set.dstBinding = 0;

    vkUpdateDescriptorSets( m_device, 1, &write_descriptor_set, 0, nullptr );
  }

  /* This will update the constant buffer object in Vulkan and D3D12 */
  void vkContext::update_constant_buffer_object( xxDescriptorBuffer* db, constant_buffer* cb ) {
    uint8_t* data = nullptr;

    vkDescriptorBuffer* m_buffer = dynamic_cast< vkDescriptorBuffer* >( db );

    VkResult vkr = vkMapMemory( m_device, m_buffer->m_memory, 0, sizeof( constant_buffer ), 0, ( void** ) &data );
    vkassert( vkr );

    memcpy( data, cb, sizeof( constant_buffer ) );

    vkUnmapMemory( m_device, m_buffer->m_memory );
  }

  /* This will reset the texture command list in Vulkan and D3D12 */
  void vkContext::reset_texture_command_list() {

    VkResult vkr = vkResetCommandBuffer( m_texture_command_buffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT );
    vkassert( vkr );

    VkCommandBufferBeginInfo command_buffer_begin_info = {};
    command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    vkr = vkBeginCommandBuffer( m_texture_command_buffer, &command_buffer_begin_info );
    vkassert( vkr );

  };

  /* This will execute the texture command list in Vulkan and D3D12 */
  void vkContext::compute_texture_upload() {

    VkResult vkr = vkEndCommandBuffer( m_texture_command_buffer );
    vkassert( vkr );

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_texture_command_buffer;

    vkr = vkQueueSubmit( m_texture_queue, 1, &submit_info, VK_NULL_HANDLE );
    vkassert( vkr );

  }

  /* This will wait for texture command list in Vulkan and D3D12 */
  void vkContext::wait_for_texture_upload() {

    VkResult vkr = vkQueueWaitIdle( m_texture_queue );
    vkassert( vkr );

  }

  /* This will defrag the host memory pool in Vulkan and do nothing is D3D12 */
  void vkContext::defrag_host_memory_pool() {
    m_vk_host_pool->defrag();
  }

  /* This will defrag the device memory pool in Vulkan and do nothing is D3D12 */
  void vkContext::defrag_device_memory_pool() {
    m_vk_device_pool->defrag();
  }

  /* This will reset the render command list in Vulkan and D3D12 */
  void vkContext::reset_render_command_list( Window* w ) {

    VkCommandBufferBeginInfo command_buffer_info = {};
    command_buffer_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    command_buffer_info.pNext = nullptr;

    VkClearValue clear[2] = {};
    clear[0].color = { { 0.025f, 0.025f, 0.025f, 1.0f } };
    clear[0].depthStencil = { 1.0f, 0 };
    clear[1].color = { { 0.025f, 0.025f, 0.025f, 1.0f } };
    clear[1].depthStencil = { 1.0f, 0 };

    VkRenderPassBeginInfo render_pass_beging_info = {};
    render_pass_beging_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_beging_info.pNext = nullptr;
    render_pass_beging_info.renderPass = m_render_pass;
    render_pass_beging_info.renderArea.offset.x = 0;
    render_pass_beging_info.renderArea.offset.y = 0;
    render_pass_beging_info.renderArea.extent.width = ( uint32_t ) w->get_width();
    render_pass_beging_info.renderArea.extent.height = ( uint32_t ) w->get_height();
    render_pass_beging_info.clearValueCount = 2;
    render_pass_beging_info.pClearValues = clear;

    int32_t cb = 0;//e->m_current_buffer;

    render_pass_beging_info.framebuffer = m_framebuffers[m_current_buffer];

    vkResetCommandBuffer( m_draw_command_buffers[cb], VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT );

    VkResult vkr = vkBeginCommandBuffer( m_draw_command_buffers[cb], &command_buffer_info );
    vkassert( vkr );

    vkCmdBeginRenderPass( m_draw_command_buffers[cb], &render_pass_beging_info, VK_SUBPASS_CONTENTS_INLINE );

  }

  /* This will clear the color buffer in Vulkan and D3D12 */
  void vkContext::clear_color() {

  }

  /* This will clear the depth in Vulkan and D3D12 */
  void vkContext::clear_depth() {

  }

  /* This will record commands list in Vulkan and D3D12 */
  void vkContext::record_commands( xxRenderer* r, Window* w, Drawable** draw, uint32_t d_count ) {

    int32_t cb = 0;

    VkViewport viewport = {};
    viewport.width = ( float ) w->get_width();
    viewport.height = ( float ) w->get_height();
    viewport.minDepth = ( float )0.0f;
    viewport.maxDepth = ( float )1.0f;
    vkCmdSetViewport( m_draw_command_buffers[cb], 0, 1, &viewport );

    VkRect2D scissor = {};
    scissor.extent.width = w->get_width();
    scissor.extent.height = w->get_height();
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    vkCmdSetScissor( m_draw_command_buffers[cb], 0, 1, &scissor );

    vkRenderer* m_renderer = dynamic_cast< vkRenderer* >( r );

    vkCmdBindPipeline( m_draw_command_buffers[cb], VK_PIPELINE_BIND_POINT_GRAPHICS, m_renderer->m_pipeline );

    vkGeometry* m_geometry = dynamic_cast< vkGeometry* >( k_engine->get_GPU_pool()->get_xx_geometry() );

    VkDeviceSize offsets[1] = { 0 };
    vkCmdBindVertexBuffers( m_draw_command_buffers[cb], 0, 1, &m_geometry->m_v_buf, offsets );

    vkCmdBindIndexBuffer( m_draw_command_buffers[cb], m_geometry->m_i_buf, 0, VK_INDEX_TYPE_UINT32 );

    VkDescriptorSet ds[2] = {};
    ds[1] = m_constant_descriptor_set;

    uint32_t count = 0;
    for( count; count < d_count; ++count ) {

      vkDrawable* m_drawable = dynamic_cast< vkDrawable* >( draw[count]->get_drawable() );

      ds[0] = m_drawable->m_descriptor_set;

      vkCmdBindDescriptorSets( m_draw_command_buffers[cb], VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline_layout,
        0, 2, &ds[0], 0, nullptr );

      vkCmdDrawIndexed( m_draw_command_buffers[cb],
        draw[count]->get_geometry()->get_indicies_count(),
        1,
        draw[count]->get_geometry()->get_indicies_offset(),
        draw[count]->get_geometry()->get_vertex_offset() / m_stride,
        1 );

    }
  }

  /* This will record indirect commands list in D3D12 and normal ones in Vulkan */
  void vkContext::record_indirect_commands( xxRenderer* r, Window* w, Drawable** draw, uint32_t d_count ) {
    //Just call the normal render function in Vulkan
    record_commands( r, w, draw, d_count );
  }

  /* This will execute the command list in Vulkan and D3D12 */
  void vkContext::execute_render_command_list() {

    vkCmdEndRenderPass( m_draw_command_buffers[0] );

    VkImageMemoryBarrier memory_barrier = {};
    memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    memory_barrier.pNext = nullptr;
    memory_barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    memory_barrier.dstAccessMask = 0;
    memory_barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    memory_barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    memory_barrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
    memory_barrier.image = m_swap_chain.buffers[m_current_buffer].image;

    vkCmdPipelineBarrier(
      m_draw_command_buffers[0],
      VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
      VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
      VK_FLAGS_NONE,
      0, nullptr,
      0, nullptr,
      1, &memory_barrier );

    VkResult vkr = vkEndCommandBuffer( m_draw_command_buffers[0] );
    vkassert( vkr );

    VkSemaphore present_complete_semaphore;
    VkSemaphoreCreateInfo prenset_complete_semaphore_create_info = {};
    prenset_complete_semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    prenset_complete_semaphore_create_info.pNext = nullptr;
    prenset_complete_semaphore_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    vkr = vkCreateSemaphore( m_device, &prenset_complete_semaphore_create_info, nullptr, &present_complete_semaphore );
    vkassert( vkr );

    vkr = m_swap_chain.acquireNextImage( present_complete_semaphore, &m_current_buffer );
    vkassert( vkr );

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &present_complete_semaphore;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_draw_command_buffers[0];

    vkr = vkQueueSubmit( m_queue, 1, &submit_info, VK_NULL_HANDLE );
    vkassert( vkr );

    vkr = m_swap_chain.queuePresent( m_queue, m_current_buffer );
    vkassert( vkr );

    vkDestroySemaphore( m_device, present_complete_semaphore, nullptr );
    
  }

  /* This will present the swap chain in Vulkan and D3D12 */
  void vkContext::present_swap_chain() {

    VkImageMemoryBarrier post_present_barrier = {};
    post_present_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    post_present_barrier.pNext = nullptr;
    post_present_barrier.srcAccessMask = 0;
    post_present_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    post_present_barrier.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    post_present_barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    post_present_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    post_present_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    post_present_barrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
    post_present_barrier.image = m_swap_chain.buffers[m_current_buffer].image;

    VkCommandBufferBeginInfo command_buffer_info = {};
    command_buffer_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    vkResetCommandBuffer( m_post_present_command_buffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT );

    VkResult vkr = vkBeginCommandBuffer( m_post_present_command_buffer, &command_buffer_info );
    vkassert( vkr );

    vkCmdPipelineBarrier(
      m_post_present_command_buffer,
      VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
      VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
      VK_FLAGS_NONE,
      0, nullptr,
      0, nullptr,
      1, &post_present_barrier );

    vkr = vkEndCommandBuffer( m_post_present_command_buffer );
    vkassert( vkr );

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_post_present_command_buffer;

    vkr = vkQueueSubmit( m_queue, 1, &submit_info, VK_NULL_HANDLE );
    vkassert( vkr );

    if( m_current_buffer == 0 ) m_current_buffer = 1;
    else if( m_current_buffer == 1 ) m_current_buffer = 0;

  }

  /* This will wait for render to finish in Vulkan and D3D12 */
  void vkContext::wait_render_completition() {
    VkResult vkr = vkQueueWaitIdle( m_queue );
    vkassert( vkr );
  }

  bool vkContext::_get_memory_type( uint32_t typeBits, VkFlags properties, uint32_t * typeIndex ) {
    for( uint32_t i = 0; i < 32; i++ ) {
      if( ( typeBits & 1 ) == 1 ) {
        if( ( m_device_memory_properties.memoryTypes[i].propertyFlags & properties ) == properties ) {
          *typeIndex = i;
          return true;
        }
      }
      typeBits >>= 1;
    }
    return false;
  }

  vkContext::vkContext() {

    m_graphics_queue_index = 0;
    m_transfer_queue_index = 0;
    m_current_buffer = 0;

    m_device_memory_properties = {};
    m_swap_chain = {};
    m_depth_format = {};
    
    m_physical_device = VK_NULL_HANDLE;
    m_queue = VK_NULL_HANDLE;
    m_texture_queue = VK_NULL_HANDLE;
    m_setup_command_buffer = VK_NULL_HANDLE;
    m_texture_command_buffer = VK_NULL_HANDLE;
    m_post_present_command_buffer = VK_NULL_HANDLE;
    m_constant_descriptor_set = VK_NULL_HANDLE;

    for( int i = 0; i < m_shader_modules.size(); ++i ) {
      if( m_shader_modules[i] != VK_NULL_HANDLE ) {
        assert( m_device != VK_NULL_HANDLE && "BAD REFERENCE" );
        vkDestroyShaderModule( m_device, m_shader_modules[i], nullptr );
        m_shader_modules[i] = VK_NULL_HANDLE;
      }
    }

    m_shader_modules.clear();
    m_shader_modules.shrink_to_fit();

    for( int i = 0; i < m_framebuffers.size(); ++i ) {
      if( m_framebuffers[i] != VK_NULL_HANDLE ) {
        assert( m_device != VK_NULL_HANDLE && "BAD REFERENCE" );
        vkDestroyFramebuffer( m_device, m_framebuffers[i], nullptr );
        m_framebuffers[i] = VK_NULL_HANDLE;
      }
    }

    m_framebuffers.clear();
    m_framebuffers.shrink_to_fit();

    if( m_depth_stencil.m_image != VK_NULL_HANDLE ) {
      vkContext* m_context = dynamic_cast<vkContext*>( k_engine->get_context() );
      vkDestroyImage( m_context->m_device, m_depth_stencil.m_image, nullptr );
      m_depth_stencil.m_image = VK_NULL_HANDLE;
    }

    if( m_depth_stencil.m_view != VK_NULL_HANDLE ) {
      vkContext* m_context = dynamic_cast<vkContext*>( k_engine->get_context() );
      vkDestroyImageView( m_context->m_device, m_depth_stencil.m_view, nullptr );
      m_depth_stencil.m_view = VK_NULL_HANDLE;
    }

    if( m_depth_stencil.m_mem != VK_NULL_HANDLE ) {
      assert( m_device != VK_NULL_HANDLE && "BAD REFERENCE" );
      vkFreeMemory( m_device, m_depth_stencil.m_mem, nullptr );
      m_depth_stencil.m_mem = VK_NULL_HANDLE;
    }

    if( m_instance_descriptor_set_layout != VK_NULL_HANDLE ) {
      assert( m_device != VK_NULL_HANDLE && "BAD REFERENCE" );
      vkDestroyDescriptorSetLayout( m_device, m_instance_descriptor_set_layout, nullptr );
      m_instance_descriptor_set_layout = VK_NULL_HANDLE;
    }

    if( m_constant_descriptor_set_layout != VK_NULL_HANDLE ) {
      assert( m_device != VK_NULL_HANDLE && "BAD REFERENCE" );
      vkDestroyDescriptorSetLayout( m_device, m_constant_descriptor_set_layout, nullptr );
      m_constant_descriptor_set_layout = VK_NULL_HANDLE;
    }

    if( m_pipeline_cache != VK_NULL_HANDLE ) {
      assert( m_device != VK_NULL_HANDLE && "BAD REFERENCE" );
      vkDestroyPipelineCache( m_device, m_pipeline_cache, nullptr );
      m_pipeline_cache = VK_NULL_HANDLE;
    }

    if( m_render_pass != VK_NULL_HANDLE ) {
      assert( m_device != VK_NULL_HANDLE && "BAD REFERENCE" );
      vkDestroyRenderPass( m_device, m_render_pass, nullptr );
      m_render_pass = VK_NULL_HANDLE;
    }

    if( m_setup_command_buffer != VK_NULL_HANDLE ) {
      assert( m_device != VK_NULL_HANDLE && "BAD REFERENCE" );
      assert( m_command_pool != VK_NULL_HANDLE && "BAD REFERENCE" );
      vkFreeCommandBuffers( m_device, m_command_pool, 1, &m_setup_command_buffer );
      m_setup_command_buffer = VK_NULL_HANDLE;
    }

    if( m_texture_command_buffer != VK_NULL_HANDLE ) {
      assert( m_device != VK_NULL_HANDLE && "BAD REFERENCE" );
      assert( m_command_pool != VK_NULL_HANDLE && "BAD REFERENCE" );
      vkFreeCommandBuffers( m_device, m_command_pool, 1, &m_texture_command_buffer );
      m_texture_command_buffer = VK_NULL_HANDLE;
    }

    if( m_post_present_command_buffer != VK_NULL_HANDLE ) {
      assert( m_device != VK_NULL_HANDLE && "BAD REFERENCE" );
      assert( m_command_pool != VK_NULL_HANDLE && "BAD REFERENCE" );
      vkFreeCommandBuffers( m_device, m_command_pool, 1, &m_post_present_command_buffer );
      m_post_present_command_buffer = VK_NULL_HANDLE;
    }

    for( int32_t i = 0; i < m_draw_command_buffers.size(); ++i ) {
      if( m_draw_command_buffers[i] != VK_NULL_HANDLE ) {
        assert( m_device != VK_NULL_HANDLE && "BAD REFERENCE" );
        assert( m_command_pool != VK_NULL_HANDLE && "BAD REFERENCE" );
        vkFreeCommandBuffers( m_device, m_command_pool, 1, &m_draw_command_buffers[i] );
        m_draw_command_buffers[i] = VK_NULL_HANDLE;
      }
    }
    m_draw_command_buffers.clear();
    m_draw_command_buffers.shrink_to_fit();

    if( m_descriptor_pool != VK_NULL_HANDLE ) {
      assert( m_device != VK_NULL_HANDLE && "BAD REFERENCE" );
      vkDestroyDescriptorPool( m_device, m_descriptor_pool, nullptr );
      m_descriptor_pool = VK_NULL_HANDLE;
    }

    if( m_command_pool != VK_NULL_HANDLE ) {
      assert( m_device != VK_NULL_HANDLE && "BAD REFERENCE" );
      vkDestroyCommandPool( m_device, m_command_pool, nullptr );
      m_command_pool = VK_NULL_HANDLE;
    }

    if( m_pipeline_layout != VK_NULL_HANDLE ) {
      assert( m_device != VK_NULL_HANDLE && "BAD REFERENCE" );
      vkDestroyPipelineLayout( m_device, m_pipeline_layout, nullptr );
      m_pipeline_layout = VK_NULL_HANDLE;
    }

    if( m_device_pool_memory != VK_NULL_HANDLE ) {
      assert( m_device != VK_NULL_HANDLE && "BAD REFERENCE" );
      vkFreeMemory( m_device, m_device_pool_memory, nullptr );
      m_device_pool_memory = VK_NULL_HANDLE;
    }

    if( m_host_pool_memory != VK_NULL_HANDLE ) {
      assert( m_device != VK_NULL_HANDLE && "BAD REFERENCE" );
      vkFreeMemory( m_device, m_host_pool_memory, nullptr );
      m_host_pool_memory = VK_NULL_HANDLE;
    }

    if( m_device != VK_NULL_HANDLE ) {
      vkDestroyDevice( m_device, nullptr );
      m_device = VK_NULL_HANDLE;
    }

    if( m_instance != VK_NULL_HANDLE ) {
      vkDestroyInstance( m_instance, nullptr );
      m_instance = VK_NULL_HANDLE;
    }
  }

  vkContext::~vkContext() {

    m_graphics_queue_index = 0;
    m_transfer_queue_index = 0;
    m_current_buffer = 0;

    m_device_memory_properties = {};
    m_swap_chain.cleanup();
    m_swap_chain = {};
    m_depth_format = {};

    m_physical_device = VK_NULL_HANDLE;
    m_queue = VK_NULL_HANDLE;
    m_texture_queue = VK_NULL_HANDLE;
    m_setup_command_buffer = VK_NULL_HANDLE;
    m_texture_command_buffer = VK_NULL_HANDLE;
    m_post_present_command_buffer = VK_NULL_HANDLE;
    m_constant_descriptor_set = VK_NULL_HANDLE;

    for( int i = 0; i < m_shader_modules.size(); ++i ) {
      if( m_shader_modules[i] != VK_NULL_HANDLE ) {
        assert( m_device != VK_NULL_HANDLE && "BAD REFERENCE" );
        vkDestroyShaderModule( m_device, m_shader_modules[i], nullptr );
        m_shader_modules[i] = VK_NULL_HANDLE;
      }
    }

    m_shader_modules.clear();
    m_shader_modules.shrink_to_fit();

    for( int i = 0; i < m_framebuffers.size(); ++i ) {
      if( m_framebuffers[i] != VK_NULL_HANDLE ) {
        assert( m_device != VK_NULL_HANDLE && "BAD REFERENCE" );
        vkDestroyFramebuffer( m_device, m_framebuffers[i], nullptr );
        m_framebuffers[i] = VK_NULL_HANDLE;
      }
    }

    m_framebuffers.clear();
    m_framebuffers.shrink_to_fit();

    if( m_depth_stencil.m_image != VK_NULL_HANDLE ) {
      assert( m_device != VK_NULL_HANDLE && "BAD REFERENCE" );
      vkDestroyImage( m_device, m_depth_stencil.m_image, nullptr );
      m_depth_stencil.m_image = VK_NULL_HANDLE;
    }

    if( m_depth_stencil.m_view != VK_NULL_HANDLE ) {
      assert( m_device != VK_NULL_HANDLE && "BAD REFERENCE" );
      vkDestroyImageView( m_device, m_depth_stencil.m_view, nullptr );
      m_depth_stencil.m_view = VK_NULL_HANDLE;
    }

    if( m_depth_stencil.m_mem != VK_NULL_HANDLE ) {
      assert( m_device != VK_NULL_HANDLE && "BAD REFERENCE" );
      vkFreeMemory( m_device, m_depth_stencil.m_mem, nullptr );
      m_depth_stencil.m_mem = VK_NULL_HANDLE;
    }

    if( m_instance_descriptor_set_layout != VK_NULL_HANDLE ) {
      assert( m_device != VK_NULL_HANDLE && "BAD REFERENCE" );
      vkDestroyDescriptorSetLayout( m_device, m_instance_descriptor_set_layout, nullptr );
      m_instance_descriptor_set_layout = VK_NULL_HANDLE;
    }

    if( m_constant_descriptor_set_layout != VK_NULL_HANDLE ) {
      assert( m_device != VK_NULL_HANDLE && "BAD REFERENCE" );
      vkDestroyDescriptorSetLayout( m_device, m_constant_descriptor_set_layout, nullptr );
      m_constant_descriptor_set_layout = VK_NULL_HANDLE;
    }

    if( m_pipeline_cache != VK_NULL_HANDLE ) {
      assert( m_device != VK_NULL_HANDLE && "BAD REFERENCE" );
      vkDestroyPipelineCache( m_device, m_pipeline_cache, nullptr );
      m_pipeline_cache = VK_NULL_HANDLE;
    }

    if( m_render_pass != VK_NULL_HANDLE ) {
      assert( m_device != VK_NULL_HANDLE && "BAD REFERENCE" );
      vkDestroyRenderPass( m_device, m_render_pass, nullptr );
      m_render_pass = VK_NULL_HANDLE;
    }

    if( m_setup_command_buffer != VK_NULL_HANDLE ) {
      assert( m_device != VK_NULL_HANDLE && "BAD REFERENCE" );
      assert( m_command_pool != VK_NULL_HANDLE && "BAD REFERENCE" );
      vkFreeCommandBuffers( m_device, m_command_pool, 1, &m_setup_command_buffer );
      m_setup_command_buffer = VK_NULL_HANDLE;
    }

    if( m_texture_command_buffer != VK_NULL_HANDLE ) {
      assert( m_device != VK_NULL_HANDLE && "BAD REFERENCE" );
      assert( m_command_pool != VK_NULL_HANDLE && "BAD REFERENCE" );
      vkFreeCommandBuffers( m_device, m_command_pool, 1, &m_texture_command_buffer );
      m_texture_command_buffer = VK_NULL_HANDLE;
    }

    if( m_post_present_command_buffer != VK_NULL_HANDLE ) {
      assert( m_device != VK_NULL_HANDLE && "BAD REFERENCE" );
      assert( m_command_pool != VK_NULL_HANDLE && "BAD REFERENCE" );
      vkFreeCommandBuffers( m_device, m_command_pool, 1, &m_post_present_command_buffer );
      m_post_present_command_buffer = VK_NULL_HANDLE;
    }

    for( int32_t i = 0; i < m_draw_command_buffers.size(); ++i ) {
      if( m_draw_command_buffers[i] != VK_NULL_HANDLE ) {
        assert( m_device != VK_NULL_HANDLE && "BAD REFERENCE" );
        assert( m_command_pool != VK_NULL_HANDLE && "BAD REFERENCE" );
        vkFreeCommandBuffers( m_device, m_command_pool, 1, &m_draw_command_buffers[i] );
        m_draw_command_buffers[i] = VK_NULL_HANDLE;
      }
    }
    m_draw_command_buffers.clear();
    m_draw_command_buffers.shrink_to_fit();

    if( m_descriptor_pool != VK_NULL_HANDLE ) {
      assert( m_device != VK_NULL_HANDLE && "BAD REFERENCE" );
      vkDestroyDescriptorPool( m_device, m_descriptor_pool, nullptr );
      m_descriptor_pool = VK_NULL_HANDLE;
    }

    if( m_command_pool != VK_NULL_HANDLE ) {
      assert( m_device != VK_NULL_HANDLE && "BAD REFERENCE" );
      vkDestroyCommandPool( m_device, m_command_pool, nullptr );
      m_command_pool = VK_NULL_HANDLE;
    }

    if( m_pipeline_layout != VK_NULL_HANDLE ) {
      assert( m_device != VK_NULL_HANDLE && "BAD REFERENCE" );
      vkDestroyPipelineLayout( m_device, m_pipeline_layout, nullptr );
      m_pipeline_layout = VK_NULL_HANDLE;
    }

    if( m_device_pool_memory != VK_NULL_HANDLE ) {
      assert( m_device != VK_NULL_HANDLE && "BAD REFERENCE" );
      vkFreeMemory( m_device, m_device_pool_memory, nullptr );
      m_device_pool_memory = VK_NULL_HANDLE;
    }

    if( m_host_pool_memory != VK_NULL_HANDLE ) {
      assert( m_device != VK_NULL_HANDLE && "BAD REFERENCE" );
      vkFreeMemory( m_device, m_host_pool_memory, nullptr );
      m_host_pool_memory = VK_NULL_HANDLE;
    }

    if( m_device != VK_NULL_HANDLE ) {
      vkDestroyDevice( m_device, nullptr );
      m_device = VK_NULL_HANDLE;
    }

    if( m_instance != VK_NULL_HANDLE ) {
      vkDestroyInstance( m_instance, nullptr );
      m_instance = VK_NULL_HANDLE;
    }
  }

}