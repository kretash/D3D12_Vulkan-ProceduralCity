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
#include <d3d12.h>
#include <Windows.h>

#include "base.hh"

class                   Window : public Base {
public:
  Window();
  ~Window();

  void                  init();

  int32_t               get_height(){ return m_height; }
  int32_t               get_width() { return m_width; }
  float                 get_aspect_ratio(){ return m_aspect_ratio; }
  void                  capture_mouse();

  D3D12_VIEWPORT        get_viewport(){ return m_viewport; }
  D3D12_RECT            get_scissor(){ return m_scissor_test; }
  HWND                  get_window_handle(){ return m_hwnd; }

private:
  int32_t               m_height;
  int32_t               m_width;
  float                 m_aspect_ratio;

  HWND                  m_hwnd;
  std::wstring          m_title;
  D3D12_VIEWPORT        m_viewport;
  D3D12_RECT            m_scissor_test;
  WNDCLASSEX            m_window_class;
  RECT                  m_window_rect;

};