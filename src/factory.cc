#include "core/factory.h"
#include "core/engine_settings.hh"

#include "core/vk/context.hh"
#include "core/vk/renderer.hh"
#include "core/vk/drawable.hh"
#include "core/vk/geometry.hh"
#include "core/vk/texture.hh"
#include "core/vk/interface.hh"

#include "core/dx/context.hh"
#include "core/dx/renderer.hh"
#include "core/dx/drawable.hh"
#include "core/dx/geometry.hh"
#include "core/dx/texture.hh"
#include "core/dx/interface.hh"

namespace kretash {
  Factory::Factory() {
    m_api = k_engine_settings->get_settings().m_api;
  }

  Factory::~Factory() {}

  void Factory::make_context( std::shared_ptr<xxContext>* c ) {
    if( m_api == kVulkan ) {
      ( *c ) = std::make_shared<vkContext>();
      m_context = c;
    } else if( m_api == kD3D12 ) {
      ( *c ) = std::make_shared<dxContext>();
      m_context = c;
    }
  }

  void Factory::make_renderer( std::shared_ptr<xxRenderer>* r ) {
    if( m_api == kVulkan ) {
      ( *r ) = std::make_shared<vkRenderer>();
      m_renderers.push_back( r );
    } else if( m_api == kD3D12 ) {
      ( *r ) = std::make_shared<dxRenderer>();
      m_renderers.push_back( r );
    }
  }

  void Factory::make_drawable( std::shared_ptr<xxDrawable>* d ) {
    if( m_api == kVulkan ) {
      ( *d ) = std::make_shared<vkDrawable>();
      m_drawables.push_back( d );
    } else if( m_api == kD3D12 ) {
      ( *d ) = std::make_shared<dxDrawable>();
      m_drawables.push_back( d );
    }
  }

  void Factory::make_descriptor_buffer( std::shared_ptr<xxDescriptorBuffer>* db ) {
    if( m_api == kVulkan ) {
      ( *db ) = std::make_shared<vkDescriptorBuffer>();
      m_descriptors.push_back( db );
    } else if( m_api == kD3D12 ) {
      ( *db ) = std::make_shared<dxDescriptorBuffer>();
      m_descriptors.push_back( db );
    }
  }

  void Factory::make_texture( std::shared_ptr<xxTexture>* t ) {
    if( m_api == kVulkan ) {
      ( *t ) = std::make_shared<vkTexture>();
      m_textures.push_back( t );
    } else if( m_api == kD3D12 ) {
      ( *t ) = std::make_shared<dxTexture>();
      m_textures.push_back( t );
    }
  }

  void Factory::make_geometry( std::shared_ptr<xxGeometry>* g ) {
    if( m_api == kVulkan ) {
      ( *g ) = std::make_shared<vkGeometry>();
      m_geometries.push_back( g );
    } else if( m_api == kD3D12 ) {
      ( *g ) = std::make_shared<dxGeometry>();
      m_geometries.push_back( g );
    }
  }

  void Factory::make_interface( std::shared_ptr<xxInterface>* i ) {
    if( m_api == kVulkan ) {
      ( *i ) = std::make_shared<vkInterface>();
      m_interface = ( i );
    } else if( m_api == kD3D12 ) {
      ( *i ) = std::make_shared<dxInterface>();
      m_interface = ( i );
    }
  }

  void Factory::reload() {

    m_api = k_engine_settings->get_settings().m_api;

    if( m_api == kVulkan ) {
      {
        std::shared_ptr<vkInterface> vk_i = std::make_shared<vkInterface>();
        m_interface->swap( static_cast< std::shared_ptr<xxInterface> >( vk_i ) );
      }

      for( int i = 0; i < m_renderers.size(); ++i ) {

        std::shared_ptr<vkRenderer> vk_r = std::make_shared<vkRenderer>();
        m_renderers[i]->swap( static_cast< std::shared_ptr<xxRenderer> >( vk_r ) );

      }
      for( int i = 0; i < m_drawables.size(); ++i ) {

        std::shared_ptr<vkDrawable> vk_d = std::make_shared<vkDrawable>();
        m_drawables[i]->swap( static_cast< std::shared_ptr<xxDrawable> >( vk_d ) );

      }
      for( int i = 0; i < m_descriptors.size(); ++i ) {

        std::shared_ptr<vkDescriptorBuffer> vk_db = std::make_shared<vkDescriptorBuffer>();
        m_descriptors[i]->swap( static_cast< std::shared_ptr<xxDescriptorBuffer> >( vk_db ) );

      }
      for( int i = 0; i < m_textures.size(); ++i ) {

        std::shared_ptr<vkTexture> vk_t = std::make_shared<vkTexture>();
        m_textures[i]->swap( static_cast< std::shared_ptr<xxTexture> >( vk_t ) );

      }
      //for( int i = 0; i < m_geometries.size(); ++i ) {

      //  std::shared_ptr<vkGeometry> vk_g = std::make_shared<vkGeometry>();
      //  ( *m_geometries[i] ).swap( static_cast< std::shared_ptr<xxGeometry> >( vk_g ) );

      //}

      std::shared_ptr<vkContext> vk_c = std::make_shared<vkContext>();
      m_context->swap( static_cast< std::shared_ptr<xxContext> >( vk_c ) );


    } else if( m_api == kD3D12 ) {
      {
        std::shared_ptr<dxInterface> dx_i = std::make_shared<dxInterface>();
        m_interface->swap( static_cast< std::shared_ptr<xxInterface> >( dx_i ) );
      }
      for( int i = 0; i < m_renderers.size(); ++i ) {

        std::shared_ptr<dxRenderer> dx_r = std::make_shared<dxRenderer>();
        m_renderers[i]->swap( static_cast< std::shared_ptr<xxRenderer> >( dx_r ) );

      }
      for( int i = 0; i < m_drawables.size(); ++i ) {

        std::shared_ptr<dxDrawable> dx_d = std::make_shared<dxDrawable>();
        m_drawables[i]->swap( static_cast< std::shared_ptr<xxDrawable> >( dx_d ) );

      }
      for( int i = 0; i < m_descriptors.size(); ++i ) {

        std::shared_ptr<dxDescriptorBuffer> dx_db = std::make_shared<dxDescriptorBuffer>();
        m_descriptors[i]->swap( static_cast< std::shared_ptr<xxDescriptorBuffer> >( dx_db ) );

      }
      for( int i = 0; i < m_textures.size(); ++i ) {

        std::shared_ptr<dxTexture> dx_t = std::make_shared<dxTexture>();
        m_textures[i]->swap( static_cast< std::shared_ptr<xxTexture> >( dx_t ) );

      }
      //for( int i = 0; i < m_geometries.size(); ++i ) {

      //  std::shared_ptr<dxGeometry> dx_g = std::make_shared<dxGeometry>();
      //  ( *m_geometries[i] ).swap( static_cast< std::shared_ptr<xxGeometry> >( dx_g ) );

      //}

      std::shared_ptr<dxContext> dx_c = std::make_shared<dxContext>();
      m_context->swap( static_cast< std::shared_ptr<xxContext> >( dx_c ) );

    }
  }
}