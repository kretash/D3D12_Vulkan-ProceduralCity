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
#include <thread>
#include <mutex>
#include <atomic>

class                             Texture;

class                             TextureGenerator {
public:
  TextureGenerator();
  ~TextureGenerator();

  void                            generate( Texture* desc );
  bool                            texture_ready( Texture* desc );
  void                            gather_texture( Texture* desc );
  void                            shutdown();
  
private:
  void                            _thread_generate();
  void                            _generate_elements( Texture* desc );
  void                            _rasterize_elements( Texture* desc );
  void                            _rasterize_diffuse( Texture* desc );
  void                            _rasterize_diffuse_rocks( Texture* desc );
  void                            _rasterize_specular( Texture* desc );
  void                            _generate_normal_maps( Texture* desc );
  void                            _rasterize_debug( Texture* desc );

  std::vector<std::thread>        m_threads;
  std::vector<Texture*>           m_generate_queue;
  std::mutex                      m_queue_mutex;
  std::atomic_bool                m_exit_all_threads;

};