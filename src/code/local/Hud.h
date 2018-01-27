#ifndef KKD_HUD_H
#define KKD_HUD_H

#include <gf/Entity.h>
#include <gf/Font.h>
#include <gf/RenderTarget.h>
#include <gf/Shapes.h>

namespace kkd {
  class Hud: public gf::Entity {
  public:
    Hud(float screenWidth);

    virtual void render(gf::RenderTarget& target, const gf::RenderStates& states) override;

    void setGenLevel(int gen);
    void setFoodLevel(float level);
  private:
    gf::Font &m_font;
    int m_genNumber;
    float m_foodLevel;
    float m_screenWidth;

    gf::RectangleShape m_maxFood;
    gf::RectangleShape m_currentFood;
  };
}

#endif // KKD_HUD_H
