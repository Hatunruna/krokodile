#ifndef KKD_MESSAGES_H
#define KKD_MESSAGES_H

#include <gf/Gamepad.h>
#include <gf/Message.h>

using namespace gf::literals;

namespace kkd {

  struct KrokodilePosition : public gf::Message {
    static constexpr gf::Id type = "KrokodilePosition"_id;

    gf::Vector2f position;
    float angle;
  };

  struct KrokodileStats : public gf::Message {
    static constexpr gf::Id type = "KrokodileStats"_id;

    float foodLevel;
    int ageLevel;
  };

  struct CompleteGame: public gf::Message {
    static constexpr gf::Id type = "Complete"_id;
  };

  struct GamepadConnected: public gf::Message {
    static constexpr gf::Id type = "GamepadConnected"_id;

    gf::GamepadId gamepadId;
  };

  struct ViewSize: public gf::Message {
    static constexpr gf::Id type = "ViewSize"_id;

    gf::Vector2f viewSize;
    gf::Vector2f viewCenter;
  };
}

#endif // KKD_MESSAGES_H
