#include <Windows.h>
#include <iostream>
#include <memory>
#include <thread>
#include "core/engine_settings.hh"
#include "core/engine.hh"
#include "core/window.hh"
#include "core/input.hh"

namespace kretash {

  Window::Window() {
    m_hwnd = nullptr;
  }

  void Window::init() {
    engine_settings settings = k_engine_settings->get_settings();

    if( m_hwnd != nullptr ) DestroyWindow( m_hwnd );

    m_height = settings.resolution_height;
    m_width = settings.resolution_width;

    m_aspect_ratio = static_cast< float >( m_width ) / static_cast< float >( m_height );
    m_title = L"Carlos Martinez Romero - Computing Project";

    m_viewport.TopLeftX = 0.0f;
    m_viewport.TopLeftY = 0.0f;
    m_viewport.Width = static_cast< float >( m_width );
    m_viewport.Height = static_cast< float >( m_height );
    m_viewport.MinDepth = 0.0f;
    m_viewport.MaxDepth = 1.0f;

    m_scissor_test.top = 0;
    m_scissor_test.left = 0;
    m_scissor_test.right = static_cast< LONG >( m_width );
    m_scissor_test.bottom = static_cast< LONG >( m_height );

    m_window_class = { 0 };
    m_window_class.cbSize = sizeof( WNDCLASSEX );
    m_window_class.style = CS_HREDRAW | CS_VREDRAW;
    m_window_class.lpfnWndProc = &( Input::WindowProc );
    m_window_class.hInstance = GetModuleHandle( 0 );
    m_window_class.hCursor = LoadCursor( NULL, IDC_ARROW );
    m_window_class.lpszClassName = "ClassName1";
    RegisterClassEx( &m_window_class );

    m_window_rect = { 0, 0, m_width, m_height };
    AdjustWindowRect( &m_window_rect, WS_POPUPWINDOW, FALSE );

    RECT desktop;
    GetWindowRect( GetDesktopWindow(), &desktop );

    int32_t x_pos = ( desktop.right - m_width ) / 2;
    int32_t y_pos = ( desktop.bottom - m_height ) / 2;


    m_hwnd = CreateWindowExW( NULL, L"ClassName1", m_title.c_str(),
      WS_POPUPWINDOW, x_pos, y_pos,
      m_window_rect.right - m_window_rect.left,
      m_window_rect.bottom - m_window_rect.top,
      NULL, NULL, GetModuleHandle( 0 ), NULL );

    ShowWindow( m_hwnd, 1 );

  }

  void Window::capture_mouse() {
    RECT desktop;
    GetWindowRect( GetDesktopWindow(), &desktop );
    int32_t x_pos = ( desktop.right - m_width ) / 2;
    int32_t y_pos = ( desktop.bottom - m_height ) / 2;

    RECT screen;
    screen.left = x_pos;
    screen.top = y_pos;
    screen.right = x_pos + m_width;
    screen.bottom = y_pos + m_height;
    k_engine->get_input()->set_cursor( screen );

  }

  Window::~Window() {

  }

}