#include "core/dx/drawable.hh"

namespace kretash{

  dxDescriptorBuffer::dxDescriptorBuffer(){
    m_constant_buffer = nullptr;
    m_constant_buffer_WO = nullptr;
    m_constant_buffer_desc = {};
  }
  
  dxDescriptorBuffer::~dxDescriptorBuffer() {
    m_constant_buffer = nullptr;
    m_constant_buffer_WO = nullptr;
    m_constant_buffer_desc = {};
  }

}