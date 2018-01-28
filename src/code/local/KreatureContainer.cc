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

#include <numeric>

#include <gf/Color.h>
#include <gf/Log.h>
#include <gf/Math.h>
#include <gf/Shapes.h>
#include <gf/Sprite.h>
#include <gf/Transform.h>

#include "Messages.h"

namespace kkd {
  static constexpr int TotalAnimal = 2;

  namespace {
    gf::Color4f getKreatureColor(KreatureContainer::ColorName ith) {
      switch (ith) {
        case KreatureContainer::Azure:
          return gf::Color::Azure;
        case KreatureContainer::Green:
          return gf::Color::Green;
        case KreatureContainer::Yellow:
          return gf::Color::lighter(gf::Color::Yellow, 0.25f);
        case KreatureContainer::Red:
          return gf::Color::lighter(gf::Color::Red, 0.25f);
        case KreatureContainer::Magenta:
          return gf::Color::lighter(gf::Color::Magenta, 0.25f);
        default:
          break;
      }

      assert(false);
      return gf::Color::Black;
    }

    KreatureContainer::ColorName randomColor() {
      return static_cast<KreatureContainer::ColorName>(gRandom().computeUniformInteger(0, 4));
    }

    int randomOffset() {
      return gRandom().computeUniformInteger(0, TotalAnimal - 1);
    }

  }

  KreatureContainer::KreatureContainer()
  : m_kreatureHeadTexture(gResourceManager().getTexture("kreature_head.png"))
  , m_kreaturePostLegTexture(gResourceManager().getTexture("kreature_postleg.png"))
  , m_kreatureAnteLegTexture(gResourceManager().getTexture("kreature_anteleg.png"))
  , m_kreatureBodyTexture(gResourceManager().getTexture("kreature_body.png"))
  , m_kreatureTailTexture(gResourceManager().getTexture("kreature_tail.png"))
  , m_isSprinting(false) {
    // register message handler
    gMessageManager().registerHandler<ViewSize>(&KreatureContainer::onSizeView, this);

    // Define hacks for sprites
    m_cropBoxs.resize(TotalAnimal);
    m_cropBoxs[0] = gf::RectF({ 0.0f, 0.0f }, { 128.0f, 135.0f });
    m_cropBoxs[1] = gf::RectF({ 0.0f, 0.0f }, { 116.0f, 170.0f });

    resetKreatures();
  }

  void KreatureContainer::playerForwardMove(int direction) {
    getPlayer().forwardMove = direction;
  }

  void KreatureContainer::playerSidedMove(int direction) {
    getPlayer().sideMove = direction;
  }

  void KreatureContainer::playerSprint(bool sprint) {
    m_isSprinting = sprint;
  }

  void KreatureContainer::swapKreature() {
    assert(m_kreatures.size() >= 2);

    auto& newKreature = getCloserKreature();

    // Reset the activity for the old kreature
    resetActivities(getPlayer());

    std::swap(getPlayerPtr(), newKreature);

    checkComplete();
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
    child->body = fusionPart(currentKreature->body, closerKreature->body);

    // Body head
    child->head = fusionPart(currentKreature->head, closerKreature->head);

    // Body tail
    child->tail = fusionPart(currentKreature->tail, closerKreature->tail);

    // Body limbs
    child->limbs = fusionPart(currentKreature->limbs, closerKreature->limbs);

    child->timeElapsed = gf::seconds(0.0f + gRandom().computeUniformFloat(0.01f, AnimationDuration.asSeconds() - 0.01f));
    child->lifeCountdown = gf::seconds(gRandom().computeUniformFloat(MinimumLifeTime, MaximumLifeTime));


    addFoodLevel(-FusionFoodConsumption);

    int age = --(currentKreature->ageLevel);
    --(closerKreature->ageLevel);

    m_kreatures.push_back(std::move(child));
    if (age <= 0) {
      std::iter_swap(m_kreatures.begin(), m_kreatures.end()-1);
    }
    removeDeadKreature();
    checkComplete();
  }

