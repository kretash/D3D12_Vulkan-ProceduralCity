#include "core/texture_manager.hh"
#include "core/texture_generator.hh"
#include "core/engine_settings.hh"
#include "core/texture.hh"
#include "core/engine.hh"
#include "core/vk/context.hh"
#include "core/dx/context.hh"
#include "core/vk/texture.hh"
#include "core/dx/texture.hh"
#include "core/drawable.hh"
#include "core/renderer.hh"
#include "stb/stb_image.h"
#include "core/pool.hh"

#include <algorithm>
#include <cassert>

#define MAX_TEXTURES 1024

#define MAX_UPLOAD_TEXTURES 12
#define MAX_GENERATED_TEXTURES 16
#define MAX_UPGRADE_TEXTURES 16
#define MAX_CLEAR_TEXTURES 32
#define SWAP_UPLOAD_TEXTURES 8

#define LOD_0_THRESHOLD 150.0f
#define LOD_1_THRESHOLD 250.0f
#define LOD_2_THRESHOLD 350.0f
#define LOD_3_THRESHOLD 550.0f

#define ADD_SIZE_MULTIPLIER 16
#define FREE_MEM_SIZE_MULTIPLIER 128
#define IMPROVE_SIZE_MULTIPLIER 32

namespace kretash {

  //TODO
  static const uint64_t LOD0_size = ( uint64_t ) 1024 * ( uint64_t ) 512 * ( uint64_t ) 4;
  static const uint64_t LOD1_size = ( uint64_t ) 512 * ( uint64_t ) 256 * ( uint64_t ) 4;
  static const uint64_t LOD2_size = ( uint64_t ) 256 * ( uint64_t ) 128 * ( uint64_t ) 4;
  static const uint64_t LOD3_size = ( uint64_t ) 128 * ( uint64_t ) 64 * ( uint64_t ) 4;

  TextureManager::TextureManager() {
    m_texture_generator = std::make_shared<TextureGenerator>();

    for( int i = 0; i < MAX_TEXTURES; ++i )
      m_free_ids.push_back( i );

    m_device_memory_free = k_engine->get_device_pool_size();
    m_host_visible_memory_free = k_engine->get_host_visible_pool_size();

  }

  void TextureManager::link_drawable( Drawable* d ) {
    m_drawables.push_back( d );
  }

  void TextureManager::prepare() {

    xxContext* m_context = k_engine->get_context();

    if( nullptr != k_engine->get_renderer( rTEXTURE ) ) {
      m_context->create_srv_view_heap( k_engine->get_renderer( rTEXTURE )->get_renderer() );
    } else {
      std::cout << "no texture renderer available \n";
      return;
    }

    m_context->reset_texture_command_list();

    _generate_placeholder_texture();

    int32_t texture_start = tDIFFUSE;
    int32_t texture_end = tCOUNT;

    for( int32_t i = 0; i < m_drawables.size(); ++i ) {
      if( m_drawables[i]->get_texture()->get_type() == tFILE_TEXTURE ) {

        Texture* c_t = m_drawables[i]->get_texture();

        for( int32_t e = texture_start; e < texture_end; ++e ) {

          bool found = false;
          texture_t tt = static_cast< texture_t >( e );

          for( int e = 0; e < m_texture_db.size(); ++e ) {
            if( c_t->get_filename( tt ) == m_texture_db[e].filename ) {
              c_t->set_id( tt, m_texture_db[e].id );
              found = true;
              break;
            }
          }

          if( found ) continue;

          std::string path = TPATH;
          path.append( c_t->get_filename( tt ) );

          unsigned char* image = stbi_load( path.c_str(),
            c_t->get_width_ref( tt ), c_t->get_height_ref( tt ), c_t->get_channels_ref( tt ), 4 );

          assert( image != nullptr && "IMAGE NOT FOUND" );

          c_t->new_texture( tt );
          c_t->copy_data( tt, image );
          stbi_image_free( image );

          int32_t w = c_t->get_width( tt );
          int32_t h = c_t->get_height( tt );
          int32_t c = c_t->get_channels( tt );

          c_t->get_texture( tt )->create_texture( c_t->get_texture_pointer( tt ), w, h, c );

          auto id_begin = m_free_ids.begin();
          int32_t offset = *id_begin._Ptr;
          m_free_ids.erase( id_begin );

          c_t->get_texture( tt )->create_shader_resource_view( k_engine->get_renderer( rTEXTURE )->get_renderer(),
            offset, 4 );

          c_t->set_id( tt, offset );
          m_texture_db.push_back( texture_db( c_t->get_filename( tt ), offset ) );

        }
      } else if( m_drawables[i]->get_texture()->get_type() == tPROCEDURAL_TEXTURE ) {

        m_non_textured_drawables.push_back( m_drawables[i] );

        m_drawables[i]->get_texture()->set_placeholder(
          m_placeholder_texture->get_id( tDIFFUSE ),
          m_placeholder_texture->get_id( tNORMAL ),
          m_placeholder_texture->get_id( tSPECULAR ) );

      }
    }

    m_context->compute_texture_upload();
    m_context->wait_for_texture_upload();

    m_placeholder_texture->delete_texture( tDIFFUSE );
    m_placeholder_texture->delete_texture( tNORMAL );
    m_placeholder_texture->delete_texture( tSPECULAR );

    m_exit_thread.store( false );
    m_upload_textures.store( false );
    m_threads.push_back( std::thread( &TextureManager::_upload_generated_textures, this ) );
  }

