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

#include <memory>
#include <vector>

#include <gf/Activities.h>
#include <gf/Entity.h>
#include <gf/Vector.h>
#include <gf/VectorOps.h>

#include "Singletons.h"

namespace kkd {
  class KreatureContainer : public gf::Entity {
  public:
    enum ColorName : int {
      Azure = 0,
      Green = 1,
      Yellow = 2,
      Red = 3,
      Magenta = 4,
    };

  private:
    struct Kreature {
      Kreature(gf::Vector2f kreaPosition, float kreaRotation, gf::Vector2f kreaTarget)
      : position(kreaPosition)
      , orientation(kreaRotation)
      , rotationActivity(kreaRotation, gf::angle(kreaTarget - kreaPosition), orientation, gf::seconds(activityRotationTime))
      , moveActivity(kreaPosition, kreaTarget, position, gf::seconds(gf::euclideanDistance(kreaPosition, kreaTarget) / (ForwardVelocity * AiMalusVelocity))) {
        moveSequence.addActivity(rotationActivity);
        moveSequence.addActivity(moveActivity);
      }

      int ageLevel = MaxAge;

      int headSprite;
      ColorName headColor;
      int bodySprite;
      ColorName bodyColor;
      int limbsSprite;
      ColorName limbsColor;
      int tailSprite;
      ColorName tailColor;

      float foodLevel = 0.0f;

      gf::Vector2f position;
      float orientation;
      float forwardMove = 0; // 1 to forward / -1 to backward
      float sideMove = 0; // 1 to rigth / -1 to left

      gf::RotateToActivity rotationActivity;
      gf::MoveToActivity moveActivity;
      gf::SequenceActivity moveSequence;
    };

  public:
    explicit KreatureContainer();

    float getPlayerFoodLevel() const;
    int getPlayerGen() const;

    void playerForwardMove(int direction);
    void playerSidedMove(int direction);
    void swapKreature();
    void fusionDNA();

    void resetActivities(std::unique_ptr<Kreature> &kreature);

    virtual void update(gf::Time time) override;
    virtual void render(gf::RenderTarget &target, const gf::RenderStates &states) override;

  private:
    static const constexpr int MaxAge = 5;
    static const constexpr int SpawnLimit = 50;
    static const constexpr float ForwardVelocity = 200.0f;
    static const constexpr float SideVelocity = 2.0f;
    static const constexpr float activityRotationTime = 1.0f;
    static const constexpr float AiMalusVelocity = 0.80f;
    static const constexpr float FoodLevelMax = 100.0f;

    static constexpr float MaxBound = 1500.0f;
    static constexpr float MinBound = - MaxBound;

    static constexpr float UpperFusionFactor = 0.75f;
    static constexpr float LowerFusionFactor = 0.25f;
    static constexpr float FumbleMutation = 0.90f;
    static constexpr float LimitLengthFusion = 150.0f;

  private:
    std::vector< std::unique_ptr<Kreature> >::iterator getCloserKreature();

    int colorCompare(ColorName color1, ColorName color2);

  private:
    std::vector< std::unique_ptr<Kreature> > m_kreatures;
  };
} /* kkd */

#endif // _KKD_KREATURE_CONTAINER_H
