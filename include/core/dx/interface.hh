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
#include "core/xx/interface.hh"
#include "imgui/imgui.h"
#include <wrl.h>
#include "d3dx12.h"
#include <d3d12.h>
#include <dxgi1_4.h>
#include <d3dcompiler.h>

#define m_ptr Microsoft::WRL::ComPtr

namespace                   kretash {

  class                     dxInterface : public virtual xxInterface {
  public:
    dxInterface();
    ~dxInterface();

    /* Initialize the interface in Vulkan and D3D12 */
    virtual void            init( Window* w ) final;

  private:
    static void _render( ImDrawData* draw_data );

    m_ptr<ID3D12DescriptorHeap>             m_srv_heap;
    m_ptr<ID3DBlob>                         vertexShader = nullptr;
    m_ptr<ID3DBlob>                         pixelShader = nullptr;
    m_ptr<ID3D12RootSignature>              m_root_signature = nullptr;
    m_ptr<ID3D12PipelineState>              m_pipeline_state = nullptr;
    m_ptr<ID3D12Resource>                   m_upload_buffer = nullptr;
    m_ptr<ID3D12Resource>                   m_font = nullptr;

  };
}