  //should be synced
  void TextureManager::regenerate() {

    m_exit_thread.store( true );
    m_threads[0].join();
    m_threads.clear();

    m_texture_generator->shutdown();
    m_texture_generator = nullptr;
    m_texture_generator = std::make_shared<TextureGenerator>();

    m_texture_db.clear();

    m_device_memory_free = k_engine->get_device_pool_size();
    m_host_visible_memory_free = k_engine->get_host_visible_pool_size();

    xxContext* m_context = k_engine->get_context();
    if( nullptr != k_engine->get_renderer( rTEXTURE ) ) {
      m_context->create_srv_view_heap( k_engine->get_renderer( rTEXTURE )->get_renderer() );
    } else {
      std::cout << "no texture renderer available \n";
      return;
    }

    m_context->reset_texture_command_list();

    m_placeholder_texture->clear();
    m_placeholder_texture->clear_upload();
    m_placeholder_texture->delete_texture( tDIFFUSE );
    m_placeholder_texture->delete_texture( tNORMAL );
    m_placeholder_texture->delete_texture( tSPECULAR );
    m_placeholder_texture->new_texture( tDIFFUSE );
    m_placeholder_texture->new_texture( tNORMAL );
    m_placeholder_texture->new_texture( tSPECULAR );

    m_texture_generator->generate( m_placeholder_texture.get() );
    while( !m_texture_generator->texture_ready( m_placeholder_texture.get() ) ){
      std::this_thread::sleep_for( std::chrono::milliseconds( 2 ) );      
    }

    int32_t base_upload = 0;
    int32_t upload_limit = 0;

    for( int32_t i = base_upload; i < m_textured_drawables.size(); ++i ) {

      Texture* c_t = m_textured_drawables[i]->get_texture();

      if( c_t->get_type() == tPROCEDURAL_TEXTURE ) {

        c_t->clear();
        c_t->clear_upload();
        c_t->delete_texture( tDIFFUSE );
        c_t->delete_texture( tNORMAL );
        c_t->delete_texture( tSPECULAR );

        c_t->new_texture( tDIFFUSE );
        c_t->new_texture( tNORMAL );
        c_t->new_texture( tSPECULAR );

        m_texture_generator->generate( c_t );

        if( upload_limit++ > SWAP_UPLOAD_TEXTURES ) break;
      }
    }

    for( int32_t e = 0; e < m_drawables.size(); ++e ) {
      if( m_drawables[e]->get_texture()->get_type() == tFILE_TEXTURE ) {

        Texture* c_t = m_drawables[e]->get_texture();
        int32_t texture_start = tDIFFUSE;
        int32_t texture_end = tCOUNT;

        for( int32_t o = texture_start; o < texture_end; ++o ) {
          bool found = false;
          texture_t tt = static_cast< texture_t >( o );

          for( int u = 0; u < m_texture_db.size(); ++u ) {
            if( c_t->get_filename( tt ) == m_texture_db[u].filename ) {
              c_t->set_id( tt, m_texture_db[u].id );
              found = true;
              break;
            }
          }

          if( found ) continue;

          std::string path = TPATH;
          path.append( c_t->get_filename( tt ) );

          unsigned char* image = stbi_load( path.c_str(),
            c_t->get_width_ref( tt ), c_t->get_height_ref( tt ), c_t->get_channels_ref( tt ), 4 );

          assert( image != nullptr && "IMAGE NOT FOUND" );

          //could just remove both, but I think this looks more clear
          c_t->delete_texture( tt );
          c_t->new_texture( tt );

          c_t->copy_data( tt, image );
          stbi_image_free( image );

          int32_t w = c_t->get_width( tt );
          int32_t h = c_t->get_height( tt );
          int32_t c = c_t->get_channels( tt );

          c_t->get_texture( tt )->create_texture( c_t->get_texture_pointer( tt ), w, h, c );

          c_t->get_texture( tt )->create_shader_resource_view(
            k_engine->get_renderer( rTEXTURE )->get_renderer(), c_t->get_id( tt ), 4 );

          m_texture_db.push_back( texture_db( c_t->get_filename( tt ), c_t->get_id( tt ) ) );
        }
      }
    }

    m_texture_generator->gather_texture( m_placeholder_texture.get() );

    //upload_limit = base_upload;
    upload_limit = 0;

    for( int i = base_upload; i < m_textured_drawables.size(); ++i ) {

      Texture* c_t = m_textured_drawables[i]->get_texture();

      if( c_t->get_type() == tPROCEDURAL_TEXTURE ) {
        m_texture_generator->gather_texture( c_t );
        m_clean_up_textures.push_back( c_t );
        if( upload_limit++ > SWAP_UPLOAD_TEXTURES ) break;
      } else {

      }
    }

    base_upload += upload_limit;
    upload_limit = 0;

    m_context->compute_texture_upload();
    m_context->wait_for_texture_upload();
    _clean_up_textures();

    while( true ) {
      m_context->reset_texture_command_list();
      std::vector<Texture*> to_gather;

      for( int i = base_upload; i < m_textured_drawables.size(); ++i ) {

        Texture* c_t = m_textured_drawables[i]->get_texture();

        if( c_t->get_type() == tPROCEDURAL_TEXTURE ) {

          c_t->clear();
          c_t->clear_upload();
          c_t->delete_texture( tDIFFUSE );
          c_t->delete_texture( tNORMAL );
          c_t->delete_texture( tSPECULAR );

          c_t->new_texture( tDIFFUSE );
          c_t->new_texture( tNORMAL );
          c_t->new_texture( tSPECULAR );

          m_texture_generator->generate( c_t );
          to_gather.push_back( c_t );

          if( upload_limit++ > SWAP_UPLOAD_TEXTURES ) break;
        }
      }
      
      for( int i = 0; i < to_gather.size(); ++i ) {

          Texture* c_t = to_gather[i];
          m_texture_generator->gather_texture( c_t );
          m_clean_up_textures.push_back( c_t );

      }

      to_gather.clear();
      base_upload += upload_limit;
      upload_limit = 0;

      m_context->compute_texture_upload();
      m_context->wait_for_texture_upload();
      _clean_up_textures();
      
      if( base_upload >= m_textured_drawables.size() ){
        break;
      }
    }

    m_exit_thread.store( false );
    m_upload_textures.store( false );
    m_threads.push_back( std::thread( &TextureManager::_upload_generated_textures, this ) );
  }

