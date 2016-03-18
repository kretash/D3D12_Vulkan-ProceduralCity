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
#include "core/xx/drawable.hh"

namespace kretash {

  class                     xxDescriptorBuffer;

  class                     World : public Base {
  public:
    World();
    ~World();

    void                    init();
    void                    update();

    xxDescriptorBuffer*     get_buffer() { return m_buffer.get(); }
    constant_buffer*        get_constant_buffer() { return &m_constant_buffer; }
    float*                  get_time() { return &m_time; }
    bool*                   get_stop_time() { return &m_stop_time; }

  private:
    std::shared_ptr<xxDescriptorBuffer>   m_buffer;
    constant_buffer                       m_constant_buffer;
    float                                 m_time = 6.5f;
    bool                                  m_stop_time = false;
  };
}