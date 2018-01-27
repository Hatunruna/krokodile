#include "Hud.h"

#include <gf/Anchor.h>
#include <gf/Color.h>
#include <gf/Text.h>
#include <gf/VectorOps.h>

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
    gf::Vector2f foodSize({300.0f, 30.0f});
    int OutlineThickness = 5;
    float RatioWarning = 80.0f;
    gf::RoundedRectangleShape foodMaxHud;
    foodMaxHud.setSize(foodSize);
    foodMaxHud.setRadius(10);
    foodMaxHud.setColor(gf::Color::Transparent);
    foodMaxHud.setOutlineColor(gf::Color::White);
    foodMaxHud.setOutlineThickness(OutlineThickness);
    foodMaxHud.setPosition({m_screenWidth - (float)OutlineThickness, (float)OutlineThickness});
    foodMaxHud.setAnchor(gf::Anchor::TopRight);

    gf::RoundedRectangleShape foodHud;
    foodHud.setColor(gf::Color::Red);
    foodHud.setRadius(10);
    foodHud.setPosition({m_screenWidth - foodSize.x - (float)OutlineThickness, (float)OutlineThickness});
    foodHud.setSize({m_foodLevel * RATIO_WIDTH_FOOD_HUD, foodSize.y});

    gf::RectangleShape foodWarning;
    foodWarning.setSize({2.0f, foodSize.y});
    foodWarning.setColor(gf::Color::Black);
    foodWarning.setPosition({(m_screenWidth - foodSize.x - (float)OutlineThickness) + RatioWarning * RATIO_WIDTH_FOOD_HUD, (float)OutlineThickness});

    // GEN INFO
    gf::Text genText("Gen : " + std::to_string(m_genNumber), m_font, 50);
    genText.setColor(gf::Color::White);
    genText.setPosition({0.0f, 0.0f});
    genText.setAnchor(gf::Anchor::TopLeft);

    // Timer
    gf::Text timer("Time : " + std::to_string((int) m_time.getElapsedTime().asSeconds()), m_font, 50);
    timer.setColor(gf::Color::White);
    timer.setPosition({0.0f, 80.0f});
    timer.setAnchor(gf::Anchor::TopLeft);

    target.draw(foodMaxHud);
    target.draw(foodHud);
    target.draw(foodWarning);
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