  void TextureManager::_generate_placeholder_texture() {

    m_placeholder_texture = std::make_shared<Texture>();

    m_placeholder_texture->init_procedural( 0.0f, 0.0f, 5 );

    m_placeholder_texture->set_future_id( tDIFFUSE, _get_new_id() );
    m_placeholder_texture->set_future_id( tNORMAL, _get_new_id() );
    m_placeholder_texture->set_future_id( tSPECULAR, _get_new_id() );

    m_placeholder_texture->new_texture( tDIFFUSE );
    m_placeholder_texture->new_texture( tNORMAL );
    m_placeholder_texture->new_texture( tSPECULAR );

    m_texture_generator->generate( m_placeholder_texture.get() );
    m_texture_generator->gather_texture( m_placeholder_texture.get() );

  }

  void TextureManager::update() { //1.2562

    static bool update_current_textures = false;
    update_current_textures = !update_current_textures;

    _clean_up_textures();

    if( !update_current_textures ) {
      m_upload_textures.store( true );
    } else {

      _sort_vectors(); //0.01998

      _look_for_upload_textures(); //0.0013462

      _look_for_upgrade_textures();//0.0061308

      bool free_memory = m_host_visible_memory_free > LOD0_size * FREE_MEM_SIZE_MULTIPLIER;
      free_memory &= m_device_memory_free > LOD0_size * FREE_MEM_SIZE_MULTIPLIER;

      if( !free_memory || m_free_ids.size() < 256 ) {
        _clear_deprecated_textures(); //0.027926
      }
    }

  }