  void KreatureContainer::removeDeadKreature() {
    m_kreatures.erase( std::remove_if(m_kreatures.begin(),
                                      m_kreatures.end(),
      [this](auto &k) {
        gf::RectF viewBox({ k->position - 0.5f * gf::Vector2f(400.0f, 400.0f) }, { 400.0f, 400.0f });
        return !m_viewRect.intersects(viewBox) && (k->ageLevel <= 0 || k->lifeCountdown.asSeconds() <= 0.0f);
      }), m_kreatures.end());
  }

  void KreatureContainer::checkComplete() {
    auto& player = getPlayer();

    if (player.head.canBeK() && player.body.canBeK() && player.limbs.canBeK() && player.tail.canBeK()) {
      CompleteGame msg;
      gMessageManager().sendMessage(&msg);
    }
  }

  void KreatureContainer::createKrokodile() {
    float x = gRandom().computeUniformFloat(MinBound, MaxBound);
    float y = gRandom().computeUniformFloat(MinBound, MaxBound);

    float xTarget = gRandom().computeUniformFloat(MinBound, MaxBound);
    float yTarget = gRandom().computeUniformFloat(MinBound, MaxBound);

    float rotation = gRandom().computeUniformFloat(0.0f, 2 * gf::Pi);

    auto kreature = std::make_unique<Kreature>(gf::Vector2f(x, y), rotation, gf::Vector2f(xTarget, yTarget));
    kreature->body.color = Green;
    kreature->body.offset = 0;
    kreature->head.color = Green;
    kreature->head.offset = 0;
    kreature->limbs.color = Green;
    kreature->limbs.offset = 0;
    kreature->tail.color = Green;
    kreature->tail.offset = 0;

    kreature->timeElapsed = gf::seconds(0.0f + gRandom().computeUniformFloat(0.01f, AnimationDuration.asSeconds() - 0.01f));
    kreature->lifeCountdown = gf::seconds(std::numeric_limits<float>::max());

    m_kreatures.push_back(std::move(kreature));
  }

