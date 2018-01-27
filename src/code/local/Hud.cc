#include "Hud.h"

#include <gf/Anchor.h>
#include <gf/Color.h>
#include <gf/Text.h>

#include "Singletons.h"

namespace kkd {

  static const constexpr float RATIO_WIDTH_FOOD_HUD = 3.0f;

  Hud::Hud(float screenWidth)
  : gf::Entity(10)
  , m_font(gResourceManager().getFont("blkchcry.ttf"))
  , m_screenWidth(screenWidth)
  {
    // register message handler
    gMessageManager().registerHandler<KrokodileStats>(&Hud::onKrokodileStats, this);
  }

  void Hud::render(gf::RenderTarget& target, const gf::RenderStates& states)
  {
    // FOOD INFO
    gf::RectangleShape foodMaxHud;
    foodMaxHud.setSize({300.0f, 30.0f});
    foodMaxHud.setColor(gf::Color::Transparent);
    foodMaxHud.setOutlineColor(gf::Color::White);
    foodMaxHud.setOutlineThickness(5);
    foodMaxHud.setPosition({m_screenWidth, 0.0f});
    foodMaxHud.setAnchor(gf::Anchor::TopRight);

    gf::RectangleShape foodHud;
    foodHud.setSize({0.0f, 30.0f});
    foodHud.setColor(gf::Color::Red);
    foodHud.setPosition({m_screenWidth - 300.0f, 0.0f});
    foodHud.setSize({m_foodLevel * RATIO_WIDTH_FOOD_HUD, 30.0f});

    // GEN INFO
    gf::Text genText("Gen : " + std::to_string(m_genNumber), m_font, 75);
    genText.setColor(gf::Color::White);
    genText.setPosition({0.0f, 0.0f});
    genText.setAnchor(gf::Anchor::TopLeft);

    // Timer
    gf::Text timer("Time : " + std::to_string((int) m_time.getElapsedTime().asSeconds()), m_font, 75);
    timer.setColor(gf::Color::White);
    timer.setPosition({0.0f, 80.0f});
    timer.setAnchor(gf::Anchor::TopLeft);

    target.draw(foodMaxHud);
    target.draw(foodHud);
    target.draw(genText);
    target.draw(timer);
  }

  gf::MessageStatus Hud::onKrokodileStats(gf::Id id, gf::Message *msg) {
    assert(id == KrokodileStats::type);
    KrokodileStats *stats = static_cast<KrokodileStats*>(msg);

    m_genNumber = stats->ageLevel;
    m_foodLevel = stats->foodLevel;

    return gf::MessageStatus::Keep;
  }

  void Hud::setWidth(float width) {
    m_screenWidth = width;
  }
}
