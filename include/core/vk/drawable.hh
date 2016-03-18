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
#include "vulkan/vulkan.h"
#include "vulkan/vulkantools.h"
#include "vulkan/vulkanswapchain.hpp"
#include "core/math/float4x4.hh"

namespace                                           kretash {
  
  class                                             vkDescriptorBuffer : public virtual xxDescriptorBuffer {
  public:
    friend class                                    vkContext;
    friend class                                    vkRenderer;

    vkDescriptorBuffer();
    ~vkDescriptorBuffer();

    /* needs at least one function to be polymorphic */
    virtual void                                    do_nothing() final {};

  private:
    VkBuffer                                        m_buffer = VK_NULL_HANDLE;
    VkDeviceMemory                                  m_memory = VK_NULL_HANDLE;
    VkDescriptorBufferInfo                          m_descriptor = {};
  };


  class                                             vkDrawable : public virtual xxDrawable {
  public:
    friend class                                    vkContext;
    friend class                                    vkRenderer;

    vkDrawable();
    ~vkDrawable();

    /* needs at least one function to be polymorphic */
    virtual void                                    do_nothing() final {};

  private:
    VkDescriptorSet                                 m_descriptor_set = VK_NULL_HANDLE;

  };
}