  void KreatureContainer::resetKreatures() {
    m_kreatures.clear();
    for (int i = 0; i < SpawnLimit; ++i) {
      // Get the initial value
      float x = gRandom().computeUniformFloat(MinBound, MaxBound);
      float y = gRandom().computeUniformFloat(MinBound, MaxBound);

      float xTarget = gRandom().computeUniformFloat(MinBound, MaxBound);
      float yTarget = gRandom().computeUniformFloat(MinBound, MaxBound);

      float rotation = gRandom().computeUniformFloat(0.0f, 2 * gf::Pi);

      auto kreature = std::make_unique<Kreature>(gf::Vector2f(x, y), rotation, gf::Vector2f(xTarget, yTarget));
      kreature->body.color = randomColor();
      kreature->body.offset = randomOffset();
      kreature->head.color = randomColor();
      kreature->head.offset = randomOffset();
      kreature->limbs.color = randomColor();
      kreature->limbs.offset = randomOffset();
      kreature->tail.color = randomColor();
      kreature->tail.offset = randomOffset();
      kreature->timeElapsed = gf::seconds(0.0f + gRandom().computeUniformFloat(0.01f, AnimationDuration.asSeconds() - 0.01f));
      kreature->lifeCountdown = gf::seconds(gRandom().computeUniformFloat(MinimumLifeTime, MaximumLifeTime));

      m_kreatures.push_back(std::move(kreature));
    }
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

    // If we move
    if (player.sideMove != 0 || player.forwardMove != 0) {
      player.timeElapsed += time;
    }

    if (player.timeElapsed >= AnimationDuration) {
      player.timeElapsed -= AnimationDuration;
      player.toggleAnimation = !player.toggleAnimation;
    }

    // Update the orientation
    player.orientation += SideVelocity * player.sideMove * time.asSeconds();
    player.orientation = std::remainder(player.orientation, 2 * gf::Pi);
    player.sideMove = 0;

    // Update the position
    float sprintFactor = 1.0f;
    if (m_isSprinting) {
      sprintFactor = SprintVeloctiy;
    }
    player.position += gf::unit(player.orientation) * ForwardVelocity * player.forwardMove * time.asSeconds() * sprintFactor;
    player.forwardMove = 0;

    player.position = gf::clamp(player.position, MinBound, MaxBound);

    // Update AI
    for (unsigned i = 1; i < m_kreatures.size(); ++i) {
      gf::ActivityStatus status = m_kreatures[i]->moveSequence.run(time);

      if (status == gf::ActivityStatus::Finished) {
        resetActivities(*m_kreatures[i]);
      }

      m_kreatures[i]->timeElapsed += time;
      if (m_kreatures[i]->timeElapsed >= AnimationDuration) {
        m_kreatures[i]->timeElapsed -= AnimationDuration;
        m_kreatures[i]->toggleAnimation = !m_kreatures[i]->toggleAnimation;
      }

      m_kreatures[i]->lifeCountdown -= time;
    }

    KrokodilePosition message;
    message.position = player.position;
    message.angle = player.orientation;
    gMessageManager().sendMessage(&message);

    // Update the food level
    float foodFactor = 1.0f;
    if (m_isSprinting) {
      foodFactor = SprintFoodConsumption;
    }
    addFoodLevel(time.asSeconds() * FoodLevelSteps * foodFactor);

    // Send stats to HUD
    KrokodileStats stats;
    stats.foodLevel = player.foodLevel;
    stats.ageLevel = player.ageLevel;
    gMessageManager().sendMessage(&stats);

    removeDeadKreature();

    // Repop if needed
    while (m_kreatures.size() < MinimumPopulation) {
      // Get the initial value
      float x = gRandom().computeUniformFloat(MinBound, MaxBound);
      float y = gRandom().computeUniformFloat(MinBound, MaxBound);
      gf::RectF viewBox({ gf::Vector2f(x, y) - 0.5f * gf::Vector2f(400.0f, 400.0f) }, { 400.0f, 400.0f });

      while (m_viewRect.intersects(viewBox)) {
        x = gRandom().computeUniformFloat(MinBound, MaxBound);
        y = gRandom().computeUniformFloat(MinBound, MaxBound);
        viewBox.setPosition({ gf::Vector2f(x, y) - 0.5f * gf::Vector2f(400.0f, 400.0f) });
      }

      float xTarget = gRandom().computeUniformFloat(MinBound, MaxBound);
      float yTarget = gRandom().computeUniformFloat(MinBound, MaxBound);

      float rotation = gRandom().computeUniformFloat(0.0f, 2 * gf::Pi);

      auto kreature = std::make_unique<Kreature>(gf::Vector2f(x, y), rotation, gf::Vector2f(xTarget, yTarget));
      kreature->body.color = randomColor();
      kreature->body.offset = randomOffset();
      kreature->head.color = randomColor();
      kreature->head.offset = randomOffset();
      kreature->limbs.color = randomColor();
      kreature->limbs.offset = randomOffset();
      kreature->tail.color = randomColor();
      kreature->tail.offset = randomOffset();
      kreature->timeElapsed = gf::seconds(0.0f + gRandom().computeUniformFloat(0.01f, AnimationDuration.asSeconds() - 0.01f));
      kreature->lifeCountdown = gf::seconds(gRandom().computeUniformFloat(MinimumLifeTime, MaximumLifeTime));

      m_kreatures.push_back(std::move(kreature));
    }
  }

