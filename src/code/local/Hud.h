#ifndef KKD_HUD_H
#define KKD_HUD_H

#include <gf/Entity.h>
#include <gf/Font.h>
#include <gf/RenderTarget.h>
#include <gf/Shapes.h>
#include <gf/Clock.h>

#include "local/Messages.h"

namespace kkd {
  class Hud: public gf::Entity {
  public:
    Hud();

    virtual void render(gf::RenderTarget& target, const gf::RenderStates& states) override;

    gf::MessageStatus onKrokodileStats(gf::Id id, gf::Message *msg);

  private:
    gf::Font &m_font;
    int m_genNumber;
    gf::Clock m_time;
    float m_foodLevel;

    gf::RectangleShape m_maxFood;
    gf::RectangleShape m_currentFood;
  };
}

#endif // KKD_HUD_H
