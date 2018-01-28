#include "Hud.h"

#include <gf/Anchor.h>
#include <gf/Color.h>
#include <gf/Coordinates.h>
#include <gf/Shapes.h>
#include <gf/Sprite.h>
#include <gf/Text.h>
#include <gf/VectorOps.h>

#include "Singletons.h"

#define UNUSED(x) (void)(x)

namespace kkd {

  Hud::Hud()
  : gf::Entity(10)
  , m_font(gResourceManager().getFont("blkchcry.ttf"))
  , m_clock(gResourceManager().getTexture("clock.png"))
  , m_gen(gResourceManager().getTexture("gen.png"))
  , m_heartOk(gResourceManager().getTexture("heart_red.png"))
  , m_heartLow(gResourceManager().getTexture("heart.png"))
  , m_penta(gResourceManager().getTexture("penta.png"))
  {
    // register message handler
    gMessageManager().registerHandler<KrokodileStats>(&Hud::onKrokodileStats, this);
    m_clock.setSmooth();
    m_gen.setSmooth();
    m_heartOk.setSmooth();
    m_heartLow.setSmooth();
    m_penta.setSmooth();
  }

  void Hud::render(gf::RenderTarget& target, const gf::RenderStates& states)
  {
    UNUSED(states);
    static constexpr float Padding = 15.0f;
    static constexpr float RatioWarning = 0.8f;

    gf::Coordinates coords(target);

    gf::Vector2f genPos = coords.getAbsolutePoint({ Padding, Padding}, gf::Anchor::BottomLeft);
    gf::Vector2f pentaPos = coords.getAbsolutePoint({ Padding, Padding }, gf::Anchor::BottomRight);

    unsigned characterSize = coords.getRelativeCharacterSize(0.08f);
    float HudIconsScale = characterSize / 128.0f / 1.25f;

    // GEN INFO
    gf::Sprite genSprite;
    genSprite.setTexture(m_gen);
    genSprite.setPosition(genPos);
    genSprite.setScale(HudIconsScale);
    genSprite.setAnchor(gf::Anchor::BottomLeft);

    gf::Text genText(std::to_string(m_genNumber), m_font, characterSize);
    genText.setColor(gf::Color::White);
    genText.setOutlineColor(gf::Color::Black);
    genText.setOutlineThickness(characterSize / 30.0f);
    genText.setPosition({genPos.x + genSprite.getLocalBounds().width * HudIconsScale + Padding, genPos.y});
    genText.setAnchor(gf::Anchor::BottomLeft);

    // FOOD INFO
    gf::Sprite heartSprite;
    if (m_foodLevel / 100.0f < RatioWarning) {
      heartSprite.setTexture(m_heartLow);
    } else {
      heartSprite.setTexture(m_heartOk);
    }
    heartSprite.setScale(HudIconsScale);
    heartSprite.setPosition({ genPos.x, genPos.y - genSprite.getLocalBounds().height * HudIconsScale });
    heartSprite.setAnchor(gf::Anchor::BottomLeft);

    // Timer
    gf::Sprite clockSprite;
    clockSprite.setTexture(m_clock);
    clockSprite.setScale(HudIconsScale);
    clockSprite.setPosition({ Padding, Padding });

    gf::Text timer(std::to_string(static_cast<int>(m_time.getElapsedTime().asSeconds())), m_font, characterSize);
    timer.setColor(gf::Color::White);
    timer.setOutlineColor(gf::Color::Black);
    timer.setOutlineThickness(characterSize / 30.0f);
    timer.setPosition({ 2.0f * Padding + clockSprite.getLocalBounds().width * HudIconsScale, Padding });
    timer.setAnchor(gf::Anchor::TopLeft);

    // PENTA COLOR
    gf::Sprite pentaSprite;
    pentaSprite.setTexture(m_penta);
    pentaSprite.setScale(HudIconsScale * 3.0f);
    pentaSprite.setPosition(pentaPos);
    pentaSprite.setAnchor(gf::Anchor::BottomRight);

    gf::ConvexShape pentaBackground(5);
    pentaBackground.setPoint(0, { 128.0f * 0.5f, 128.0f * 0.0f });
    pentaBackground.setPoint(1, { 128.0f * 1.0f, 128.0f * 0.28f });
    pentaBackground.setPoint(2, { 128.0f * 0.83f, 128.0f * 1.0f });
    pentaBackground.setPoint(3, { 128.0f * 0.19f, 128.0f * 1.0f });
    pentaBackground.setPoint(4, { 128.0f * 0.0f,  128.0f * 0.28f });

    pentaBackground.setOutlineThickness(5.0f);
    pentaBackground.setOutlineColor(gf::Color::Opaque(0.3f));
    pentaBackground.setColor(gf::Color::Opaque(0.6f));
    pentaBackground.setScale(HudIconsScale * 3.0f);
    pentaBackground.setPosition(pentaPos);
    pentaBackground.setAnchor(gf::Anchor::BottomRight);

    // DRAW EVERYTHING
    target.draw(genText);
    target.draw(timer);
    target.draw(clockSprite);
    target.draw(genSprite);
    target.draw(heartSprite);
    target.draw(pentaBackground);
    target.draw(pentaSprite);
  }

  void Hud::reset() {
    m_time.restart();
  }

  gf::MessageStatus Hud::onKrokodileStats(gf::Id id, gf::Message *msg) {
    assert(id == KrokodileStats::type);
    KrokodileStats *stats = static_cast<KrokodileStats*>(msg);

    m_genNumber = stats->ageLevel;
    m_foodLevel = stats->foodLevel;

    return gf::MessageStatus::Keep;
  }

}
