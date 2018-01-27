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
#include "Singletons.h"

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
      float x = gRandom().computeUniformFloat(-50.0f, 50.0f);
      float y = gRandom().computeUniformFloat(-50.0f, 50.0f);

      float xTarget = gRandom().computeUniformFloat(-50.0f, 50.0f);
      float yTarget = gRandom().computeUniformFloat(-50.0f, 50.0f);

      float rotation = gRandom().computeUniformFloat(0.0f, 2 * gf::Pi);

      auto kreature = std::make_unique<Kreature>(gf::Vector2f(x, y), rotation, gf::Vector2f(xTarget, yTarget));
      kreature->bodyColor = gRandom().computeUniformInteger(0, 4);

      m_kreatures.push_back(std::move(kreature));
    }
  }

  void KreatureContainer::playerForwardMove(int direction) {
    m_kreatures[0]->forwardMove = direction;
  }

  void KreatureContainer::playerSidedMove(int direction) {
    m_kreatures[0]->sideMove = direction;
  }

  void KreatureContainer::update(gf::Time time) {
    // Update the player
    // Update the orientation
    m_kreatures[0]->orientation += SideVelocity * m_kreatures[0]->sideMove * time.asSeconds();
    m_kreatures[0]->sideMove = 0;

    // Update the position
    m_kreatures[0]->position += gf::unit(m_kreatures[0]->orientation) * ForwardVelocity * m_kreatures[0]->forwardMove * time.asSeconds();
    m_kreatures[0]->forwardMove = 0;

    // Update AI
    for (unsigned i = 1; i < m_kreatures.size(); ++i) {
      gf::ActivityStatus status = m_kreatures[i]->moveSequence.run(time);

      if (status == gf::ActivityStatus::Finished) {
        float xTarget = gRandom().computeUniformFloat(-50.0f, 50.0f);
        float yTarget = gRandom().computeUniformFloat(-50.0f, 50.0f);
        gf::Vector2f target = { xTarget, yTarget };

        // Reset the activities
        m_kreatures[i]->rotationActivity.setOrigin(m_kreatures[i]->orientation);
        m_kreatures[i]->rotationActivity.setTarget(gf::angle(target - m_kreatures[i]->position));

        m_kreatures[i]->moveActivity.setOrigin(m_kreatures[i]->position);
        m_kreatures[i]->moveActivity.setTarget(target);
        m_kreatures[i]->moveActivity.setDuration(gf::seconds(gf::euclideanDistance(m_kreatures[i]->position, target) / (ForwardVelocity * AiMalusVelocity)));

        m_kreatures[i]->moveSequence.restart();
      }
    }

    KrokodilePosition message;
    message.position = m_kreatures[0].position;
    message.angle = m_kreatures[0].orientation;
    gMessageManager().sendMessage(&message);
  }

  void KreatureContainer::render(gf::RenderTarget &target, const gf::RenderStates &states) {
    for (unsigned i = 0; i < m_kreatures.size(); ++i) {
      auto &kreature = m_kreatures[i];

      gf::RectangleShape rect({ 10.0f, 5.0f });
      rect.setColor(getKreaturColor(kreature->bodyColor));
      rect.setPosition(kreature->position);
      rect.setRotation(kreature->orientation);
      rect.setAnchor(gf::Anchor::Center);
      rect.draw(target, states);
    }
  }

}
