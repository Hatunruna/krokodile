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
#include <gf/Math.h>
#include <gf/Shapes.h>

#include "Messages.h"

namespace kkd {

  namespace {
    gf::Color4f getKreaturColor(int ith) {
      switch (ith) {
        case 0:
          return gf::Color::darker(gf::Color::Azure, 0.25);
        case 1:
          return gf::Color::darker(gf::Color::Green, 0.25);
        case 2:
          return gf::Color::darker(gf::Color::Yellow, 0.25);
        case 3:
          return gf::Color::darker(gf::Color::Red, 0.25);
        case 4:
          return gf::Color::darker(gf::Color::Magenta, 0.25);
        default:
          break;
      }

      assert(false);
      return gf::Color::Black;
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
      kreature->bodyColor = gRandom().computeUniformInteger(0, 4);

      m_kreatures.push_back(std::move(kreature));
    }
  }

  float KreatureContainer::getPlayerFoodLevel() const{
    return m_kreatures[0]->foodLevel;
  }

  int KreatureContainer::getPlayerGen() const{
    return m_kreatures[0]->ageLevel;
  }

  void KreatureContainer::playerForwardMove(int direction) {
    m_kreatures[0]->forwardMove = direction;
  }

  void KreatureContainer::playerSidedMove(int direction) {
    m_kreatures[0]->sideMove = direction;
  }

  void KreatureContainer::swapKreature() {
    assert(m_kreatures.size() >= 2);

    auto newKreature = std::min_element(m_kreatures.begin() + 1, m_kreatures.end(), [this](auto &krea1, auto &krea2){
      float length1 = gf::euclideanDistance(m_kreatures[0]->position, krea1->position);
      float length2 = gf::euclideanDistance(m_kreatures[0]->position, krea2->position);
      return length1 < length2;
    });

    // Reset the activity for the old kreature
    resetActivities(*(m_kreatures.begin()));

    std::iter_swap(m_kreatures.begin(), newKreature);
  }

  void KreatureContainer::resetActivities(std::unique_ptr<Kreature> &kreature) {
    float xTarget = gRandom().computeUniformFloat(MinBound, MaxBound);
    float yTarget = gRandom().computeUniformFloat(MinBound, MaxBound);
    gf::Vector2f target = { xTarget, yTarget };

    // Reset the activities
    kreature->rotationActivity.setOrigin(kreature->orientation);
    kreature->rotationActivity.setTarget(gf::angle(target - kreature->position));

    kreature->moveActivity.setOrigin(kreature->position);
    kreature->moveActivity.setTarget(target);
    kreature->moveActivity.setDuration(gf::seconds(gf::euclideanDistance(kreature->position, target) / (ForwardVelocity * AiMalusVelocity)));

    kreature->moveSequence.restart();
  }

  void KreatureContainer::update(gf::Time time) {
    assert(!m_kreatures.empty());

    // Update the player
    Kreature& player = *m_kreatures[0];

    // Update the orientation
    player.orientation += SideVelocity * m_kreatures[0]->sideMove * time.asSeconds();
    player.sideMove = 0;

    // Update the position
    player.position += gf::unit(m_kreatures[0]->orientation) * ForwardVelocity * m_kreatures[0]->forwardMove * time.asSeconds();
    player.forwardMove = 0;

    player.position = gf::clamp(player.position, MinBound, MaxBound);

    // Update AI
    for (unsigned i = 1; i < m_kreatures.size(); ++i) {
      gf::ActivityStatus status = m_kreatures[i]->moveSequence.run(time);

      if (status == gf::ActivityStatus::Finished) {
        resetActivities(m_kreatures[i]);
      }
    }

    KrokodilePosition message;
    message.position = m_kreatures[0]->position;
    message.angle = m_kreatures[0]->orientation;
    gMessageManager().sendMessage(&message);

    m_kreatures[0]->foodLevel += 0.5f;
    if (m_kreatures[0]->foodLevel > FoodLevelMax) {
      m_kreatures[0]->foodLevel = 0.0f;
    }
  }

  void KreatureContainer::render(gf::RenderTarget &target, const gf::RenderStates &states) {
    for (unsigned i = 0; i < m_kreatures.size(); ++i) {
      auto &kreature = m_kreatures[i];

      gf::RectangleShape rect({ 100.0f, 50.0f });
      rect.setColor(getKreaturColor(kreature->bodyColor));
      rect.setPosition(kreature->position);
      rect.setRotation(kreature->orientation);
      rect.setAnchor(gf::Anchor::Center);
      rect.draw(target, states);
    }
  }

}