  void TextureManager::_clean_up_textures(){
    xxContext* m_context = k_engine->get_context();

    for( int i = 0; i < m_clean_up_textures.size(); ++i ) {

      if( m_clean_up_textures[i]->get_LOD() == 0 ) {
        m_host_visible_memory_free -= LOD0_size;
      } else if( m_clean_up_textures[i]->get_LOD() == 1 ) {
        m_host_visible_memory_free -= LOD1_size;
      } else if( m_clean_up_textures[i]->get_LOD() == 2 ) {
        m_host_visible_memory_free -= LOD2_size;
      } else if( m_clean_up_textures[i]->get_LOD() == 3 ) {
        m_host_visible_memory_free -= LOD3_size;
      }

      m_clean_up_textures[i]->clear_upload();

      m_clean_up_textures[i]->delete_texture( tDIFFUSE );
      m_clean_up_textures[i]->delete_texture( tNORMAL );
      m_clean_up_textures[i]->delete_texture( tSPECULAR );
    }

    if( m_clean_up_textures.size() != 0 ) {
      m_context->defrag_host_memory_pool();
    }

    m_clean_up_textures.clear();
  }

  void TextureManager::synch() {
    if( m_upload_textures.load() == true ) {
      //std::cout << "Waiting for textures.\n";
      while( m_upload_textures.load() ) { /*wait*/ }
    }
  }

  void TextureManager::_upload_generated_textures() {
    while( !m_exit_thread.load() ) {

      if( m_upload_textures.load() ) {
        if( m_loading_textures.size() != 0 ) {

          bool needs_execute = false;
          for( auto i = m_loading_textures.begin(); i != m_loading_textures.end(); ++i ) {
            if( m_texture_generator->texture_ready( ( *i._Ptr )->get_texture() ) ) {
              needs_execute = true;
              break;
            }
          }

          if( needs_execute ) {

            int32_t uploaded = 0;
            xxContext* m_context = k_engine->get_context();

            m_context->reset_texture_command_list();

            {
              auto i = m_loading_textures.begin();
              while( i != m_loading_textures.end() ) {

                if( m_texture_generator->texture_ready( ( *i._Ptr )->get_texture() ) ) {

                  m_texture_generator->gather_texture( ( *i._Ptr )->get_texture() );

                  m_textured_drawables.push_back( *i._Ptr );
                  m_clean_up_textures.push_back( ( *i._Ptr )->get_texture() );

                  i = m_loading_textures.erase( i );
                  uploaded++;

                } else {
                  ++i;
                }

                if( m_loading_textures.size() == 0 ) break;
                if( uploaded == MAX_UPLOAD_TEXTURES ) break;
              }

              m_context->compute_texture_upload();
              m_context->wait_for_texture_upload();

            }
          }
        }
        m_upload_textures.store( false );
      } else {
        std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
      }

    }
  }

