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

#include <gf/Shapes.h>

#include "Singletons.h"

kkd::KreatureContainer::KreatureContainer() {
  for (int i = 0; i < SpawnLimit; ++i) {
    float x = gRandom().computeUniformFloat(-50.0f, 50.0f);
    float y = gRandom().computeUniformFloat(-50.0f, 50.0f);

    Kreature kreature;
    kreature.position = { x, y };

    m_kreatures.push_back(kreature);
  }
}

void kkd::KreatureContainer::update(gf::Time time) {
  for (auto &kreature: m_kreatures) {
    // do something
  }
}

void kkd::KreatureContainer::render(gf::RenderTarget &target, const gf::RenderStates &states) {
  for (auto &kreature: m_kreatures) {
    gf::RectangleShape rect({ 10.0f, 10.0f });
    rect.setColor(gf::Color::Green);
    rect.setPosition(kreature.position);
    rect.setAnchor(gf::Anchor::Center);
    rect.draw(target, states);
  }
}
