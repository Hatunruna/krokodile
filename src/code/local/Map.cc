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
#include "Map.h"

#include <gf/RenderTarget.h>

#include "Singletons.h"

namespace kkd {

  Map::Map()
  : m_texture(gResourceManager().getTexture("tileset.png"))
  , m_layer({ 50u, 50u })
  {
    m_layer.setTexture(m_texture);
    m_layer.setTileSize({ 64u, 64u });

    for (unsigned y = 0; y < 50; ++y) {
      for (unsigned x = 0; x < 50; ++x) {
        m_layer.setTile({ x, y }, gRandom().computeUniformInteger(0, 3));
      }
    }

    m_layer.setOrigin({ 64.f * 25, 64.f * 25 });
  }

  void Map::render(gf::RenderTarget &target, const gf::RenderStates &states) {
    target.draw(m_layer, states);
  }

}