  void TextureManager::_sort_vectors() {
    {
      auto comp = [] ( Drawable* a, Drawable* b ) {
        bool a_active = a->get_active();
        bool b_active = b->get_active();

        bool close = a->get_distance() < LOD_1_THRESHOLD || b->get_distance() < LOD_1_THRESHOLD;

        if( a_active && b_active || !a_active && !b_active || close )
          return a->get_distance() < b->get_distance();

        return a_active;
      };

      std::sort( m_non_textured_drawables.begin(), m_non_textured_drawables.end(), comp );
    }
    {
      //Sort them by least render possible
      auto comp = [] ( Drawable* a, Drawable* b ) {
        bool a_active = a->get_active();
        bool b_active = b->get_active();

        if( a_active && b_active || !a_active && !b_active )
          return a->get_distance() > b->get_distance();

        return b_active;
      };

      std::sort( m_textured_drawables.begin(), m_textured_drawables.end(), comp );
    }
  }

  void TextureManager::_look_for_upload_textures() {
    // load from non_textured
    for( int32_t i = 0; i < MAX_GENERATED_TEXTURES; ++i ) {

      if( m_non_textured_drawables.size() == 0 ) break;
      if( static_cast<int32_t>(m_non_textured_drawables.size() ) <= i ) break;
      if( m_non_textured_drawables[0]->get_distance() == 99999.9f ) break;

      Texture* c_t = m_non_textured_drawables[i]->get_texture();
      int32_t LOD = c_t->get_LOD();
      uint64_t size = 0;

      if( m_non_textured_drawables[i]->get_distance() < LOD_0_THRESHOLD ) {
        LOD = 0;
        size = LOD0_size;
      } else if( m_non_textured_drawables[i]->get_distance() < LOD_1_THRESHOLD ) {
        LOD = 1;
        size = LOD1_size;
      } else if( m_non_textured_drawables[i]->get_distance() < LOD_2_THRESHOLD ) {
        LOD = 2;
        size = LOD2_size;
      } else if( m_non_textured_drawables[i]->get_distance() < LOD_3_THRESHOLD ) {
        LOD = 3;
        size = LOD3_size;
      }

      bool free_memory = m_host_visible_memory_free > size * ADD_SIZE_MULTIPLIER;
      free_memory &= m_device_memory_free > size * ADD_SIZE_MULTIPLIER;

      bool free_ids = m_free_ids.size() > 2;
      bool close = m_non_textured_drawables[i]->get_distance() < LOD_1_THRESHOLD;
      bool active = m_non_textured_drawables[i]->get_active();
      
      if( free_memory && free_ids && ( close || active ) ) {

        Texture* c_t = m_non_textured_drawables[i]->get_texture();

        c_t->set_LOD( LOD );
        c_t->set_future_id( tDIFFUSE, _get_new_id() );
        c_t->set_future_id( tNORMAL, _get_new_id() );
        c_t->set_future_id( tSPECULAR, _get_new_id() );

        c_t->new_texture( tDIFFUSE );
        c_t->new_texture( tNORMAL );
        c_t->new_texture( tSPECULAR );

        m_texture_generator->generate( c_t );

        m_loading_textures.push_back( m_non_textured_drawables[i] );
        m_non_textured_drawables.erase( m_non_textured_drawables.begin() + i );

        m_host_visible_memory_free -= LOD3_size;
        m_device_memory_free -= LOD3_size;

      } else {
        break;
      }
    }
  }

