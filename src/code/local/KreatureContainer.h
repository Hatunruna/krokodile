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
    struct Part {
      int offset = 0;
      ColorName color;

      bool canBeK() const {
        return offset == 0 && color == Green;
      }
    };

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
      float foodLevel = 0.0f;

      Part head;
      Part body;
      Part limbs;
      Part tail;

      gf::Vector2f position;
      float orientation;
      float forwardMove = 0; // 1 to forward / -1 to backward
      float sideMove = 0; // 1 to rigth / -1 to left
      gf::Time timeElapsed;
      bool toggleAnimation = true;
      gf::Time lifeCountdown;

      gf::RotateToActivity rotationActivity;
      gf::MoveToActivity moveActivity;
      gf::SequenceActivity moveSequence;
    };

  public:
    explicit KreatureContainer();

    void playerForwardMove(int direction);
    void playerSidedMove(int direction);
    void playerSprint(bool sprint);
    void swapKreature();
    void fusionDNA();

    void removeDeadKreature();
    void checkComplete();
    void createKrokodile();

    void resetKreatures();

    void resetActivities(Kreature& kreature);

    virtual void update(gf::Time time) override;
    virtual void render(gf::RenderTarget &target, const gf::RenderStates &states) override;

    gf::MessageStatus onSizeView(gf::Id id, gf::Message *msg);

  private:
    static constexpr int MaxAge = 5;
    static constexpr int SpawnLimit = 25;
    static constexpr int MinimumPopulation = 15;
    static constexpr float MinimumLifeTime = 45.0f;
    static constexpr float MaximumLifeTime = 90.0f;
    static constexpr float ForwardVelocity = 200.0f;
    static constexpr float SideVelocity = 2.0f;
    static constexpr float activityRotationTime = 1.0f;
    static constexpr float AiMalusVelocity = 0.80f;
    static constexpr float FoodLevelMax = 100.0f;
    static constexpr float FoodLevelSteps = 15.0f;
    static constexpr float SprintVeloctiy = 2.0f;
    static constexpr float SprintFoodConsumption = -2.0f;

    static constexpr float MaxBound = 1500.0f;
    static constexpr float MinBound = - MaxBound;

    static constexpr float FusionFoodConsumption = 0.80f * FoodLevelMax;
    static constexpr float UpperFusionFactor = 0.75f;
    static constexpr float LowerFusionFactor = 0.25f;
    static constexpr float FumbleMutation = 0.90f;
    static constexpr float LimitLengthFusion = 150.0f;
    static constexpr gf::Time AnimationDuration = gf::seconds(0.25f);

  private:
    std::unique_ptr<Kreature>& getPlayerPtr();
    Kreature& getPlayer();
    const Kreature& getPlayer() const;
    std::unique_ptr<Kreature>& getCloserKreature();
    int colorCompare(ColorName color1, ColorName color2);
    Part fusionPart(Part currentPart, Part otherPart);
    void addFoodLevel(float consumption);

  private:
    std::vector< std::unique_ptr<Kreature> > m_kreatures;
    gf::Texture& m_kreatureHeadTexture;
    gf::Texture& m_kreaturePostLegTexture;
    gf::Texture& m_kreatureAnteLegTexture;
    gf::Texture& m_kreatureBodyTexture;
    gf::Texture& m_kreatureTailTexture;
    std::vector< std::array< gf::Vector2f, 6> > m_cropBoxes;

    bool m_isSprinting;
    gf::RectF m_viewRect;
  };
} /* kkd */

#endif // _KKD_KREATURE_CONTAINER_H
