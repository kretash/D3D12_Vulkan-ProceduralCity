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

#include "base.hh"

class						            Renderer;
class						            Drawable;
class						            Geometry;

class                       Skydome : public Base {
public:
  Skydome();
  ~Skydome();

  void                      init();
  void                      update();

  Renderer*                 get_renderer() { return m_renderer.get(); }
private:
  std::shared_ptr<Drawable> m_drawable;
  std::shared_ptr<Geometry> m_geometry;
  std::shared_ptr<Renderer> m_renderer;
};