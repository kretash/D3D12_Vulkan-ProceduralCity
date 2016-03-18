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
#include "core/types.hh"
#include "core/xx/drawable.hh"
#include "core/math/float4x4.hh"
#include <wrl.h>
#include "d3dx12.h"
#include <d3d12.h>
#include <dxgi1_4.h>
#include <d3dcompiler.h>

#define m_ptr Microsoft::WRL::ComPtr

namespace                                           kretash {

  class                                             dxDescriptorBuffer : public virtual xxDescriptorBuffer {
  public:
    friend class                                    dxContext;
    friend class                                    dxRenderer;

    dxDescriptorBuffer();
    ~dxDescriptorBuffer();

    /* needs at least one function to be polymorphic */
    virtual void                                    do_nothing() final {};

  private:
    m_ptr<ID3D12Resource>                           m_constant_buffer = nullptr;
    UINT8*                                          m_constant_buffer_WO = nullptr;
    D3D12_CONSTANT_BUFFER_VIEW_DESC                 m_constant_buffer_desc = {};
  };


  class                                             dxDrawable : public virtual xxDrawable {
  public:
    friend class                                    dxContext;
    friend class                                    dxRenderer;

    dxDrawable(){}
    ~dxDrawable(){}

    /* needs at least one function to be polymorphic */
    virtual void                                    do_nothing() final {};

  private:

  };
}