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

namespace kretash {

  enum key {
    k_W = 0,
    k_A,
    k_S,
    k_D,
    k_Q,
    k_E,

    k_SPACE,
    k_SHIFT,
    e_EXIT,
    k_F1,
    k_F2,
    k_F4,
    k_F5,

    k_LEFT_MOUSE_BTN,
    k_MID_MOUSE_BTN,
    k_RIGHT_MOUSE_BTN,

    nn_TOTAL_KEYS,
  };

  struct gamepad {
    float thumb_L_side;
    float thumb_L_vert;
    float thumb_R_side;
    float thumb_R_vert;
    float left_trigger;
    float right_trigger;
    WORD buttons;
  };

  struct cursor {
    int change_x;
    int change_y;
    int m_x;
    int m_y;
    int x;
    int y;

    cursor() {
      change_x = 0; change_y = 0; m_x = 0; m_y = 0; x = 0; y = 0;
    }
  };

  class Input {
  public:
    Input();
    ~Input() {}

    void                          update();
    bool                          get_key( key k );
    bool                          stealth_get_key( key k );
    cursor                        get_cursor() { return m_cursor; }
    gamepad                       get_gamepad() { return m_gamepad; }
    void                          set_cursor( RECT screen );
    void                          toggle_focus();
    bool                          has_focus() { return m_focus; }
    static LRESULT CALLBACK       WindowProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );

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
}