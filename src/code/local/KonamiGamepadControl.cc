#include "KonamiGamepadControl.h"

#include <cassert>

namespace kkd {

  namespace {

    constexpr int KonamiKeysCount = 10;

    gf::GamepadButton nthKeyForKonami(int index) {
      assert(index < KonamiKeysCount);

      switch (index) {
        case 0:
        case 1:
          return gf::GamepadButton::DPadUp;
        case 2:
        case 3:
          return gf::GamepadButton::DPadDown;
        case 4:
        case 6:
          return gf::GamepadButton::DPadLeft;
        case 5:
        case 7:
          return gf::GamepadButton::DPadRight;
        case 8:
          return gf::GamepadButton::B;
        case 9:
          return gf::GamepadButton::A;
      }

      assert(false);
      return gf::GamepadButton::Invalid;
    }

  }

  KonamiGamepadControl::KonamiGamepadControl()
  : m_index(0)
  , m_state(Released)
  {

  }

  void KonamiGamepadControl::processEvent(const gf::Event& event) {
    if (m_state == Released && event.type == gf::EventType::GamepadButtonPressed){
      if(event.gamepadButton.button == nthKeyForKonami(m_index)) {
        m_state = Pressed;
      } else {
        m_index = 0;
      }
    }

    if (m_state == Pressed && event.type == gf::EventType::GamepadButtonReleased){
      m_state = Released;

      if (event.gamepadButton.button == nthKeyForKonami(m_index)) {
        ++m_index;
      } else {
        m_index = 0;
      }
    }

    if (m_index == KonamiKeysCount) {
      setActive(true);
      m_index = 0;
    } else {
      setActive(false);
    }
  }
}
