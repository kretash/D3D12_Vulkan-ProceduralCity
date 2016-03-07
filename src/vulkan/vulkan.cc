#include "core/k_vulkan.hh"

#ifdef __VULKAN__

#include <array>

#include <core/window.hh>

using namespace kretash;

namespace vk {

  void create_instance( engine_data* e ) {
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

    VkResult vkr = vkCreateInstance( &info, &callbacks, &e->m_instance );
    vkassert( vkr );
  }

  void create_device( engine_data* e ) {
    uint32_t gpu_count = 0;
    VkResult vkr = vkEnumeratePhysicalDevices( e->m_instance, &gpu_count, &e->m_physical_device );
    vkassert( vkr );

    e->m_graphics_queue_index = 0;
    e->m_transfer_queue_index = 0;
    uint32_t queue_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties( e->m_physical_device, &queue_count, nullptr );
    assert( queue_count >= 1 );

    std::vector<VkQueueFamilyProperties> queue_props;
    queue_props.resize( queue_count );
    vkGetPhysicalDeviceQueueFamilyProperties( e->m_physical_device, &queue_count, queue_props.data() );

    for( e->m_graphics_queue_index = 0; e->m_graphics_queue_index < queue_count; ++e->m_graphics_queue_index ) {
      if( queue_props[e->m_graphics_queue_index].queueFlags & VK_QUEUE_GRAPHICS_BIT )
        break;
    }
    for( e->m_transfer_queue_index = 0; e->m_transfer_queue_index < queue_count; ++e->m_transfer_queue_index ) {
      if( queue_props[e->m_transfer_queue_index].queueFlags & VK_QUEUE_TRANSFER_BIT )
        break;
    }
    assert( e->m_graphics_queue_index < queue_count );
    assert( e->m_transfer_queue_index < queue_count );

    std::array< float, 1> queue_priorities = { 0.0f };
    VkDeviceQueueCreateInfo queue_create_info = {};
    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    queue_create_info.queueFamilyIndex = e->m_graphics_queue_index;
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

    vkr = vkCreateDevice( e->m_physical_device, &device_create_info, nullptr, &e->m_device );

#if ENABLE_VALIDATION
    vkDebug::setupDebugging( e->m_instance, VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT, NULL );
#endif
  }

  void create_swap_chain( engine_data* e, Window* w ) {

    e->m_device_memory_properties = {};
    vkGetPhysicalDeviceMemoryProperties( e->m_physical_device, &e->m_device_memory_properties );

    vkGetDeviceQueue( e->m_device, e->m_graphics_queue_index, 0, &e->m_queue );
    vkGetDeviceQueue( e->m_device, e->m_transfer_queue_index, 0, &e->m_texture_queue );

    std::vector<VkFormat> depth_formats = { VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D16_UNORM_S8_UINT, VK_FORMAT_D16_UNORM };
    bool depth_format_found = false;
    for( auto& format : depth_formats ) {

      VkFormatProperties format_props;
      vkGetPhysicalDeviceFormatProperties( e->m_physical_device, format, &format_props );

      if( format_props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT ) {
        depth_format_found = true;
        e->m_depth_format = format;
        break;
      }
    }
    assert( depth_format_found );

    e->m_swap_chain.init( e->m_instance, e->m_physical_device, e->m_device );
    e->m_swap_chain.initSwapChain( GetModuleHandle( 0 ), w->get_window_handle() );

  }

  void create_command_pool( engine_data* e ) {
    VkCommandPoolCreateInfo command_pool_info = {};
    command_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    command_pool_info.queueFamilyIndex = e->m_swap_chain.queueNodeIndex;
    command_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    VkResult vkr = vkCreateCommandPool( e->m_device, &command_pool_info, nullptr, &e->m_command_pool );
    vkassert( vkr );
  }

