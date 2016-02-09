#include "core/texture_manager.hh"
#include "core/texture_generator.hh"
#include "core/engine_settings.hh"
#include "core/k_graphics.hh"
#include "core/texture.hh"
#include "core/engine.hh"
#include "core/drawable.hh"
#include "core/renderer.hh"
#include "stb/stb_image.h"

#include <algorithm>
#include <cassert>

#define CLEAR_TEXTURE_UPLOAD 0
#define MAX_TEXTURED_DRAWABLES 1024
#define MAX_TEXTURES MAX_TEXTURED_DRAWABLES * 3 + 16

#define MAX_UPLOAD_TEXTURES 8
#define MAX_GENERATED_TEXTURES 16
#define MAX_UPGRADE_TEXTURES 16
#define MAX_CLEAR_TEXTURES 16

#define LOD_0_THRESHOLD 150.0f
#define LOD_1_THRESHOLD 250.0f
#define LOD_2_THRESHOLD 350.0f
#define LOD_3_THRESHOLD 550.0f


TextureManager::TextureManager() {
  m_texture_generator = std::make_shared<TextureGenerator>();

  for( int i = 0; i < MAX_TEXTURES; ++i )
    m_free_ids.push_back( i );

  m_done.store(true);
}

void TextureManager::link_drawable( Drawable* d ) {
  m_drawables.push_back( d );
}

void TextureManager::prepare() {

  GPU::create_srv_view_heap( k_engine->get_renderer( rTEXTURE )->get_render_data(), MAX_TEXTURES );
  GPU::reset_texture_command_list( k_engine->get_engine_data() );

  _generate_placeholder_texture();

  int32_t texture_start = tDIFFUSE;
  int32_t texture_end = tCOUNT;

  for( int32_t i = 0; i < m_drawables.size(); ++i ) {
    if( m_drawables[i]->get_texture()->get_type() == tFILE_TEXTURE ) {

      Texture* c_t = m_drawables[i]->get_texture();
      c_t = m_drawables[i]->get_texture();

      for( int32_t e = texture_start; e < texture_end; ++e ) {

        bool found = false;
        texture_t tt = static_cast< texture_t >( e );

        for( int e = 0; e < m_texture_db.size(); ++e ) {
          if( c_t->get_filename( tt ) == m_texture_db[e].filename ) {
            c_t->set_id( tt, m_texture_db[e].id );
            found = true;
          }
        }

        if( found ) continue;

        std::string path = TPATH;
        path.append( c_t->get_filename( tt ) );

        unsigned char* image = stbi_load( path.c_str(),
          c_t->get_width_ref( tt ), c_t->get_height_ref( tt ), c_t->get_channels_ref( tt ), 4 );

        c_t->new_texture( tt );
        c_t->copy_data( tt, image );
        stbi_image_free( image );

        int32_t w = c_t->get_width( tt );
        int32_t h = c_t->get_height( tt );
        int32_t c = c_t->get_channels( tt );

        GPU::create_texture( k_engine->get_engine_data(), c_t->get_texture_data( tt ),
          c_t->get_texture_pointer( tt ),
          w, h, c );

        auto id_begin = m_free_ids.begin();
        int32_t offset = *id_begin._Ptr;
        m_free_ids.erase( id_begin );

        GPU::create_shader_resource_view( k_engine->get_renderer( rTEXTURE )->get_render_data(),
          c_t->get_texture_data( tt ), offset );

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


  GPU::compute_texture_upload( k_engine->get_engine_data() );
  GPU::wait_for_texture_upload( k_engine->get_engine_data() );

#if CLEAR_TEXTURE_UPLOAD
  for( int i = 0; i < m_drawables.size(); ++i ) {
    for( int type = tDIFFUSE; type < tCOUNT; ++type ) {
      texture_t tt = static_cast< texture_t >( type );
      GPU::clear_texture_upload( m_drawables[i]->get_texture()->get_texture_data( tt ) );
    }
  }
#endif

  m_placeholder_texture->delete_texture( tDIFFUSE );
  m_placeholder_texture->delete_texture( tNORMAL );
  m_placeholder_texture->delete_texture( tSPECULAR );
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

  for( int i = 0; i < m_clean_up_textures.size(); ++i ) {

    m_clean_up_textures[i]->clear_upload();

    m_clean_up_textures[i]->delete_texture( tDIFFUSE );
    m_clean_up_textures[i]->delete_texture( tNORMAL );
    m_clean_up_textures[i]->delete_texture( tSPECULAR );
  }

  m_clean_up_textures.clear();

  if( !update_current_textures ) {
    m_threads.push_back( std::thread( &TextureManager::_upload_generated_textures, this ) );
    //_upload_generated_textures(); //1.226
  } else {

    _sort_vectors(); // 0.01998

    _look_for_upload_textures(); //0.0013462

    _look_for_upgrade_textures();//0.0061308

    if( m_free_ids.size() < 32 ) {
      _clear_deprecated_textures(); // 0.027926
    }
  }

}

void TextureManager::synch() {
  for( int i = 0; i < m_threads.size(); ++i ) {
    m_threads[i].join();
  }
  m_threads.clear();
}

void TextureManager::_upload_generated_textures() {
  m_done.store( false );
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

      GPU::reset_texture_command_list( k_engine->get_engine_data() );

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

        GPU::compute_texture_upload( k_engine->get_engine_data() );
        GPU::wait_for_texture_upload( k_engine->get_engine_data() );
      }
    }
  }
  m_done.store( true );
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
  for( int i = 0; i < MAX_GENERATED_TEXTURES; ++i ) {

    if( m_non_textured_drawables.size() == 0 ) break;

    bool possible = m_free_ids.size() > 2 && m_non_textured_drawables.size() > 0;
    bool close = m_non_textured_drawables[0]->get_distance() < LOD_1_THRESHOLD;
    bool active = m_non_textured_drawables[0]->get_active()
      && m_non_textured_drawables[0]->get_distance() < LOD_3_THRESHOLD;

    if( possible && ( close || active ) ) {

      Texture* c_t = m_non_textured_drawables[0]->get_texture();

      c_t->set_LOD( 3 );
      c_t->set_future_id( tDIFFUSE, _get_new_id() );
      c_t->set_future_id( tNORMAL, _get_new_id() );
      c_t->set_future_id( tSPECULAR, _get_new_id() );

      c_t->new_texture( tDIFFUSE );
      c_t->new_texture( tNORMAL );
      c_t->new_texture( tSPECULAR );

      m_texture_generator->generate( c_t );

      m_loading_textures.push_back( m_non_textured_drawables[0] );
      m_non_textured_drawables.erase( m_non_textured_drawables.begin() );

    } else {
      break;
    }
  }
}

