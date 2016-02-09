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
#include <string>

#include "engine.hh"

class               Geometry {
public:
  Geometry();
  Geometry( const Geometry* c );
  ~Geometry();

  void              load( std::string filename );

  void              set_vertex_offset( int32_t v ) { m_vertex_offset = v; }
  void              set_index_offset( uint32_t i ) { m_indicies_offset = i; }
  void              set_indicies_count( uint32_t c ) { m_indicies_count = c; }

  uint32_t          get_indicies_count() { return m_indicies_count; }
  uint32_t          get_indicies_offset() { return m_indicies_offset; }
  int               get_vertex_offset() { return m_vertex_offset; }
protected:
  uint32_t          m_indicies_count;
  uint32_t          m_indicies_offset;
  int32_t           m_vertex_offset;
};