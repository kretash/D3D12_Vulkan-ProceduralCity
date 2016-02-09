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
#include <atomic>
#include <thread>
#include <string>
#include "k_graphics.hh"

enum texture_type {
  tFILE_TEXTURE = 0,
  tPROCEDURAL_TEXTURE,
  tIGNORE_TEXTURE,
};

enum texture_t {
  tDIFFUSE = 0,
  tNORMAL,
  tSPECULAR,
  tCOUNT,
};

class                           Texture {
public:
  Texture();
  ~Texture();

  void                          init_procedural( float seed_x, float seed_y, uint8_t LOD );
  void                          load( texture_t t, std::string filename );
  void                          set_LOD( uint8_t LOD );

  texture_type                  get_type() { return m_texture_type; }
  uint8_t                       get_LOD() { return m_LOD; }

  void                          new_texture( texture_t t );
  void                          copy_data( texture_t t, void* d );
  void                          delete_texture( texture_t t );
  void                          apply_future_ids();
  void                          clear_upload();
  void                          clear();

  int32_t                       get_width( texture_t t ) { return m_width[t]; }
  int32_t                       get_height( texture_t t ) { return m_height[t]; }
  int32_t                       get_channels( texture_t t ) { return m_channels[t]; }
  int32_t*                      get_width_ref( texture_t t ) { return &m_width[t]; }
  int32_t*                      get_height_ref( texture_t t ) { return &m_height[t]; }
  int32_t*                      get_channels_ref( texture_t t ) { return &m_channels[t]; }

  void*                         get_texture_pointer( texture_t t ) { return m_texture_pointer[t]; }
  texture_data*                 get_texture_data( texture_t t ) { return &m_texture_data[t]; }
  std::string                   get_filename( texture_t t ) { return m_filename[t]; }

  int32_t                       get_id( texture_t t ) { return m_texture_id[t]; }
  void                          set_id( texture_t t, int32_t id ) { m_texture_id[t] = id; }
  void                          set_future_id( texture_t t, int32_t id ) { m_future_texture_id[t] = id; }
  void                          set_placeholder( int32_t d_id, int32_t n_id, int32_t s_id );
  bool                          is_placeholder() { return m_LOD == 5; }

private:
  friend class TextureGenerator;

  //functions for my friend the generator, who deals with textures the same size by default
  int32_t                       _get_width() { return m_width[0]; }
  int32_t                       _get_height() { return m_height[0]; }
  int32_t                       _get_channels() { return m_channels[0]; }

  texture_type                  m_texture_type;
  std::atomic_bool              m_ready;
  std::vector<texture_element>  m_texture_e;
  float                         m_seed_x;
  float                         m_seed_y;

  // 0 - 1024x512  // 1 - 512x256  // 2 - 256x128  // 3 - 128x64 // 4 - custom/load // 5 - placeholder
  uint8_t                       m_LOD;
  int32_t                       m_width[tCOUNT];
  int32_t                       m_height[tCOUNT];
  int32_t                       m_channels[tCOUNT];
  void*                         m_texture_pointer[tCOUNT];
  texture_data                  m_texture_data[tCOUNT];
  int32_t                       m_texture_id[tCOUNT];
  int32_t                       m_future_texture_id[tCOUNT];
  std::string                   m_filename[tCOUNT];
};