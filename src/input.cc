#include "core/input.hh"
#include "core/tools.hh"
#include <Windows.h>
#include <limits>

Input::Input() {
  for( int i = 0; i < nn_TOTAL_KEYS; ++i ) {
    m_event[i] = false;
  }
  m_focus = false;

  m_controller_connected = false;
  m_controller_num = 0;
  memset( &m_controller_state, 0, sizeof( XINPUT_STATE ) );
  memset( &m_gamepad, 0, sizeof( gamepad ) );
}

void Input::update() {
  pull_events();

  if( m_focus ) {
    POINT cursor_pos = {};
    GetCursorPos( &cursor_pos );

    m_cursor.change_x = m_screen_center.x - cursor_pos.x;
    m_cursor.change_y = m_screen_center.y - cursor_pos.y;

    m_cursor.m_x += m_cursor.change_x;
    m_cursor.m_y += m_cursor.change_y;

    SetCursorPos( m_screen_center.x, m_screen_center.y );

    DWORD result = XInputGetState( m_controller_num, &m_controller_state );
    if( result == ERROR_SUCCESS ) {
      m_controller_connected = true;

#ifdef max
#define tmp_max max
#undef max
#endif

      m_gamepad.thumb_L_side = ( ( float ) m_controller_state.Gamepad.sThumbLX ) /
        std::numeric_limits<short>::max();
      m_gamepad.thumb_L_vert = ( ( float ) m_controller_state.Gamepad.sThumbLY ) /
        std::numeric_limits<short>::max();
      m_gamepad.thumb_R_side = ( ( float ) m_controller_state.Gamepad.sThumbRX ) /
        std::numeric_limits<short>::max();
      m_gamepad.thumb_R_vert = ( ( float ) m_controller_state.Gamepad.sThumbRY ) /
        std::numeric_limits<short>::max();
      m_gamepad.left_trigger = ( ( float ) m_controller_state.Gamepad.bLeftTrigger ) /
        std::numeric_limits<unsigned char>::max();
      m_gamepad.right_trigger = ( ( float ) m_controller_state.Gamepad.bRightTrigger ) /
        std::numeric_limits<unsigned char>::max();

      m_gamepad.thumb_L_side = tools::round_down( m_gamepad.thumb_L_side, 0.05f );
      m_gamepad.thumb_L_vert = tools::round_down( m_gamepad.thumb_L_vert, 0.05f );
      m_gamepad.thumb_R_side = tools::round_down( m_gamepad.thumb_R_side, 0.05f );
      m_gamepad.thumb_R_vert = tools::round_down( m_gamepad.thumb_R_vert, 0.05f );
      m_gamepad.left_trigger = tools::round_down( m_gamepad.left_trigger, 0.05f );
      m_gamepad.right_trigger = tools::round_down( m_gamepad.right_trigger, 0.05f );

      m_gamepad.buttons = m_controller_state.Gamepad.wButtons;

#ifdef tmp_max
#define max tmp_max
#undef tmp_max
#endif

    } else {
      m_controller_connected = false;
      memset( &m_gamepad, 0, sizeof( gamepad ) );
    }
  }
}

void Input::set_cursor( RECT screen ) {
  ShowCursor( FALSE );
  m_focus = true;
  ClipCursor( &screen );
  m_screen_center.x = screen.left + ( screen.right - screen.left ) / 2;
  m_screen_center.y = screen.top + ( screen.bottom - screen.top ) / 2;
  SetCursorPos( m_screen_center.x, m_screen_center.y );
}

void Input::toggle_focus() {
  if( m_focus ) {
    ShowCursor( TRUE );
    m_focus = false;
  } else {
    ShowCursor( FALSE );
    m_focus = true;
  }
}

void Input::pull_events() {
  while( PeekMessage( &m_msg, NULL, 0, 0, PM_REMOVE ) ) {
    TranslateMessage( &m_msg );
    DispatchMessage( &m_msg );

    switch( m_msg.message ) {
    case WM_KEYDOWN:
      key_down( m_msg.wParam );
      break;
    case WM_KEYUP:
      key_up( m_msg.wParam );
      break;
    case WM_QUIT:
      m_event[e_EXIT] = true;
      break;
    }
  }
}

void Input::key_down( WPARAM key ) {
  switch( key ) {
  case 'W':
    m_event[k_W] = true;
    break;
  case 'A':
    m_event[k_A] = true;
    break;
  case 'S':
    m_event[k_S] = true;
    break;
  case 'D':
    m_event[k_D] = true;
    break;
  case 'Q':
    m_event[k_Q] = true;
    break;
  case 'E':
    m_event[k_E] = true;
    break;
  case VK_SPACE:
    m_event[k_SPACE] = true;
    break;
  case VK_SHIFT:
    m_event[k_SHIFT] = true;
    break;
  case VK_ESCAPE:
    m_event[e_EXIT] = true;
    break;
  case VK_F2:
    m_event[k_F2] = true;
    break;
  case VK_F4:
    m_event[k_F4] = true;
    break;
  case VK_F5:
    m_event[k_F5] = true;
    break;
  }
}

void Input::key_up( WPARAM key ) {
  switch( key ) {
  case 'W':
    m_event[k_W] = false;
    break;
  case 'A':
    m_event[k_A] = false;
    break;
  case 'S':
    m_event[k_S] = false;
    break;
  case 'D':
    m_event[k_D] = false;
    break;
  case 'Q':
    m_event[k_Q] = false;
    break;
  case 'E':
    m_event[k_E] = false;
    break;
  case VK_SPACE:
    m_event[k_SPACE] = false;
    break;
  case VK_SHIFT:
    m_event[k_SHIFT] = false;
    break;
  case VK_ESCAPE:
    m_event[e_EXIT] = false;
    break;
  case VK_F2:
    m_event[k_F2] = false;
    break;
  case VK_F4:
    m_event[k_F4] = false;
    break;
  case VK_F5:
    m_event[k_F5] = false;
    break;
  }
}
