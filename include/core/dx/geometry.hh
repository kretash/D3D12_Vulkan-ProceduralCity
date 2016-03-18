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
#include "core/xx/geometry.hh"
#include <wrl.h>
#include "d3dx12.h"
#include <d3d12.h>
#include <dxgi1_4.h>
#include <d3dcompiler.h>

#define m_ptr Microsoft::WRL::ComPtr

namespace                   kretash {

  class                     Window;
  struct                    queue;

  class                     dxGeometry : public virtual xxGeometry {
  public:

    friend class            dxContext;

    dxGeometry();
    ~dxGeometry();

    /* This will create an empty vertex buffer in Vulkan and D3D12 */
    virtual void            create_empty_vertex_buffer( uint64_t size ) final;

    /* This will create an empty index buffer in Vulkan and D3D12 */
    virtual void            create_empty_index_buffer( uint64_t size ) final;

    /* This will upload into an vertex buffer in Vulkan and D3D12 */
    virtual void            upload_into_vertex_buffer( uint64_t offset, float* array_data, uint64_t size ) final;

    /* This will queue an upload into an vertex buffer in Vulkan and D3D12 */
    virtual void            upload_queue_into_vertex_buffer( std::vector<kretash::queue>* queue ) final;

    /* This will upload into an index buffer in Vulkan and D3D12 */
    virtual void            upload_into_index_buffer( uint64_t offset, uint32_t* elements_data, uint64_t size ) final;

    /* This will queue an upload into an index buffer in Vulkan and D3D12 */
    virtual void            upload_queue_into_index_buffer( std::vector<kretash::queue>* queue ) final;

  private:
    
    m_ptr<ID3D12Resource>                           m_vertexBuffer = nullptr;
    D3D12_VERTEX_BUFFER_VIEW                        m_vertexBufferView = {};
    m_ptr<ID3D12Resource>                           m_indexBuffer = nullptr;
    D3D12_INDEX_BUFFER_VIEW                         m_indexBufferView = {};
    uint32_t                                        size_v = 0;
    uint32_t                                        size_i = 0;

  };
}