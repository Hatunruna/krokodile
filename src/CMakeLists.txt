cmake_minimum_required(VERSION 3.0)

project(KROKODILE
  VERSION 0.1.0
)


find_package(gf REQUIRED)
if(NOT WIN32)
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(SFML2 REQUIRED sfml-audio>=2.1)
endif()
include(GNUInstallDirs)

set(KROKODILE_DATA_DIR "${CMAKE_INSTALL_PREFIX}/share/games/krokodile")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/code/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h @ONLY)

if(NOT MSVC)
  add_definitions(-Wall -Wextra -g -O2 -std=c++14 -pedantic)
endif()

add_executable(krokodile
  code/krokodile.cc
  code/local/Hud.cc
  code/local/KonamiGamepadControl.cc
  code/local/KreatureContainer.cc
  code/local/Map.cc
  code/local/Singletons.cc
)

target_include_directories(krokodile
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/code
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(krokodile
  gf::gf0
  ${SFML2_LIBRARIES}
)

install(
  TARGETS krokodile
  RUNTIME DESTINATION games
)

install(
  DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/data/krokodile"
  DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/games"
)
