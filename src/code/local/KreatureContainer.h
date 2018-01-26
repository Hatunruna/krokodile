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

#ifndef _KKD_KREATURE_CONTAINER_H
#define _KKD_KREATURE_CONTAINER_H

#include <vector>

#include <gf/Entity.h>
#include <gf/Vector.h>

namespace kkd {
  class KreatureContainer : public gf::Entity {
  public:
    explicit KreatureContainer();

    virtual void update(gf::Time time) override;
    virtual void render(gf::RenderTarget &target, const gf::RenderStates &states) override;

  private:
    static const constexpr int MaxAge = 5;
    static const constexpr int SpawnLimit = 5;

  private:
    struct Kreature {
      int ageLevel = MaxAge;

      int headSprite;
      int bodySprite;
      int limbsSprite;
      int tailSprite;

      float foodLevel = 0.0f;

      gf::Vector2f position;
      float angle;
    };

  private:
    std::vector<Kreature> m_kreatures;
  };
} /* kkd */

#endif // _KKD_KREATURE_CONTAINER_H
