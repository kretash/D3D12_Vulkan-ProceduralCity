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
#include <vector>

namespace FMOD {
  class System;
  class Sound;
  class Channel;
  class DSP;
}

namespace kretash {

  class                 Sound {
  public:
    Sound();
    ~Sound();

    void                play_sound( std::string file );

    //only call one per frame
    void                get_spectrum( std::vector<float>* spectrum_L, std::vector<float>* spectrum_R );
    void                get_smooth_spectrum( std::vector<float>* spectrum_L, std::vector<float>* spectrum_R );
    void                get_smooth_simplified_spectrum( std::vector<float>* spectrum );

  private:

    bool                m_system_started;
    std::vector<float>  m_smooth_spectrum_L;
    std::vector<float>  m_smooth_spectrum_R;

    FMOD::System*       m_system;
    FMOD::Sound*        m_sound;
    FMOD::Channel*      m_channel;
    FMOD::DSP*          m_dsp;
  };
}