  void KreatureContainer::render(gf::RenderTarget &target, const gf::RenderStates &states) {
    for (unsigned i = 0; i < m_kreatures.size(); ++i) {
      auto &kreature = m_kreatures[i];

      static constexpr gf::Vector2f BodySpriteSize = { 256.0f, 256.0f };
      static constexpr gf::Vector2f BodyWorldSize = { 128.0f, 128.0f };
      static constexpr gf::Vector2f BoxCropsVoid = { 10.0f, 10.0f };

      gf::Sprite body(m_kreatureBodyTexture, gf::RectF(kreature->body.offset * gf::Vector2f(1.0f / TotalAnimal, 0.0f), { 1.0f / TotalAnimal, 1.0f }));
      body.setScale(BodyWorldSize / BodySpriteSize);
      body.setColor(getKreatureColor(kreature->body.color));
      body.setPosition(kreature->position);
      body.setRotation(kreature->orientation);

      gf::Matrix3f bodyMatrix = body.getTransform();

      body.setAnchor(gf::Anchor::Center);

      float animationRotationOffset = 0.0f;

      if (kreature->toggleAnimation) {
        animationRotationOffset = gf::Pi / 8.0f * -1.0f;
      }
      else {
        animationRotationOffset = gf::Pi / 8.0f * +1.0f;
      }

      static constexpr gf::Vector2f HeadSpriteSize = { 256.0f, 256.0f };
      static constexpr gf::Vector2f HeadWorldSize = { 128.0f, 128.0f };
      // Not work !
      // static constexpr gf::Vector2f HeadScale = HeadWorldSize / HeadSpriteSize;

      gf::Sprite head(m_kreatureHeadTexture, gf::RectF(kreature->head.offset * gf::Vector2f(1.0f / TotalAnimal, 0.0f), { 1.0f / TotalAnimal, 1.0f }));
      head.setScale(HeadWorldSize.x / HeadSpriteSize);
      head.setAnchor(gf::Anchor::CenterLeft);
      head.setColor(getKreatureColor(kreature->head.color));
      head.setPosition(gf::transform(bodyMatrix, { m_cropBoxs[kreature->body.offset].width + BoxCropsVoid.width, 0.0f }));
      head.setRotation(kreature->orientation);
      head.draw(target, states);

      static constexpr gf::Vector2f AnteLegSpriteSize = { 128.0f, 128.0f };
      static constexpr gf::Vector2f AnteLegWorldSize = { 64.0f, 64.0f };

      gf::Sprite anteLeg(m_kreatureAnteLegTexture, gf::RectF(kreature->limbs.offset * gf::Vector2f(1.0f / TotalAnimal, 0.0f), { 1.0f / TotalAnimal, 1.0f }));
      anteLeg.setScale(AnteLegWorldSize / AnteLegSpriteSize);
      anteLeg.setAnchor(gf::Anchor::BottomCenter);
      anteLeg.setColor(getKreatureColor(kreature->limbs.color));
      anteLeg.setPosition(gf::transform(bodyMatrix, { 0.60f * m_cropBoxs[kreature->body.offset].width + BoxCropsVoid.width, -0.4f * m_cropBoxs[kreature->body.offset].height - BoxCropsVoid.height }));
      anteLeg.setRotation(kreature->orientation + animationRotationOffset);
      anteLeg.draw(target, states);
      anteLeg.scale({ 1.0f, -1.0f });
      anteLeg.setPosition(gf::transform(bodyMatrix, { 0.60f * m_cropBoxs[kreature->body.offset].width + BoxCropsVoid.width, 0.4f * m_cropBoxs[kreature->body.offset].height + BoxCropsVoid.height }));
      anteLeg.draw(target, states);

      static constexpr gf::Vector2f PostLegSpriteSize = { 128.0f, 128.0f };
      static constexpr gf::Vector2f PostLegWorldSize = { 64.0f, 64.0f };

      gf::Sprite postLeg(m_kreaturePostLegTexture, gf::RectF(kreature->limbs.offset * gf::Vector2f(1.0f / TotalAnimal, 0.0f), { 1.0f / TotalAnimal, 1.0f }));
      postLeg.setScale(PostLegWorldSize / PostLegSpriteSize);
      postLeg.setAnchor(gf::Anchor::BottomCenter);
      postLeg.setColor(getKreatureColor(kreature->limbs.color));
      postLeg.setPosition(gf::transform(bodyMatrix, { -0.40f * m_cropBoxs[kreature->body.offset].width - BoxCropsVoid.width, -0.45f * m_cropBoxs[kreature->body.offset].height - BoxCropsVoid.height}));
      postLeg.setRotation(kreature->orientation + animationRotationOffset);
      postLeg.draw(target, states);
      postLeg.setScale({ PostLegWorldSize.x / PostLegSpriteSize.x, -PostLegWorldSize.y / PostLegSpriteSize.y });
      postLeg.setPosition(gf::transform(bodyMatrix, { -0.40f * m_cropBoxs[kreature->body.offset].width - BoxCropsVoid.width, 0.45f * m_cropBoxs[kreature->body.offset].height + BoxCropsVoid.height }));
      postLeg.draw(target, states);

      static constexpr gf::Vector2f TailSpriteSize = { 256.0f, 256.0f };
      static constexpr gf::Vector2f TailWorldSize = { 128.0f, 128.0f };

      gf::Sprite tail(m_kreatureTailTexture, gf::RectF(kreature->tail.offset * gf::Vector2f(1.0f / TotalAnimal, 0.0f), { 1.0f / TotalAnimal, 1.0f }));
      tail.setScale(TailWorldSize / TailSpriteSize);
      tail.setAnchor(gf::Anchor::CenterRight);
      tail.setColor(getKreatureColor(kreature->tail.color));
      tail.setPosition(gf::transform(bodyMatrix, { -m_cropBoxs[kreature->body.offset].width - BoxCropsVoid.width - 5.0f, 0.0f }));
      tail.setRotation(kreature->orientation);
      tail.draw(target, states);

      // to print over
      body.draw(target, states);
    }
  }

