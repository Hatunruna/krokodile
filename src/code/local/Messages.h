#ifndef KKD_MESSAGES_H
#define KKD_MESSAGES_H

#include <gf/Message.h>

using namespace gf::literals;

namespace kkd {

  struct KrokodilePosition : public gf::Message {
    static constexpr gf::Id type = "KrokodilePosition"_id;

    gf::Vector2f position;
    float angle;
  };
}

#endif // KKD_MESSAGES_H
