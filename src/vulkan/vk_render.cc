#include "core/k_vulkan.hh"

#ifdef __VULKAN__

#include "core/drawable.hh"
#include "core/geometry.hh"
#include "core/GPU_pool.hh"
#include "core/world.hh"
#include "core/window.hh"
#include "vulkan/vulkan.h"
#include "vulkan/vulkantools.h"
#include "vulkan/vulkandebug.h"

using namespace kretash;

namespace vk {

  void reset_render_command_list( engine_data* e, Window* w ) {
    VkCommandBufferBeginInfo command_buffer_info = {};
    command_buffer_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    command_buffer_info.pNext = nullptr;

    VkClearValue clear[2] = {};
    clear[0].color = { { 0.025f, 0.025f, 0.025f, 1.0f } };
    clear[1].depthStencil = { 1.0f, 0 };

    VkRenderPassBeginInfo render_pass_beging_info = {};
    render_pass_beging_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_beging_info.pNext = nullptr;
    render_pass_beging_info.renderPass = e->m_render_pass;
    render_pass_beging_info.renderArea.offset.x = 0;
    render_pass_beging_info.renderArea.offset.y = 0;
    render_pass_beging_info.renderArea.extent.width = ( uint32_t ) w->get_width();
    render_pass_beging_info.renderArea.extent.height = ( uint32_t ) w->get_height();
    render_pass_beging_info.clearValueCount = 2;
    render_pass_beging_info.pClearValues = clear;

    int32_t cb = 0;//e->m_current_buffer;

    render_pass_beging_info.framebuffer = e->m_framebuffers[e->m_current_buffer];

    vkResetCommandBuffer( e->m_draw_command_buffers[cb], VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT );

    VkResult vkr = vkBeginCommandBuffer( e->m_draw_command_buffers[cb], &command_buffer_info );
    vkassert( vkr );

    vkCmdBeginRenderPass( e->m_draw_command_buffers[cb], &render_pass_beging_info, VK_SUBPASS_CONTENTS_INLINE );
  }

  void clear_color( engine_data* e, Window* w ) {

  }

  void clear_depth( engine_data* e, Window* w ) {

  }

  void populate_command_list( engine_data* e, renderer_data* r, Window* w, Drawable** draw, uint32_t d_count ) {

    int32_t cb = 0;//e->m_current_buffer;

    VkViewport viewport = {};
    viewport.width = ( float ) w->get_width();
    viewport.height = ( float ) w->get_height();
    viewport.minDepth = ( float )0.0f;
    viewport.maxDepth = ( float )1.0f;
    vkCmdSetViewport( e->m_draw_command_buffers[cb], 0, 1, &viewport );

    VkRect2D scissor = {};
    scissor.extent.width = w->get_width();
    scissor.extent.height = w->get_height();
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    vkCmdSetScissor( e->m_draw_command_buffers[cb], 0, 1, &scissor );


    vkCmdBindPipeline( e->m_draw_command_buffers[cb], VK_PIPELINE_BIND_POINT_GRAPHICS, r->m_pipeline );

    geometry_data* g_data = k_engine->get_GPU_pool()->get_geometry_data();


    VkDeviceSize offsets[1] = { 0 };
    vkCmdBindVertexBuffers( e->m_draw_command_buffers[cb], VERTEX_BUFFER_BIND_ID, 1, &g_data->m_v_buf, offsets );

    vkCmdBindIndexBuffer( e->m_draw_command_buffers[cb], g_data->m_i_buf, 0, VK_INDEX_TYPE_UINT32 );

    VkDescriptorSet ds[2] = {};
    ds[1] = k_engine->get_world()->get_buffer_data()->m_descriptor_set;

    //not the classiest way to do it
    //vkCmdBindDescriptorSets( e->m_draw_command_buffers[cb], VK_PIPELINE_BIND_POINT_GRAPHICS, e->m_pipeline_layout,
    //  0, 1, &k_engine->get_world()->get_buffer_data()->m_descriptor_set, 0, nullptr );

    uint32_t count = 0;
    for( count; count < d_count; ++count ) {

      ds[0] = draw[count]->get_drawable_data()->m_descriptor_set;
/*
    vkCmdBindDescriptorSets( e->m_draw_command_buffers[cb], VK_PIPELINE_BIND_POINT_GRAPHICS, e->m_pipeline_layout,
      0, 1, &draw[count]->get_drawable_data()->m_descriptor_set, 0, nullptr );*/

    vkCmdBindDescriptorSets( e->m_draw_command_buffers[cb], VK_PIPELINE_BIND_POINT_GRAPHICS, e->m_pipeline_layout,
      0, 2, &ds[0], 0, nullptr );

      vkCmdDrawIndexed( e->m_draw_command_buffers[cb], 
        draw[count]->get_geometry()->get_indicies_count(),
        1,
        draw[count]->get_geometry()->get_indicies_offset(),
        draw[count]->get_geometry()->get_vertex_offset() / m_stride,
        1 );

    }
  }

