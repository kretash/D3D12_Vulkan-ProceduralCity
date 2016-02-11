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
#include <vector>
#include <memory>
#include <thread>
#include <atomic>

#include "base.hh"

class                                     Drawable;
class                                     Texture;
class                                     TextureGenerator;
struct                                    texture_db;

class                                     TextureManager : public Base {
public:
  TextureManager();
  ~TextureManager();
  
  void                                    link_drawable( Drawable* d );
  void                                    prepare();
  void                                    update();
  void                                    shutdown();
  void                                    synch();

protected:

private:
  void                                    _generate_placeholder_texture();
  void                                    _upload_generated_textures();
  void                                    _sort_vectors();
  void                                    _look_for_upload_textures();
  void                                    _look_for_upgrade_textures();
  void                                    _clear_deprecated_textures();
  int32_t                                 _get_new_id();

  std::vector<std::thread>                m_threads;
  std::atomic_bool                        m_upload_textures;
  std::atomic_bool                        m_exit_thread;

  std::shared_ptr<Texture>                m_placeholder_texture;
  std::vector<int32_t>                    m_free_ids;
  std::vector<Drawable*>                  m_drawables;
  std::vector<Drawable*>                  m_textured_drawables;
  std::vector<Drawable*>                  m_non_textured_drawables;
  std::vector<Drawable*>                  m_loading_textures;
  std::vector<Texture*>                   m_clean_up_textures;
  std::vector<texture_db>                 m_texture_db;
  std::shared_ptr<TextureGenerator>       m_texture_generator;
};