void TextureManager::_look_for_upgrade_textures() {

  int32_t upgraded = 0;
  if( m_textured_drawables.size() == 0 ) return;

  for( int32_t i = static_cast<int32_t>(m_textured_drawables.size()) - 1; i >= 0; --i ) {

    Texture* c_t = m_textured_drawables[i]->get_texture();
    int32_t LOD = c_t->get_LOD();

    if( m_textured_drawables[i]->get_distance() < LOD_0_THRESHOLD )
      LOD = 0;
    else if( m_textured_drawables[i]->get_distance() < LOD_1_THRESHOLD )
      LOD = 1;
    else if( m_textured_drawables[i]->get_distance() < LOD_2_THRESHOLD )
      LOD = 2;
    else if( m_textured_drawables[i]->get_distance() < LOD_3_THRESHOLD )
      LOD = 3;

    if( LOD != c_t->get_LOD() ) {

      c_t->set_LOD( LOD );
      c_t->new_texture( tDIFFUSE );
      c_t->new_texture( tNORMAL );
      c_t->new_texture( tSPECULAR );

      m_texture_generator->generate( c_t );

      m_loading_textures.push_back( m_textured_drawables[i] );
      m_textured_drawables.erase( m_textured_drawables.begin() + i );

      if( upgraded == MAX_UPGRADE_TEXTURES ) break;
    }
  }
}

void TextureManager::_clear_deprecated_textures() {

  int32_t cleared = 0;

  auto i = m_textured_drawables.begin();
  while( i != m_textured_drawables.end() ) {

    if( ( *i._Ptr )->get_active() &&
      ( *i._Ptr )->get_texture()->get_LOD() != 3 ) break;

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
}