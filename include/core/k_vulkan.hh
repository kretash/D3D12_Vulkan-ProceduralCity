/*
----------------------------------------------------------------------------------------------------
------                  _   _____ _  __                     ------------ /_/\  ---------------------
------              |/ |_) |_  | |_|(_ |_|                  ----------- / /\ \  --------------------
------              |\ | \ |__ | | |__)| |                  ---------- / / /\ \  -------------------
------   CARLOS MARTINEZ ROMERO - kretash.wordpress.com     --------- / / /\ \ \  ------------------
------                                                      -------- / /_/__\ \ \  -----------------
------       PROCEDURAL CITY RENDERING WITH THE NEW         ------  /_/______\_\/\  ----------------
------            GENERATION GRAPHICS APIS                  ------- \_\_________\/ -----------------
----------------------------------------------------------------------------------------------------

Licensed under the MIT License (the "License"); you may not use this file except
in compliance with the License. You may obtain a copy of the License at
http://opensource.org/licenses/MIT
*/

#pragma once

#include "core.hh"

#ifdef __VULKAN__

#include "vulkan/vulkan.h"
#include "vulkan/vulkantools.h"
#include "vulkan/vulkandebug.h"
#include "vulkan/vulkanswapchain.hpp"

#include <string>
#include <vector>

#include "types.hh"
#include "math/float4x4.hh"
#include "tools.hh"

class Window;
class Drawable;

//variables of shame
namespace vk {
  static const int32_t                            m_stride = 14;
}

struct depth_stencil {
  VkImage                                         m_image;
  VkDeviceMemory                                  m_mem;
  VkImageView                                     m_view;
};

struct engine_data {
  VkInstance                                      m_instance;
  VkPhysicalDevice                                m_physical_device;
  VkPhysicalDeviceMemoryProperties                m_device_memory_properties;
  uint32_t                                        m_graphics_queue_index;
  VkDevice                                        m_device;
  VkQueue                                         m_queue;
  VulkanSwapChain                                 m_swap_chain;
  VkFormat                                        m_depth_format;
  VkCommandPool                                   m_command_pool;
  VkCommandBuffer                                 m_setup_command_buffer;
  VkCommandBuffer                                 m_post_present_command_buffer;
  std::vector<VkCommandBuffer>                    m_draw_command_buffers;
  depth_stencil                                   m_depth_stencil;
  VkRenderPass                                    m_render_pass;
  std::vector<VkFramebuffer>                      m_framebuffers;
  VkPipelineCache                                 m_pipeline_cache;
  uint32_t                                        m_current_buffer;
  std::vector<VkShaderModule>                     m_shader_modules;
  VkDescriptorSetLayout                           m_descriptor_set_layout;
  VkDescriptorSetLayout                           m_descriptor_set_layout2;
  VkDescriptorPool                                m_descriptor_pool;
  VkPipelineLayout                                m_pipeline_layout;

  engine_data() :
    m_setup_command_buffer( VK_NULL_HANDLE ),
    m_graphics_queue_index( 0 ),
    m_current_buffer( 0 ) {
  }

};

struct constant_buffer {
  float3                                          light_pos;
  float                                           sky_color;
  float3                                          fog_color;
  float                                           ambient_intensity;
  float3                                          sun_light_intensity;
  float                                           padding;
  float3                                          eye_view;
  float4x4                                        view;

  constant_buffer() {}
  ~constant_buffer() {}
};
struct constant_buffer_data {
  VkBuffer                                        m_buffer;
  VkDeviceMemory                                  m_memory;
  VkDescriptorBufferInfo                          m_descriptor;
  VkDescriptorSet                                 m_descriptor_set;
};

struct instance_buffer {
  float4x4                                        mvp;
  float4x4                                        model;
  float4x4                                        normal_matrix;

  uint32_t                                        d_texture_id;
  uint32_t                                        n_texture_id;
  uint32_t                                        s_texture_id;

  uint32_t                                        pad[13];

  instance_buffer() :
    d_texture_id( 0 ),
    n_texture_id( 0 ),
    s_texture_id( 0 ) {
  }
};

struct instance_buffer_data {
  VkBuffer                                        m_buffer;
  VkDeviceMemory                                  m_memory;
  VkDescriptorBufferInfo                          m_descriptor;
};

struct renderer_data {
  std::vector<VkShaderModule>                     m_shader_modules;
  VkPipeline                                      m_pipeline;
  VkPipelineVertexInputStateCreateInfo            m_vi;
  std::vector<VkVertexInputBindingDescription>    m_binding_descriptions;
  std::vector<VkVertexInputAttributeDescription>  m_attribute_descriptions;

  instance_buffer_data                            m_uniform_buffer_data;
};

struct indirect_command { // investigate

};

struct geometry_data {
  int32_t                                         m_i_count;
  VkBuffer                                        m_i_buf;
  VkDeviceMemory                                  m_i_mem;
  VkBuffer                                        m_v_buf;
  VkDeviceMemory                                  m_v_mem;
};

