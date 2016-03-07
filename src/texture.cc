#include "core/engine_settings.hh"
#include "core/k_graphics.hh"
#include "core/texture.hh"
#include "core/engine.hh"
#include "core/texture_manager.hh"

#define STB_IMAGE_IMPLEMENTATION 1
#include "stb/stb_image.h"

Texture::Texture() {
  m_LOD = 4;
  m_texture_type = tIGNORE_TEXTURE;
  for( int i = 0; i < tCOUNT; ++i ) {
    m_width[i] = 0;
    m_height[i] = 0;
    m_channels[i] = 0;
    m_texture_pointer[i] = nullptr;
    m_texture_data[i] = {};
    m_texture_id[i] = 0;
    m_future_texture_id[i] = 0;
    m_filename[i] = "UNINITIALIZED";
  }
  m_ready.store( true );
}

void Texture::init_procedural( float seed_x, float seed_y, uint8_t LOD ) {
  m_seed_x = seed_x;
  m_seed_y = seed_y;
  m_LOD = LOD;
  m_texture_type = tPROCEDURAL_TEXTURE;

  if( LOD == 0 )    m_width[0] = m_width[1] = m_width[2] = 1024;
  if( LOD == 1 )    m_width[0] = m_width[1] = m_width[2] = 512;
  if( LOD == 2 )    m_width[0] = m_width[1] = m_width[2] = 256;
  if( LOD == 3 )    m_width[0] = m_width[1] = m_width[2] = 128;
  if( LOD == 5 )    m_width[0] = m_width[1] = m_width[2] = 1024;

  if( LOD == 0 )    m_height[0] = m_height[1] = m_height[2] = 512;
  if( LOD == 1 )    m_height[0] = m_height[1] = m_height[2] = 256;
  if( LOD == 2 )    m_height[0] = m_height[1] = m_height[2] = 128;
  if( LOD == 3 )    m_height[0] = m_height[1] = m_height[2] = 64;
  if( LOD == 5 )    m_height[0] = m_height[1] = m_height[2] = 512;

  m_channels[0] = m_channels[1] = m_channels[2] = 4;
}

void Texture::set_LOD( uint8_t LOD ) {
  m_LOD = LOD;

  if( LOD == 0 )    m_width[0] = m_width[1] = m_width[2] = 1024;
  if( LOD == 1 )    m_width[0] = m_width[1] = m_width[2] = 512;
  if( LOD == 2 )    m_width[0] = m_width[1] = m_width[2] = 256;
  if( LOD == 3 )    m_width[0] = m_width[1] = m_width[2] = 128;
  if( LOD == 5 )    m_width[0] = m_width[1] = m_width[2] = 1024;

  if( LOD == 0 )    m_height[0] = m_height[1] = m_height[2] = 512;
  if( LOD == 1 )    m_height[0] = m_height[1] = m_height[2] = 256;
  if( LOD == 2 )    m_height[0] = m_height[1] = m_height[2] = 128;
  if( LOD == 3 )    m_height[0] = m_height[1] = m_height[2] = 64;
  if( LOD == 5 )    m_height[0] = m_height[1] = m_height[2] = 512;

  m_channels[0] = m_channels[1] = m_channels[2] = 4;
}

void Texture::load( texture_t t, std::string filename ) {
  m_filename[t] = filename;
  m_texture_type = tFILE_TEXTURE;
}

void Texture::new_texture( texture_t t ) {
  //NOTE: change this for different LODs
  int32_t size = m_width[t] * m_height[t] * m_channels[t];

  if( m_texture_pointer[t] == nullptr )
    m_texture_pointer[t] = new unsigned char[size];
  else
    std::cout << "double_new\n";

  memset( m_texture_pointer[t], size, 0 );
}

void Texture::copy_data( texture_t t, void* d ) {
  memcpy( m_texture_pointer[t], d, m_width[t] * m_height[t] * m_channels[t] );
}

void Texture::delete_texture( texture_t t ) {
  delete[] m_texture_pointer[t];
  m_texture_pointer[t] = nullptr;
}

void Texture::apply_future_ids() {
  for( int i = 0; i < tCOUNT; ++i ) {
    m_texture_id[i] = m_future_texture_id[i];
  }
}

void Texture::clear_upload() {
  GPU::clear_texture_upload( k_engine->get_engine_data(), &m_texture_data[tDIFFUSE] );
  GPU::clear_texture_upload( k_engine->get_engine_data(), &m_texture_data[tNORMAL] );
  GPU::clear_texture_upload( k_engine->get_engine_data(), &m_texture_data[tSPECULAR] );
}

void Texture::clear() {
  GPU::clear_texture( k_engine->get_engine_data(), &m_texture_data[tDIFFUSE] );
  GPU::clear_texture( k_engine->get_engine_data(), &m_texture_data[tNORMAL] );
  GPU::clear_texture( k_engine->get_engine_data(), &m_texture_data[tSPECULAR] );
}

void Texture::set_placeholder( int32_t d_id, int32_t n_id, int32_t s_id ) {
  m_LOD = 5;

  m_texture_id[tDIFFUSE] = d_id;
  m_texture_id[tNORMAL] = n_id;
  m_texture_id[tSPECULAR] = s_id;
}

Texture::~Texture() {
  for( int i = 0; i < tCOUNT; ++i ) {
    if( m_texture_pointer[i] != nullptr )
      delete[] m_texture_pointer[i];
  }
}