  void create_setup_command_buffer( engine_data* e ) {

    if( e->m_setup_command_buffer != VK_NULL_HANDLE ) {
      vkFreeCommandBuffers( e->m_device, e->m_command_pool, 1, &e->m_setup_command_buffer );
      e->m_setup_command_buffer = VK_NULL_HANDLE;
    }

    VkCommandBufferAllocateInfo command_buffer_allocate_info =
      vkTools::initializers::commandBufferAllocateInfo(
        e->m_command_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1 );

    VkResult vkr = vkAllocateCommandBuffers( e->m_device, &command_buffer_allocate_info, &e->m_setup_command_buffer );
    vkassert( vkr );

    VkCommandBufferBeginInfo command_buffer_begin_info = {};
    command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    vkr = vkBeginCommandBuffer( e->m_setup_command_buffer, &command_buffer_begin_info );
    vkassert( vkr );

    {//texture command buffer, look for another place
      VkCommandBufferAllocateInfo command_buffer_allocate_info =
        vkTools::initializers::commandBufferAllocateInfo(
          e->m_command_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1 );

      VkResult vkr = vkAllocateCommandBuffers( e->m_device, &command_buffer_allocate_info, &e->m_texture_command_buffer );
      vkassert( vkr );
    }

  }

  void setup_swap_chain( engine_data* e, Window* w ) {
    uint32_t h = w->get_height();
    uint32_t ww = w->get_width();
    e->m_swap_chain.setup( e->m_setup_command_buffer, &ww, &h );
  }

  void create_command_buffer( engine_data* e ) {
    e->m_draw_command_buffers.resize( e->m_swap_chain.imageCount );

    VkCommandBufferAllocateInfo command_buffer_allocate_info =
      vkTools::initializers::commandBufferAllocateInfo(
        e->m_command_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, ( uint32_t ) e->m_draw_command_buffers.size() );

    VkResult vkr = vkAllocateCommandBuffers( e->m_device, &command_buffer_allocate_info, e->m_draw_command_buffers.data() );
    vkassert( vkr );

    command_buffer_allocate_info.commandBufferCount = 1;

    vkr = vkAllocateCommandBuffers( e->m_device, &command_buffer_allocate_info, &e->m_post_present_command_buffer );
    vkassert( vkr );
  }

  void setup_depth_stencil( engine_data* e, Window* w ) {
    VkImageCreateInfo image = {};
    image.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image.pNext = nullptr;
    image.imageType = VK_IMAGE_TYPE_2D;
    image.format = e->m_depth_format;
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
    depth_stencil_view.format = e->m_depth_format;
    depth_stencil_view.flags = 0;
    depth_stencil_view.subresourceRange = {};
    depth_stencil_view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    depth_stencil_view.subresourceRange.baseMipLevel = 0;
    depth_stencil_view.subresourceRange.levelCount = 1;
    depth_stencil_view.subresourceRange.baseArrayLayer = 0;
    depth_stencil_view.subresourceRange.layerCount = 1;

    VkMemoryRequirements mem_req = {};

    VkResult vkr = vkCreateImage( e->m_device, &image, nullptr, &e->m_depth_stencil.m_image );
    vkassert( vkr );

    vkGetImageMemoryRequirements( e->m_device, e->m_depth_stencil.m_image, &mem_req );
    mem_alloc.allocationSize = mem_req.size;

    _get_memory_type( e, mem_req.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &mem_alloc.memoryTypeIndex );
    vkr = vkAllocateMemory( e->m_device, &mem_alloc, nullptr, &e->m_depth_stencil.m_mem );
    vkassert( vkr );

    vkr = vkBindImageMemory( e->m_device, e->m_depth_stencil.m_image, e->m_depth_stencil.m_mem, 0 );
    vkassert( vkr );
    vkTools::setImageLayout( e->m_setup_command_buffer, e->m_depth_stencil.m_image, VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL );

    depth_stencil_view.image = e->m_depth_stencil.m_image;
    vkr = vkCreateImageView( e->m_device, &depth_stencil_view, nullptr, &e->m_depth_stencil.m_view );
    vkassert( vkr );
  }

