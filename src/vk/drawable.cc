#include "core/vk/drawable.hh"
#include "core/vk/context.hh"
#include "core/engine.hh"

namespace kretash{

  vkDescriptorBuffer::vkDescriptorBuffer() {

    m_descriptor = {};

    if( m_buffer != VK_NULL_HANDLE ) {
      vkContext* m_context = dynamic_cast<vkContext*>( k_engine->get_context() );
      vkDestroyBuffer( m_context->m_device, m_buffer, nullptr );
      m_buffer = VK_NULL_HANDLE;
    }

    if( m_memory != VK_NULL_HANDLE ) {
      vkContext* m_context = dynamic_cast<vkContext*>( k_engine->get_context() );
      vkFreeMemory( m_context->m_device, m_memory, nullptr );
      m_memory = VK_NULL_HANDLE;
    }

  }

  vkDescriptorBuffer::~vkDescriptorBuffer() {
    
    if( m_buffer != VK_NULL_HANDLE ) {
      vkContext* m_context = dynamic_cast<vkContext*>( k_engine->get_context() );
      vkDestroyBuffer( m_context->m_device, m_buffer, nullptr );
      m_buffer = VK_NULL_HANDLE;
    }

    if( m_memory != VK_NULL_HANDLE ) {
      vkContext* m_context = dynamic_cast<vkContext*>( k_engine->get_context() );
      vkFreeMemory( m_context->m_device, m_memory, nullptr );
      m_memory = VK_NULL_HANDLE;
    }

  }



  vkDrawable::vkDrawable() {

    if( m_descriptor_set != VK_NULL_HANDLE ) {
      m_descriptor_set = VK_NULL_HANDLE;
    }

  }

  vkDrawable::~vkDrawable() {

    if( m_descriptor_set != VK_NULL_HANDLE ) {
      m_descriptor_set = VK_NULL_HANDLE;
    }

  }
}