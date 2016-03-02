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

#include <Windows.h>
#include <Xinput.h>
#include "types.hh"

#pragma comment(lib, "XInput.lib")
#define PS4_CONTROLLER_GAMEPAD_O XINPUT_GAMEPAD_B
#define PS4_CONTROLLER_GAMEPAD_LEFT_THUMB XINPUT_GAMEPAD_LEFT_THUMB

class Input {
public:
  Input();
  ~Input() {}

  void                          update();
  bool                          get_key( key k ) { return m_event[k]; }
  cursor                        get_cursor() { return m_cursor; }
  gamepad                       get_gamepad(){ return m_gamepad; }
  void                          set_cursor( RECT screen );
  void                          toggle_focus();

private:
  void                          pull_events();
  void                          key_down( WPARAM key );
  void                          key_up( WPARAM key );


  MSG                           m_msg;
  bool                          m_event[nn_TOTAL_KEYS];
  int2                          m_screen_center;
  cursor                        m_cursor;
  bool                          m_focus;
  gamepad                       m_gamepad;
  bool                          m_controller_connected;
  int                           m_controller_num;
  XINPUT_STATE                  m_controller_state;
};