  void TextureManager::_look_for_upgrade_textures() {

    //int32_t upgraded = 0;
    if( m_textured_drawables.size() == 0 ) return;

    for( int32_t i = static_cast< int32_t >( m_textured_drawables.size() ) - 1; i >= 0; --i ) {

      Texture* c_t = m_textured_drawables[i]->get_texture();
      int32_t LOD = c_t->get_LOD();//could be initialized to 0
      uint64_t size = 0;

      if( m_textured_drawables[i]->get_distance() < LOD_0_THRESHOLD ) {
        LOD = 0;
        size = LOD0_size;
      } else if( m_textured_drawables[i]->get_distance() < LOD_1_THRESHOLD ) {
        LOD = 1;
        size = LOD1_size;
      } else if( m_textured_drawables[i]->get_distance() < LOD_2_THRESHOLD ) {
        LOD = 2;
        size = LOD2_size;
      } else if( m_textured_drawables[i]->get_distance() < LOD_3_THRESHOLD ) {
        LOD = 3;
        size = LOD3_size;
      }

      bool free_memory = m_host_visible_memory_free > size * IMPROVE_SIZE_MULTIPLIER;
      free_memory &= m_device_memory_free > size * IMPROVE_SIZE_MULTIPLIER;

      if( free_memory && LOD != c_t->get_LOD() && m_textured_drawables[i]->get_active() ) {

        c_t->set_LOD( LOD );
        c_t->new_texture( tDIFFUSE );
        c_t->new_texture( tNORMAL );
        c_t->new_texture( tSPECULAR );

        m_texture_generator->generate( c_t );

        m_loading_textures.push_back( m_textured_drawables[i] );
        m_textured_drawables.erase( m_textured_drawables.begin() + i );

        m_host_visible_memory_free -= size;
        m_device_memory_free -= size;

      }
    }
  }

  void TextureManager::_clear_deprecated_textures() {

    int32_t cleared = 0;

    auto i = m_textured_drawables.begin();
    while( i != m_textured_drawables.end() ) {

      Texture* t = ( *i._Ptr )->get_texture();

      bool close = ( *i._Ptr )->get_distance() < LOD_1_THRESHOLD;
      bool active = ( *i._Ptr )->get_active();

      if( close || active ) break;

      if( t->get_LOD() == 0 ) {
        m_device_memory_free -= LOD0_size;
      } else if( t->get_LOD() == 1 ) {
        m_device_memory_free -= LOD1_size;
      } else if( t->get_LOD() == 2 ) {
        m_device_memory_free -= LOD2_size;
      } else if( t->get_LOD() == 3 ) {
        m_device_memory_free -= LOD3_size;
      }

      t->get_texture( tDIFFUSE )->clear_descriptor_set( m_placeholder_texture->get_texture( tDIFFUSE ), t->get_id( tDIFFUSE ) );
      t->get_texture( tNORMAL )->clear_descriptor_set( m_placeholder_texture->get_texture( tNORMAL ), t->get_id( tNORMAL ) );
      t->get_texture( tSPECULAR )->clear_descriptor_set( m_placeholder_texture->get_texture( tSPECULAR ), t->get_id( tSPECULAR ) );

      ( *i._Ptr )->get_texture()->clear();

      m_free_ids.push_back( ( *i._Ptr )->get_texture()->get_id( tDIFFUSE ) );
      m_free_ids.push_back( ( *i._Ptr )->get_texture()->get_id( tNORMAL ) );
      m_free_ids.push_back( ( *i._Ptr )->get_texture()->get_id( tSPECULAR ) );


      ( *i._Ptr )->get_texture()->set_placeholder(
        m_placeholder_texture->get_id( tDIFFUSE ),
        m_placeholder_texture->get_id( tNORMAL ),
        m_placeholder_texture->get_id( tSPECULAR ) );

      m_non_textured_drawables.push_back( ( *i._Ptr ) );
      i = m_textured_drawables.erase( i );
      ++cleared;

      if( cleared == MAX_CLEAR_TEXTURES ) break;

    }

    xxContext* m_context = k_engine->get_context();
    m_context->defrag_device_memory_pool();

  }

  int32_t TextureManager::_get_new_id() {
    assert( m_free_ids.size() != 0 && "DONT ASK ME FOR AN ID, IM ALL OUT!" );
    int32_t ID = *m_free_ids.begin()._Ptr;
    m_free_ids.erase( m_free_ids.begin() );
    return ID;
  }

  void TextureManager::shutdown() {
    m_texture_generator->shutdown();
  }

  TextureManager::~TextureManager() {
    m_exit_thread.store( true );
    for( int i = 0; i < m_threads.size(); ++i )
      m_threads[i].join();
  }
}