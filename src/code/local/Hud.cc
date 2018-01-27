#include "Hud.h"

#include <gf/Anchor.h>
#include <gf/Color.h>
#include <gf/Coordinates.h>
#include <gf/Text.h>
#include <gf/VectorOps.h>

#include "Singletons.h"

namespace kkd {

  Hud::Hud()
  : gf::Entity(10)
  , m_font(gResourceManager().getFont("blkchcry.ttf"))
  {
    // register message handler
    gMessageManager().registerHandler<KrokodileStats>(&Hud::onKrokodileStats, this);
  }

  void Hud::render(gf::RenderTarget& target, const gf::RenderStates& states)
  {
    static constexpr float Padding = 15.0f;
    static constexpr float RatioWarning = 0.8f;

    gf::Coordinates coords(target);

    // FOOD INFO
    gf::Vector2f foodSize = coords.getRelativeSize({ 0.25f, 0.04f });
    gf::Vector2f foodPosition = coords.getAbsolutePoint({ foodSize.width + Padding, Padding }, gf::Anchor::TopRight);


    gf::RoundedRectangleShape foodMaxHud;
    foodMaxHud.setSize(foodSize);
    foodMaxHud.setRadius(5);
    foodMaxHud.setColor(gf::Color::White);
    foodMaxHud.setOutlineColor(gf::Color::Black);
    foodMaxHud.setOutlineThickness(foodSize.height / 15.0f);
    foodMaxHud.setPosition(foodPosition);

    gf::RoundedRectangleShape foodHud;
    foodHud.setColor(gf::Color::Red);
    foodHud.setRadius(5);
    foodHud.setPosition(foodPosition);
    foodHud.setSize({ std::max(m_foodLevel / 100.0f, 0.01f) * foodSize.width, foodSize.height });

    gf::RectangleShape foodWarning;
    foodWarning.setSize({ 2.0f, foodSize.height });
    foodWarning.setColor(gf::Color::Black);
    foodWarning.setPosition({ foodPosition.x + foodSize.width * RatioWarning, foodPosition.y });


    unsigned characterSize = coords.getRelativeCharacterSize(0.08f);

    // GEN INFO
    gf::Text genText("Gen: " + std::to_string(m_genNumber), m_font, characterSize);
    genText.setColor(gf::Color::White);
    genText.setOutlineColor(gf::Color::Black);
    genText.setOutlineThickness(characterSize / 30.0f);
    genText.setPosition({ Padding, Padding });
    genText.setAnchor(gf::Anchor::TopLeft);

    // Timer
    gf::Text timer("Time: " + std::to_string(static_cast<int>(m_time.getElapsedTime().asSeconds())), m_font, characterSize);
    timer.setColor(gf::Color::White);
    timer.setOutlineColor(gf::Color::Black);
    timer.setOutlineThickness(characterSize / 30.0f);
    timer.setPosition({ Padding, 2 * Padding + characterSize });
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

}