  void setup_render_pass( engine_data* e ) {
    VkAttachmentDescription attachements[2];
    attachements[0].format = VK_FORMAT_B8G8R8A8_UNORM;
    attachements[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachements[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachements[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachements[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachements[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachements[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachements[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    attachements[1].format = e->m_depth_format;
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

    VkResult vkr = vkCreateRenderPass( e->m_device, &render_pass_info, nullptr, &e->m_render_pass );
    vkassert( vkr );
  }

  void create_pipeline_cache( engine_data* e ) {
    VkPipelineCacheCreateInfo pipeline_cache_create_info = {};
    pipeline_cache_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    VkResult vkr = vkCreatePipelineCache( e->m_device, &pipeline_cache_create_info, nullptr, &e->m_pipeline_cache );
    vkassert( vkr );
  }

  void setup_framebuffer( engine_data* e, Window* w ) {
    VkImageView attachements[2];

    attachements[1] = e->m_depth_stencil.m_view;

    VkFramebufferCreateInfo frame_buffer_create_info = {};
    frame_buffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    frame_buffer_create_info.pNext = nullptr;
    frame_buffer_create_info.renderPass = e->m_render_pass;
    frame_buffer_create_info.attachmentCount = 2;
    frame_buffer_create_info.pAttachments = attachements;
    frame_buffer_create_info.width = w->get_width();
    frame_buffer_create_info.height = w->get_height();
    frame_buffer_create_info.layers = 1;

    e->m_framebuffers.resize( e->m_swap_chain.imageCount );
    for( uint32_t i = 0; i < e->m_framebuffers.size(); ++i ) {
      attachements[0] = e->m_swap_chain.buffers[i].view;
      VkResult vkr = vkCreateFramebuffer( e->m_device, &frame_buffer_create_info, nullptr, &e->m_framebuffers[i] );
      vkassert( vkr );
    }
  }

  void flush_setup_command_buffer( engine_data* e ) {
    if( e->m_setup_command_buffer == VK_NULL_HANDLE )
      return;

    VkResult vkr = vkEndCommandBuffer( e->m_setup_command_buffer );
    vkassert( vkr );

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &e->m_setup_command_buffer;

    vkr = vkQueueSubmit( e->m_queue, 1, &submit_info, VK_NULL_HANDLE );
    vkassert( vkr );

    vkr = vkQueueWaitIdle( e->m_queue );
    vkassert( vkr );

    vkFreeCommandBuffers( e->m_device, e->m_command_pool, 1, &e->m_setup_command_buffer );
    e->m_setup_command_buffer = VK_NULL_HANDLE;
  }

  void wait_for_previous_frame( engine_data* e ) {
    VkResult vkr = vkQueueWaitIdle( e->m_queue );
    vkassert( vkr );
  }
  
  void init_device_memory_pool( engine_data* e, uint64_t size ){

    VkMemoryAllocateInfo mem_alloc = {};
    mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mem_alloc.pNext = nullptr;
    mem_alloc.allocationSize = size;
    mem_alloc.memoryTypeIndex = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    VkResult vkr = vkAllocateMemory( e->m_device, &mem_alloc, nullptr, &e->m_device_pool_memory );
    vkassert( vkr );

  }

  void init_host_memory_pool( engine_data* e, uint64_t size ) {

    VkMemoryAllocateInfo mem_alloc = {};
    mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mem_alloc.pNext = nullptr;
    mem_alloc.allocationSize = size;
    mem_alloc.memoryTypeIndex = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

    VkResult vkr = vkAllocateMemory( e->m_device, &mem_alloc, nullptr, &e->m_host_pool_memory );
    vkassert( vkr );
  }

  VkBool32 _get_memory_type( engine_data* e, uint32_t typeBits, VkFlags properties, uint32_t * typeIndex ) {
    for( uint32_t i = 0; i < 32; i++ ) {
      if( ( typeBits & 1 ) == 1 ) {
        if( ( e->m_device_memory_properties.memoryTypes[i].propertyFlags & properties ) == properties ) {
          *typeIndex = i;
          return true;
        }
      }
      typeBits >>= 1;
    }
    return false;
  }
}

#endif