struct texture_data {
  VkSampler                                       m_sampler;
  VkImage                                         m_image;
  VkImageLayout                                   m_image_layout;
  VkDeviceMemory                                  m_device_memory;
  VkImageView                                     m_view;
  uint32_t                                        m_width;
  uint32_t                                        m_height;
  uint32_t                                        m_mip_levels;
};

struct drawable_data {
  VkDescriptorSet                                 m_descriptor_set;
  instance_buffer                                 m_uniform_buffer;
  instance_buffer_data                            m_uniform_buffer_data;

};

#define ENABLE_VALIDATION true
#define VK_FLAGS_NONE 0
#define VERTEX_BUFFER_BIND_ID 0

namespace vk {

  void create_instance( engine_data* e );
  void create_device( engine_data* e );
  void create_swap_chain( engine_data* e, Window* w );
  void create_command_pool( engine_data* e );
  void create_setup_command_buffer( engine_data* e );
  void setup_swap_chain( engine_data* e, Window* w );
  void create_command_buffer( engine_data* e );
  void setup_depth_stencil( engine_data* e, Window* w );
  void setup_render_pass( engine_data* e );
  void create_pipeline_cache( engine_data* e );
  void setup_framebuffer( engine_data* e, Window* w );
  void flush_setup_command_buffer( engine_data* e );
  void wait_for_previous_frame( engine_data* e );
  VkBool32 _get_memory_type( engine_data* e, uint32_t typeBits, VkFlags properties, uint32_t * typeIndex );

  //buffer
  void create_empty_vertex_buffer( engine_data* e, geometry_data* g, uint32_t size );
  void create_empty_index_buffer( engine_data* e, geometry_data* g, uint32_t size );
  void upload_into_vertex_buffer( engine_data* e, geometry_data* g, uint32_t offset, float* array_data, 
    uint32_t size );
  void upload_queue_into_vertex_buffer( engine_data* e, geometry_data* g, std::vector<queue>* queue );
  void upload_into_index_buffer( engine_data* e, geometry_data* g, uint32_t offset, uint32_t* elements_data, 
    uint32_t size );
  void upload_queue_into_index_buffer( engine_data* e, geometry_data* g, std::vector<queue>* queue );

  //renderer
  void create_srv_view_heap( engine_data* e, renderer_data* r, int32_t size );
  void create_root_signature( renderer_data* r );
  VkPipelineShaderStageCreateInfo _load_shaders( engine_data* e, std::string filename, VkShaderStageFlagBits flags );
  void create_graphics_pipeline( engine_data* e, renderer_data* r, render_type s );

  //post - renderer
  void create_post_root_signature( renderer_data* r );
  void create_post_pipeline_state_object( renderer_data* r );
  void add_post_textures_to_srv( engine_data* e, renderer_data* r );
  void post_render( engine_data* e, renderer_data* r, Window* w );

  //texture
  void reset_texture_command_list( engine_data* d );
  void create_texture( engine_data* d, texture_data* t, void* data, int32_t width, int32_t height, int32_t channels );
  void create_shader_resource_view( renderer_data* r, texture_data* t, int32_t offset );
  void compute_texture_upload( engine_data* d );
  void wait_for_texture_upload( engine_data* d );
  void clear_texture_upload( texture_data* t );
  void clear_texture( texture_data* t );

  //drawable
  void init_descriptor_pool_and_layout( engine_data* e );
  void create_constant_buffer_object( engine_data* e, constant_buffer_data* cbd, constant_buffer cb );
  void update_constant_buffer_object( engine_data* e, constant_buffer_data* cbd, constant_buffer cb );
  void create_instance_buffer_object( engine_data* e, renderer_data* r, instance_buffer* ub );
  void create_instance_buffer_view( engine_data* e, renderer_data* r, drawable_data* d, uint64_t buffer_offset, int32_t cbv_offset );
  void update_instance_buffer_object( engine_data* e, renderer_data* r, instance_buffer* ub );
  void create_and_update_descriptor_sets( engine_data* e, renderer_data* r, std::vector<Drawable*>* draw);
  void update_decriptor_sets( engine_data* e, renderer_data* r, std::vector<Drawable*>* draw );

  //render
  void reset_render_command_list( engine_data* e, Window* w );
  void clear_color( engine_data* e, Window* w );
  void clear_depth( engine_data* e, Window* w );
  void populate_command_list( engine_data* d, renderer_data* r, Window* w, Drawable** draw, uint32_t d_count );
  void execute_command_lists( engine_data* d, renderer_data* r );
  void present_swap_chain( engine_data* d );

  //execute indirect
  void create_command_signature( engine_data* d, renderer_data* r );
  void create_and_fill_command_buffer( engine_data* d, renderer_data* r, Drawable** draw, uint32_t d_count );
  void update_command_buffer( engine_data* d, renderer_data* r, Drawable** draw, uint32_t d_count );
  void populate_indirect_command_list( engine_data* e, renderer_data* r, Window* w, Drawable** draw, uint32_t d_count );

}

#endif