/*
 * Krokodile
 * Copyright (C) 2018 Hatunruna team
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "KreatureContainer.h"

#include <gf/Color.h>
#include <gf/Log.h>
#include <gf/Math.h>
#include <gf/Shapes.h>
#include <gf/Transform.h>

#include "Messages.h"

namespace kkd {

  namespace {
    gf::Color4f getKreatureColor(KreatureContainer::ColorName ith) {
      switch (ith) {
        case KreatureContainer::Azure:
          return gf::Color::darker(gf::Color::Azure, 0.25);
        case KreatureContainer::Green:
          return gf::Color::darker(gf::Color::Green, 0.25);
        case KreatureContainer::Yellow:
          return gf::Color::darker(gf::Color::Yellow, 0.25);
        case KreatureContainer::Red:
          return gf::Color::darker(gf::Color::Red, 0.25);
        case KreatureContainer::Magenta:
          return gf::Color::darker(gf::Color::Magenta, 0.25);
        default:
          break;
      }

      assert(false);
      return gf::Color::Black;
    }

    KreatureContainer::ColorName randomColor() {
      return static_cast<KreatureContainer::ColorName>(gRandom().computeUniformInteger(0, 4));
    }

  }

  KreatureContainer::KreatureContainer() {
    for (int i = 0; i < SpawnLimit; ++i) {
      // Get the initial value
      float x = gRandom().computeUniformFloat(MinBound, MaxBound);
      float y = gRandom().computeUniformFloat(MinBound, MaxBound);

      float xTarget = gRandom().computeUniformFloat(MinBound, MaxBound);
      float yTarget = gRandom().computeUniformFloat(MinBound, MaxBound);

      float rotation = gRandom().computeUniformFloat(0.0f, 2 * gf::Pi);

      auto kreature = std::make_unique<Kreature>(gf::Vector2f(x, y), rotation, gf::Vector2f(xTarget, yTarget));
      kreature->bodyColor = randomColor();
      kreature->headColor = randomColor();
      kreature->limbsColor = randomColor();
      kreature->tailColor = randomColor();

      m_kreatures.push_back(std::move(kreature));
    }
  }

  void KreatureContainer::playerForwardMove(int direction) {
    getPlayer().forwardMove = direction;
  }

  void KreatureContainer::playerSidedMove(int direction) {
    getPlayer().sideMove = direction;
  }

  void KreatureContainer::swapKreature() {
    assert(m_kreatures.size() >= 2);

    auto& newKreature = getCloserKreature();

    // Reset the activity for the old kreature
    resetActivities(getPlayer());

    std::swap(getPlayerPtr(), newKreature);
  }

  void KreatureContainer::fusionDNA() {
    assert(m_kreatures.size() >= 2);

    auto& closerKreature = getCloserKreature();
    auto& currentKreature = getPlayerPtr();

    // If the kreatures is too for
    if (gf::euclideanDistance(closerKreature->position, currentKreature->position) > LimitLengthFusion || currentKreature->ageLevel <= 0 || currentKreature->foodLevel < FusionFoodConsumption) {
      return;
    }

    // Create the child
    auto newPosition = currentKreature->position + gf::Vector2f(100.0f, 100.0f);
    float xTarget = gRandom().computeUniformFloat(MinBound, MaxBound);
    float yTarget = gRandom().computeUniformFloat(MinBound, MaxBound);

    float rotation = gRandom().computeUniformFloat(0.0f, 2 * gf::Pi);
    auto child = std::make_unique<Kreature>(newPosition, rotation, gf::Vector2f(xTarget, yTarget));

    // Body fusion
    child->bodyColor = fusionPart(currentKreature->bodyColor, closerKreature->bodyColor);

    // Body head
    child->headColor = fusionPart(currentKreature->headColor, closerKreature->headColor);

    // Body tail
    child->tailColor = fusionPart(currentKreature->tailColor, closerKreature->tailColor);

    // Body limbs
    child->limbsColor = fusionPart(currentKreature->limbsColor, closerKreature->limbsColor);


    addFoodLevel(-FusionFoodConsumption);

    --currentKreature->ageLevel;

    m_kreatures.push_back(std::move(child));
  }

  void KreatureContainer::resetActivities(Kreature& kreature) {
    float xTarget = gRandom().computeUniformFloat(MinBound, MaxBound);
    float yTarget = gRandom().computeUniformFloat(MinBound, MaxBound);
    gf::Vector2f target = { xTarget, yTarget };

    // Reset the activities
    kreature.rotationActivity.setOrigin(kreature.orientation);
    kreature.rotationActivity.setTarget(gf::angle(target - kreature.position));

    kreature.moveActivity.setOrigin(kreature.position);
    kreature.moveActivity.setTarget(target);
    kreature.moveActivity.setDuration(gf::seconds(gf::euclideanDistance(kreature.position, target) / (ForwardVelocity * AiMalusVelocity)));

    kreature.moveSequence.restart();
  }

  void KreatureContainer::update(gf::Time time) {
    assert(!m_kreatures.empty());

    // Update the player
    Kreature& player = getPlayer();

    // Update the orientation
    player.orientation += SideVelocity * player.sideMove * time.asSeconds();
    player.sideMove = 0;

    // Update the position
    player.position += gf::unit(player.orientation) * ForwardVelocity * player.forwardMove * time.asSeconds();
    player.forwardMove = 0;

    player.position = gf::clamp(player.position, MinBound, MaxBound);

    // Update AI
    for (unsigned i = 1; i < m_kreatures.size(); ++i) {
      gf::ActivityStatus status = m_kreatures[i]->moveSequence.run(time);

      if (status == gf::ActivityStatus::Finished) {
        resetActivities(*m_kreatures[i]);
      }
    }

    KrokodilePosition message;
    message.position = player.position;
    message.angle = player.orientation;
    gMessageManager().sendMessage(&message);

    // Update the food level
    addFoodLevel(time.asSeconds() * FoodLevelSteps);

    // Send stats to HUD
    KrokodileStats stats;
    stats.foodLevel = player.foodLevel;
    stats.ageLevel = player.ageLevel;
    gMessageManager().sendMessage(&stats);
  }

  void KreatureContainer::render(gf::RenderTarget &target, const gf::RenderStates &states) {
    for (unsigned i = 0; i < m_kreatures.size(); ++i) {
      auto &kreature = m_kreatures[i];

      gf::RectangleShape body({ 100.0f, 50.0f });
      body.setColor(getKreatureColor(kreature->bodyColor));
      body.setPosition(kreature->position);
      body.setRotation(kreature->orientation);

      gf::Matrix3f bodyMatrix = body.getTransform();

      body.setAnchor(gf::Anchor::Center);

      gf::RectangleShape head({ 25.0f, 25.0f });
      head.setAnchor(gf::Anchor::CenterLeft);
      head.setColor(getKreatureColor(kreature->headColor));
      head.setPosition(gf::transform(bodyMatrix, {50.0f, 0.0f}));
      head.setRotation(kreature->orientation);

      gf::RectangleShape armLeft({50.0f, 25.0f});
      armLeft.setAnchor(gf::Anchor::BottomCenter);
      armLeft.setColor(getKreatureColor(kreature->limbsColor));
      armLeft.setPosition(gf::transform(bodyMatrix, {35.0f, -25.0f}));
      armLeft.setRotation(kreature->orientation);

      gf::RectangleShape armRight({50.0f, 25.0f});
      armRight.setAnchor(gf::Anchor::TopCenter);
      armRight.setColor(getKreatureColor(kreature->limbsColor));
      armRight.setPosition(gf::transform(bodyMatrix, {35.0f, 25.0f}));
      armRight.setRotation(kreature->orientation);

      gf::RectangleShape legLeft({50.0f, 25.0f});
      legLeft.setAnchor(gf::Anchor::BottomCenter);
      legLeft.setColor(getKreatureColor(kreature->limbsColor));
      legLeft.setPosition(gf::transform(bodyMatrix, {-35.0f, -25.0f}));
      legLeft.setRotation(kreature->orientation);

      gf::RectangleShape legRight({50.0f, 25.0f});
      legRight.setAnchor(gf::Anchor::TopCenter);
      legRight.setColor(getKreatureColor(kreature->limbsColor));
      legRight.setPosition(gf::transform(bodyMatrix, {-35.0f, 25.0f}));
      legRight.setRotation(kreature->orientation);

      gf::RectangleShape tail({75.0f, 25.0f});
      tail.setAnchor(gf::Anchor::CenterRight);
      tail.setColor(getKreatureColor(kreature->tailColor));
      tail.setPosition(gf::transform(bodyMatrix, {-50.0f, 0.0f}));
      tail.setRotation(kreature->orientation);

      armLeft.draw(target, states);
      armRight.draw(target, states);
      legLeft.draw(target, states);
      legRight.draw(target, states);
      tail.draw(target, states);
      head.draw(target, states);
      body.draw(target, states);
    }
  }

  std::unique_ptr<KreatureContainer::Kreature>& KreatureContainer::getPlayerPtr() {
    assert(!m_kreatures.empty());
    return m_kreatures.front();
  }

  KreatureContainer::Kreature& KreatureContainer::getPlayer() {
    assert(!m_kreatures.empty());
    return *m_kreatures.front();
  }

  const KreatureContainer::Kreature& KreatureContainer::getPlayer() const {
    assert(!m_kreatures.empty());
    return *m_kreatures.front();
  }

  std::unique_ptr<KreatureContainer::Kreature>& KreatureContainer::getCloserKreature() {
    auto newKreature = std::min_element(m_kreatures.begin() + 1, m_kreatures.end(), [this](auto &krea1, auto &krea2){
      float length1 = gf::euclideanDistance(m_kreatures[0]->position, krea1->position);
      float length2 = gf::euclideanDistance(m_kreatures[0]->position, krea2->position);
      return length1 < length2;
    });

    return *newKreature;
  }

  int KreatureContainer::colorCompare(ColorName color1, ColorName color2) {
    switch (color1) {
      case Azure:
        if (color2 == Yellow || color2 == Magenta) {
          return 1;
        }
        if (color2 == Green || color2 == Red) {
          return -1;
        }
        return 0;

      case KreatureContainer::Green:
        if (color2 == Azure || color2 == Yellow) {
          return 1;
        }
        if (color2 == Magenta || color2 == Red) {
          return -1;
        }
        return 0;

      case KreatureContainer::Yellow:
        if (color2 == Red || color2 == Magenta) {
          return 1;
        }
        if (color2 == Azure || color2 == Green) {
          return -1;
        }
        return 0;

      case KreatureContainer::Red:
        if (color2 == Azure || color2 == Green) {
          return 1;
        }
        if (color2 == Yellow || color2 == Magenta) {
          return -1;
        }
        return 0;

      case KreatureContainer::Magenta:
        if (color2 == Red || color2 == Green) {
          return 1;
        }
        if (color2 == Yellow || color2 == Azure) {
          return -1;
        }
        return 0;

      default:
        assert(false);
        break;
    }

    return 0;
  }

  KreatureContainer::ColorName KreatureContainer::fusionPart(KreatureContainer::ColorName currentColor, KreatureContainer::ColorName otherColor) {
    float fusionFactor = 0.0f;
    if (colorCompare(currentColor, otherColor) == 1) {
      fusionFactor = UpperFusionFactor;
    }
    else {
      fusionFactor = LowerFusionFactor;
    }

    float rand = gRandom().computeUniformFloat(0.0f, 1.0f);
    if (rand < fusionFactor) {
      return otherColor;
    }
    if (rand >= FumbleMutation) {
      return randomColor();
    }

    return currentColor;
  }

  void KreatureContainer::addFoodLevel(float consumption) {
    auto& player = getPlayer();
    player.foodLevel += consumption;
    player.foodLevel = gf::clamp(player.foodLevel, 0.0f, FoodLevelMax);
  }

}
