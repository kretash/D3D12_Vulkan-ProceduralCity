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

namespace                    kretash {

  class                      xxContext;
  class                      xxRenderer;
  class                      xxDrawable;
  class                      xxDescriptorBuffer;
  class                      xxTexture;
  class                      xxGeometry;
  class                      xxInterface;
  enum                       API;

  class                      Factory {
  public:
    Factory();
    ~Factory();

    void                     make_context( std::shared_ptr<xxContext>* c );
    void                     make_renderer( std::shared_ptr<xxRenderer>* r );
    void                     make_drawable( std::shared_ptr<xxDrawable>* d );
    void                     make_descriptor_buffer( std::shared_ptr<xxDescriptorBuffer>* db );
    void                     make_texture( std::shared_ptr<xxTexture>* t );
    void                     make_geometry( std::shared_ptr<xxGeometry>* g );
    void                     make_interface( std::shared_ptr<xxInterface>* i );
    API                      current_api() { return m_api; }

    void                     reload();

  private:
    API                                                   m_api;
    std::shared_ptr<xxContext>*                           m_context;
    std::shared_ptr<xxInterface>*                         m_interface;
    std::vector<std::shared_ptr<xxRenderer>*>             m_renderers;
    std::vector<std::shared_ptr<xxDrawable>*>             m_drawables;
    std::vector<std::shared_ptr<xxDescriptorBuffer>*>     m_descriptors;
    std::vector<std::shared_ptr<xxTexture>*>              m_textures;
    std::vector<std::shared_ptr<xxGeometry>*>             m_geometries;
  };
}