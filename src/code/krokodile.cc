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
#include <cassert>

#include <gf/Anchor.h>
#include <gf/Action.h>
#include <gf/Clock.h>
#include <gf/Color.h>
#include <gf/Controls.h>
#include <gf/Coordinates.h>
#include <gf/EntityContainer.h>
#include <gf/Event.h>
#include <gf/RenderWindow.h>
#include <gf/Shapes.h>
#include <gf/Text.h>
#include <gf/ViewContainer.h>
#include <gf/Views.h>
#include <gf/Window.h>

#include <iostream>

#include "config.h"
#include "local/Hud.h"
#include "local/KreatureContainer.h"
#include "local/Map.h"
#include "local/Messages.h"
#include "local/Singletons.h"

#define UNUSED(x) (void)(x)

int main() {
  bool isGameComplete = false;
  int nbGen = 0;

  static constexpr gf::Vector2u ScreenSize(1024, 576);
  static constexpr gf::Vector2f ViewSize(1000.0f, 1000.0f); // dummy values
  static constexpr gf::Vector2f ViewCenter(0.0f, 0.0f); // dummy values

  // Set the singletons
  gf::SingletonStorage<gf::ResourceManager> storageForResourceManager(kkd::gResourceManager);
  kkd::gResourceManager().addSearchDir(KROKODILE_DATA_DIR);

  gf::SingletonStorage<gf::MessageManager> storageForMessageManager(kkd::gMessageManager);
  gf::SingletonStorage<gf::Random> storageForRandom(kkd::gRandom);

  gf::Clock startClock;
  float endTime;
  kkd::gMessageManager().registerHandler<kkd::CompleteGame>(
      [&isGameComplete, &startClock, &endTime](gf::Id type, gf::Message *msg) {
        assert(type == kkd::CompleteGame::type);
        UNUSED(msg);
        isGameComplete = true;
        endTime = startClock.getElapsedTime().asSeconds();
        return gf::MessageStatus::Keep;
  });

  // initialization
  gf::Window window("Krokodile", ScreenSize);
  window.setVerticalSyncEnabled(true);
  window.setFramerateLimit(60);

  gf::RenderWindow renderer(window);

  // views
  gf::ViewContainer views;
  gf::ExtendView mainView(ViewCenter, ViewSize);
  views.addView(mainView);

  gf::ScreenView hudView;
  views.addView(hudView);
  views.setInitialScreenSize(ScreenSize);

  kkd::gMessageManager().registerHandler<kkd::KrokodilePosition>([&mainView](gf::Id type, gf::Message *msg) {
    assert(type == kkd::KrokodilePosition::type);
    auto positionKrokodileMessage = static_cast<kkd::KrokodilePosition*>(msg);
    mainView.setCenter(positionKrokodileMessage->position);
    return gf::MessageStatus::Keep;
  });

  // actions
  gf::ActionContainer actions;
  gf::Action closeWindowAction("Close window");
  closeWindowAction.addCloseControl();
  closeWindowAction.addKeycodeKeyControl(gf::Keycode::Escape);
  actions.addAction(closeWindowAction);

  gf::Action fullscreenAction("Fullscreen");
  fullscreenAction.addKeycodeKeyControl(gf::Keycode::F);
  actions.addAction(fullscreenAction);

  gf::Action leftAction("Left");
  leftAction.addScancodeKeyControl(gf::Scancode::A);
  leftAction.addScancodeKeyControl(gf::Scancode::Left);
  leftAction.setContinuous();
  actions.addAction(leftAction);

  gf::Action rightAction("Right");
  rightAction.addScancodeKeyControl(gf::Scancode::D);
  rightAction.addScancodeKeyControl(gf::Scancode::Right);
  rightAction.setContinuous();
  actions.addAction(rightAction);

  gf::Action upAction("Up");
  upAction.addScancodeKeyControl(gf::Scancode::W);
  upAction.addScancodeKeyControl(gf::Scancode::Up);
  upAction.setContinuous();
  actions.addAction(upAction);

  gf::Action downAction("Down");
  downAction.addScancodeKeyControl(gf::Scancode::S);
  downAction.addScancodeKeyControl(gf::Scancode::Down);
  downAction.setContinuous();
  actions.addAction(downAction);

  gf::Action swapAction("Swap");
  swapAction.addScancodeKeyControl(gf::Scancode::Tab);
  actions.addAction(swapAction);

  gf::Action fusionAction("Fusion");
  fusionAction.addScancodeKeyControl(gf::Scancode::Space);
  actions.addAction(fusionAction);

  //Konami
  gf::KonamiKeyboardControl konami;

  gf::Action easterEgg("Easter egg");
  easterEgg.addControl(konami);
  easterEgg.setInstantaneous();

  // entities
  gf::EntityContainer mainEntities;

  kkd::Map map;
  mainEntities.addEntity(map);

  kkd::KreatureContainer kreatures;
  mainEntities.addEntity(kreatures);

  gf::EntityContainer hudEntities;

  // add entities to hudEntities
  kkd::Hud hud;
  hudEntities.addEntity(hud);

  // game loop
  renderer.clear(gf::Color::lighter(gf::Color::Chartreuse));
  gf::Clock clock;
  while (window.isOpen()) {
    // 1. input
    gf::Event event;
    while (window.pollEvent(event)) {
      actions.processEvent(event);
      views.processEvent(event);

      easterEgg.processEvent(event);
    }

    if (closeWindowAction.isActive()) {
      window.close();
    }

    if (fullscreenAction.isActive()) {
      window.toggleFullscreen();
    }

    // Movement
    if (rightAction.isActive()) {
      kreatures.playerSidedMove(1);
    } else if (leftAction.isActive()) {
      kreatures.playerSidedMove(-1);
    }
    if (upAction.isActive()) {
        kreatures.playerForwardMove(1);
    } else if (downAction.isActive()) {
      kreatures.playerForwardMove(-1);
    }

    if (swapAction.isActive()) {
      kreatures.swapKreature();
    }
    if (fusionAction.isActive()) {
      kreatures.fusionDNA();
      nbGen++;
    }

    if (easterEgg.isActive()) {
      //Do event or message
      std::cout << "###############\n";
      std::cout << "# Easter egg! #\n";
      std::cout << "###############\n";
    }

    // 2. update
    gf::Time time = clock.restart();
    if (!isGameComplete) {
      // 2. update
      mainEntities.update(time);
      hudEntities.update(time);
    }

    // 3. draw
    renderer.clear();

    if (!isGameComplete) {
      renderer.setView(mainView);
      mainEntities.render(renderer);

      renderer.setView(hudView);
      hudEntities.render(renderer);
    } else {
      renderer.setView(hudView);

      gf::Coordinates coords(renderer);
      gf::Vector2f screenCenter = coords.getCenter();

      gf::Text timeText("Time : " + std::to_string((int)endTime) + " seconds", kkd::gResourceManager().getFont("blkchcry.ttf"), 100);
      timeText.setOutlineColor(gf::Color::Black);
      timeText.setOutlineThickness(2.0f);
      timeText.setColor(gf::Color::White);
      timeText.setPosition(screenCenter);
      timeText.setAnchor(gf::Anchor::Center);
      timeText.setParagraphWidth(2000.0f);
      timeText.setAlignment(gf::Alignment::Center);

      gf::Text genText("Generations : " + std::to_string(nbGen), kkd::gResourceManager().getFont("blkchcry.ttf"), 100);
      genText.setOutlineColor(gf::Color::Black);
      genText.setOutlineThickness(2.0f);
      genText.setColor(gf::Color::White);
      gf::Vector2f genTextPos = timeText.getPosition();
      genTextPos.y -= timeText.getLocalBounds().height / 2;
      genText.setPosition(genTextPos);
      genText.setAnchor(gf::Anchor::BottomCenter);

      int finalScore = (int)((10000.0f / (nbGen * endTime + 1)) * 1000.0f);
      gf::Text scoreText("Score : " + std::to_string(finalScore), kkd::gResourceManager().getFont("blkchcry.ttf"), 100);
      scoreText.setOutlineColor(gf::Color::Black);
      scoreText.setOutlineThickness(2.0f);
      scoreText.setColor(gf::Color::White);
      gf::Vector2f scoreTextPos = timeText.getPosition();
      scoreTextPos.y += timeText.getLocalBounds().height / 2;
      scoreText.setPosition(scoreTextPos);
      scoreText.setAnchor(gf::Anchor::TopCenter);

      renderer.draw(scoreText);
      renderer.draw(genText);
      renderer.draw(timeText);
    }

    renderer.display();
    actions.reset();
    easterEgg.reset();
  }
  return 0;
}
