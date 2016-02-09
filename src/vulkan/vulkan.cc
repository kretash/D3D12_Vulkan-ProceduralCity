#include "core/k_vulkan.hh"

#ifdef __VULKAN__

namespace vk {

  //pipeline
#ifdef _DEBUG
  void enable_debug_layer( engine_data* d ){
  
  }
#endif
  void create_device( engine_data* d ){
  
  }
  void create_factory( engine_data* d ){
  
  }
  void create_command_queue( engine_data* d ){
  
  }
  void create_swap_chain( engine_data* d, Window* w ){
  
  }
  void create_render_target_view_heap( engine_data* d ){
  
  }
  void create_depth_stencil_view_heap( engine_data* d ){
  
  }
  void create_render_target_view( engine_data* d ){
  
  }
  void create_command_allocator( engine_data* d ){
  
  }
  void create_command_list( engine_data* d ){
  
  }
  void create_depth_stencil_view( engine_data* d, Window* w ){
  
  }
  void create_fences( engine_data* d ){
  
  }
  void wait_for_previous_frame( engine_data* d ){
  
  }

  //buffer
  void upload_vertex_buffer( geometry_data* g, float* array_data, int array_lenght, int stride ){
  
  }
  void upload_index_buffer( geometry_data* g, unsigned int* elements_data, int indicies_count ){
  
  }

  //material
  void create_root_signature( material_data* m ){
  
  }
  void load_and_compile_shaders( material_data* m ){
  
  }
  void create_pipeline_state_object( material_data* m ){
  
  }

  //drawable
  void create_constant_buffer_object( drawable_data* d, constant_buffer cb ){
  
  }

  //render
  void populate_command_list( engine_data* d, Window* w, Drawable** draw, int d_count ){
  
  }
  void execute_command_lists( engine_data* d ){
  
  }
  void present_swap_chain( engine_data* d ){
  
  }
}

#endif