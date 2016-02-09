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
#include "base.hh"
#include "k_graphics.hh"

class                     World : public Base {
public:
  World();
  ~World();

  void                    init();
  void                    create_buffer( renderer_data* r_data, int32_t offset );
  void                    update();

  constant_buffer_data*   get_buffer_data() { return &m_buffer_data; }
  constant_buffer*        get_buffer() { return &m_buffer; }

private:
  constant_buffer_data    m_buffer_data;
  constant_buffer         m_buffer;
};