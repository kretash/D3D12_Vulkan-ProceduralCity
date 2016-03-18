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
#include <memory>
#include <vector>

namespace                   kretash {

  class                     Window;
  struct                    queue;

  class                     xxGeometry {
  public:
    xxGeometry() {}
    ~xxGeometry() {}

    /* This will create an empty vertex buffer in Vulkan and D3D12 */
    virtual void            create_empty_vertex_buffer( uint64_t size ) {};

    /* This will create an empty index buffer in Vulkan and D3D12 */
    virtual void            create_empty_index_buffer( uint64_t size ) {};

    /* This will upload into an vertex buffer in Vulkan and D3D12 */
    virtual void            upload_into_vertex_buffer( uint64_t offset, float* array_data, uint64_t size ) {};

    /* This will queue an upload into an vertex buffer in Vulkan and D3D12 */
    virtual void            upload_queue_into_vertex_buffer( std::vector<kretash::queue>* queue ) {};

    /* This will upload into an index buffer in Vulkan and D3D12 */
    virtual void            upload_into_index_buffer( uint64_t offset, uint32_t* elements_data, uint64_t size ) {};

    /* This will queue an upload into an index buffer in Vulkan and D3D12 */
    virtual void            upload_queue_into_index_buffer( std::vector<kretash::queue>* queue ) {};

  };
}