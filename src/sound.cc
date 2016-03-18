#include <iostream>
#include <cassert>
#include <string>

#include "FMOD/fmod.hpp"
#include "core/sound.hh"
#include "core/engine_settings.hh"

namespace kretash {

  Sound::Sound() :
    m_system_started( false ),
    m_system( nullptr ),
    m_sound( nullptr ),
    m_channel( nullptr ),
    m_dsp( nullptr ) {

    FMOD_RESULT result = FMOD::System_Create( &m_system );
    assert( result == FMOD_OK && "FMOD ERROR" );

    result = m_system->init( 32, FMOD_INIT_NORMAL, nullptr );
    assert( result == FMOD_OK && "FMOD ERROR" );
  }

  void Sound::play_sound( std::string file ) {

    FMOD_RESULT result = m_system->createStream( ( SOUNDPATH + file ).c_str(), FMOD_LOOP_NORMAL | FMOD_2D, 0, &m_sound );
    assert( result != FMOD_ERR_FILE_NOTFOUND && "FMOD FILE NOT FOUND" );
    assert( result == FMOD_OK && "FMOD ERROR" );

    result = m_system->playSound( m_sound, 0, false, &m_channel );
    assert( result == FMOD_OK && "FMOD ERROR" );

    m_system->createDSPByType( FMOD_DSP_TYPE_FFT, &m_dsp );

    m_channel->addDSP( 0, m_dsp );

    FMOD_DSP_PARAMETER_FFT *fftparameter;
    char s[256];
    uint32_t len;

    result = m_dsp->getParameterData( FMOD_DSP_FFT_SPECTRUMDATA, ( void ** ) &fftparameter, &len, s, 256 );
    assert( result == FMOD_OK && "FMOD ERROR" );

    m_system_started = true;
    m_smooth_spectrum_L.resize( len );
    m_smooth_spectrum_R.resize( len );
  }

  void Sound::get_spectrum( std::vector<float>* spectrum_L, std::vector<float>* spectrum_R ) {
    if( m_system_started ) {
      FMOD_DSP_PARAMETER_FFT *fftparameter;
      char s[256];
      uint32_t len;

      FMOD_RESULT result = m_dsp->getParameterData( FMOD_DSP_FFT_SPECTRUMDATA, ( void ** ) &fftparameter, &len, s, 256 );
      assert( result == FMOD_OK && "FMOD ERROR" );

      for( uint32_t i = 0; i < len; ++i ) {
        spectrum_L->push_back( fftparameter->spectrum[0][i] );
        spectrum_R->push_back( fftparameter->spectrum[1][i] );
      }
    }
  }

  void Sound::get_smooth_spectrum( std::vector<float>* spectrum_L, std::vector<float>* spectrum_R ) {
    if( m_system_started ) {
      FMOD_DSP_PARAMETER_FFT *fftparameter;
      char s[256];
      uint32_t len;

      FMOD_RESULT result = m_dsp->getParameterData( FMOD_DSP_FFT_SPECTRUMDATA, ( void ** ) &fftparameter, &len, s, 256 );
      assert( result == FMOD_OK && "FMOD ERROR" );

      float mantain = 0.7f;
      float incorporate = 1.0f - mantain;

      for( uint32_t i = 0; i < len; ++i ) {
        m_smooth_spectrum_L[i] = m_smooth_spectrum_L[i] * mantain + fftparameter->spectrum[0][i] * incorporate;
        spectrum_L->push_back( m_smooth_spectrum_L[i] );

        m_smooth_spectrum_R[i] = m_smooth_spectrum_R[i] * mantain + fftparameter->spectrum[1][i] * incorporate;
        spectrum_R->push_back( m_smooth_spectrum_R[i] );
      }
    }
  }

  void Sound::get_smooth_simplified_spectrum( std::vector<float>* spectrum ) {
    if( m_system_started ) {

      spectrum->clear();
      std::vector<float> channel_L;
      std::vector<float> channel_R;
      this->get_smooth_spectrum( &channel_L, &channel_R );

      assert( channel_L.size() == channel_R.size() && "SOUND ERROR" );

      int32_t pack_together = ( int32_t ) channel_L.size() / 10;

      int32_t count = 0;
      for( int32_t i = 0; i < 9; ++i ) {

        float acumulative = 0.0f;

        for( int32_t e = 0; e < pack_together; ++e ) {
          acumulative += channel_L[count];
          acumulative += channel_R[count];
          ++count;
        }

        spectrum->push_back( acumulative / ( float ) pack_together );
      }

      int32_t last_bit = ( int32_t ) channel_L.size() - count;
      float acumulative = 0.0f;

      for( int32_t i = 0; i < last_bit; ++i ) {
        acumulative += channel_L[count];
        acumulative += channel_R[count];
        ++count;
      }

      spectrum->push_back( acumulative / ( float ) last_bit );
    } else {

      for( int32_t i = 0; i < 10; ++i ) {
        spectrum->push_back( 0.0f );
      }
    }
  }

  Sound::~Sound() {

  }
}