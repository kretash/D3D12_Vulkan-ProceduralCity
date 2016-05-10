#include "core/core.hh"
#ifdef NOPE

#pragma once

#include <fstream>
#include <stdint.h>
#include <vector>
#include <windows.h>
#include "vulkan/vulkan.h"
#include "imgui/imgui.h"

namespace kretash {
  class ImGuiVulkanRenderer {
  public:
    ImGuiVulkanRenderer::~ImGuiVulkanRenderer();
    bool initialize( void* handle, void* h_instance );
    void new_frame();

    // TODO: These shouldn't probably be public. Maybe use a struct with needed handles for rendering?
    // Vulkan
    VkPhysicalDeviceMemoryProperties memory_properties;

    // Rendering
    VkPipeline pipeline;
    VkPipelineCache pipeline_cache;
    VkPipelineLayout pipeline_layout;
    VkDescriptorSet descriptor_set;
    VkDescriptorSetLayout descriptor_set_layout;
    VkDescriptorPool descriptor_pool;
    VkVertexInputAttributeDescription vertex_input_attribute[3];
    VkVertexInputBindingDescription vertex_input_binding;
    VkRenderPass render_pass;
    VkShaderModule vertex_shader;
    VkShaderModule fragment_shader;

  private:
    // Vulkan
    VkSurfaceFormatKHR surface_format;
    VkPresentModeKHR present_mode;

    // For font
    VkImage font_image;
    VkImageView font_image_view;
    VkSampler font_sampler;
    VkDeviceMemory font_memory;

    // For convenience
    VkShaderModule load_shader_GLSL( std::string file_name );

    // Internal functions for Vulkan
    bool prepare_vulkan( uint8_t device_num );
    static void imgui_render( ImDrawData* draw_data );

    // Internal values
    void* window_handle;
    void* window_instance;
    std::string vertex_shader_path = "../shaders/imgui.vert.spv";
    std::string fragment_shader_path = "../shaders/imgui.frag.spv";
    int64_t ticks_per_second;
    int64_t time;
  };
}
#endif