  void execute_command_lists( engine_data* e, renderer_data* r ) {

    vkCmdEndRenderPass( e->m_draw_command_buffers[0] );

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
    memory_barrier.image = e->m_swap_chain.buffers[e->m_current_buffer].image;

    vkCmdPipelineBarrier(
      e->m_draw_command_buffers[0],
      VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
      VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
      VK_FLAGS_NONE,
      0, nullptr,
      0, nullptr,
      1, &memory_barrier );

    VkResult vkr = vkEndCommandBuffer( e->m_draw_command_buffers[0] );
    //std::cout << "end -> with " << e->m_current_buffer << std::endl;
    vkassert( vkr );

    VkSemaphore present_complete_semaphore;
    VkSemaphoreCreateInfo prenset_complete_semaphore_create_info = {};
    prenset_complete_semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    prenset_complete_semaphore_create_info.pNext = nullptr;
    prenset_complete_semaphore_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    vkr = vkCreateSemaphore( e->m_device, &prenset_complete_semaphore_create_info, nullptr, &present_complete_semaphore );
    vkassert( vkr );

    vkr = e->m_swap_chain.acquireNextImage( present_complete_semaphore, &e->m_current_buffer );
    vkassert( vkr );

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &present_complete_semaphore;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &e->m_draw_command_buffers[0];

    //.mERROR at 2d time
    vkr = vkQueueSubmit( e->m_queue, 1, &submit_info, VK_NULL_HANDLE );
    //std::cout << "sub with " << e->m_current_buffer << std::endl;
    vkassert( vkr );

    vkr = e->m_swap_chain.queuePresent( e->m_queue, e->m_current_buffer );
    vkassert( vkr );

    vkDestroySemaphore( e->m_device, present_complete_semaphore, nullptr );

    if( e->m_current_buffer == 0 ) e->m_current_buffer = 1;
    else if( e->m_current_buffer == 1 ) e->m_current_buffer = 0;
  }

  void present_swap_chain( engine_data* e ) {

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
    post_present_barrier.image = e->m_swap_chain.buffers[e->m_current_buffer].image;

    VkCommandBufferBeginInfo command_buffer_info = {};
    command_buffer_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    vkResetCommandBuffer( e->m_post_present_command_buffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT );

    VkResult vkr = vkBeginCommandBuffer( e->m_post_present_command_buffer, &command_buffer_info );
    vkassert( vkr );

    vkCmdPipelineBarrier(
      e->m_post_present_command_buffer,
      VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
      VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
      VK_FLAGS_NONE,
      0, nullptr,
      0, nullptr,
      1, &post_present_barrier );

    vkr = vkEndCommandBuffer( e->m_post_present_command_buffer );
    vkassert( vkr );

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &e->m_post_present_command_buffer;

    vkr = vkQueueSubmit( e->m_queue, 1, &submit_info, VK_NULL_HANDLE );
    vkassert( vkr );
  }

}

#endif