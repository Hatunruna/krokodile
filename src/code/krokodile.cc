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

#include <gf/Action.h>
#include <gf/Clock.h>
#include <gf/Color.h>
#include <gf/EntityContainer.h>
#include <gf/Event.h>
#include <gf/Shapes.h>
#include <gf/RenderWindow.h>
#include <gf/ViewContainer.h>
#include <gf/Views.h>
#include <gf/Window.h>

#include "config.h"
#include "local/KreatureContainer.h"
#include "local/Messages.h"
#include "local/Singletons.h"

int main() {
  static constexpr gf::Vector2u ScreenSize(1024, 576);
  static constexpr gf::Vector2f ViewSize(1000.0f, 1000.0f); // dummy values
  static constexpr gf::Vector2f ViewCenter(0.0f, 0.0f); // dummy values

  // Set the singletons
  gf::SingletonStorage<gf::ResourceManager> storageForResourceManager(kkd::gResourceManager);
  kkd::gResourceManager().addSearchDir(KROKODILE_DATA_DIR);

  gf::SingletonStorage<gf::MessageManager> storageForMessageManager(kkd::gMessageManager);
  gf::SingletonStorage<gf::Random> storageForRandom(kkd::gRandom);

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

  // entities
  gf::EntityContainer mainEntities;
  kkd::KreatureContainer kreatures;
  mainEntities.addEntity(kreatures);

  gf::EntityContainer hudEntities;
  // add entities to hudEntities

  // game loop
  renderer.clear(gf::Color::lighter(gf::Color::Chartreuse));
  gf::Clock clock;
  while (window.isOpen()) {
    // 1. input
    gf::Event event;
    while (window.pollEvent(event)) {
      actions.processEvent(event);
      views.processEvent(event);
    }

    if (closeWindowAction.isActive()) {
      window.close();
    }

    if (fullscreenAction.isActive()) {
      window.toggleFullscreen();
    }

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

    // 2. update
    gf::Time time = clock.restart();
    mainEntities.update(time);
    hudEntities.update(time);

    // 3. draw
    renderer.clear();

    renderer.setView(mainView);
    mainEntities.render(renderer);

    renderer.setView(hudView);
    hudEntities.render(renderer);

    renderer.display();
    actions.reset();
  }
  return 0;
}
