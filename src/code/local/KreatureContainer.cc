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
      float x = gRandom().computeUniformFloat(-500.0f, 500.0f);
      float y = gRandom().computeUniformFloat(-500.0f, 500.0f);

      Kreature kreature;
      kreature.position = { x, y };
      kreature.bodyColor = gRandom().computeUniformInteger(0, 4);
      kreature.orientation = gRandom().computeUniformFloat(0.0f, 2 * gf::Pi);

      m_kreatures.push_back(kreature);
    }
  }

  void KreatureContainer::playerForwardMove(int direction) {
    m_kreatures[0].forwardMove = direction;
  }

  void KreatureContainer::playerSidedMove(int direction) {
    m_kreatures[0].sideMove = direction;
  }

  void KreatureContainer::update(gf::Time time) {
    for (auto &kreature: m_kreatures) {
      // Update the orientation
      kreature.orientation += SideVelocity * kreature.sideMove * time.asSeconds();
      kreature.sideMove = 0;

      // Update the position
      kreature.position += gf::unit(kreature.orientation) * ForwardVelocity * kreature.forwardMove * time.asSeconds();
      kreature.forwardMove = 0;
    }

    KrokodilePosition message;
    message.position = m_kreatures[0].position;
    message.angle = m_kreatures[0].orientation;
    gMessageManager().sendMessage(&message);
  }

  void KreatureContainer::render(gf::RenderTarget &target, const gf::RenderStates &states) {
    for (auto &kreature: m_kreatures) {
      gf::RectangleShape rect({ 100.0f, 50.0f });
      rect.setColor(getKreaturColor(kreature.bodyColor));
      rect.setPosition(kreature.position);
      rect.setRotation(kreature.orientation);
      rect.setAnchor(gf::Anchor::Center);
      rect.draw(target, states);
    }
  }

}