  gf::MessageStatus KreatureContainer::onSizeView(gf::Id id, gf::Message *msg) {
    assert(id == ViewSize::type);
    ViewSize *viewSize = static_cast<ViewSize*>(msg);

    m_viewRect = gf::RectF(viewSize->viewCenter - 0.5f * viewSize->viewSize - gf::Vector2f(25.0f, 25.0f), viewSize->viewSize + 2 * gf::Vector2f(25.0f, 25.0f));

    return gf::MessageStatus::Keep;
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

      case Green:
        if (color2 == Azure || color2 == Yellow) {
          return 1;
        }
        if (color2 == Magenta || color2 == Red) {
          return -1;
        }
        return 0;

      case Yellow:
        if (color2 == Red || color2 == Magenta) {
          return 1;
        }
        if (color2 == Azure || color2 == Green) {
          return -1;
        }
        return 0;

      case Red:
        if (color2 == Azure || color2 == Green) {
          return 1;
        }
        if (color2 == Yellow || color2 == Magenta) {
          return -1;
        }
        return 0;

      case Magenta:
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

  KreatureContainer::Part KreatureContainer::fusionPart(KreatureContainer::Part currentPart, KreatureContainer::Part otherPart) {
    Part newPart = currentPart;

    float fusionFactor = 0.0f;
    if (colorCompare(currentPart.color, otherPart.color) == 1) {
      fusionFactor = UpperFusionFactor;
    }
    else {
      fusionFactor = LowerFusionFactor;
    }

    float rand = gRandom().computeUniformFloat(0.0f, 1.0f);
    if (rand > 0.5) {
      newPart.offset = otherPart.offset;
    }
    if (rand > fusionFactor) {
      newPart.color = otherPart.color;
    }
    else if (rand >= FumbleMutation) {
      newPart.color = randomColor();
      newPart.offset = randomOffset();
    }

    return newPart;
  }

  void KreatureContainer::addFoodLevel(float consumption) {
    auto& player = getPlayer();
    player.foodLevel += consumption;
    player.foodLevel = gf::clamp(player.foodLevel, 0.0f, FoodLevelMax);
  }

}
