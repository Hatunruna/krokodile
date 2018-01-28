#ifndef KKD_KONAMIGAMEPADCONTROL_H
#define KKD_KONAMIGAMEPADCONTROL_H

#include <gf/Control.h>
#include <gf/Event.h>
#include <gf/Gamepad.h>

namespace kkd {

  class KonamiGamepadControl : public gf::Control {
    public:
      KonamiGamepadControl();

      virtual void processEvent(const gf::Event& event) override;

    private:
      int m_index;
      enum { Released, Pressed } m_state;
  };
}

#endif // KKD_KONAMIGAMEPADCONTROL_H
