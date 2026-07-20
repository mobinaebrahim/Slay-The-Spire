cmake_minimum_required(VERSION 3.16)
project(slay-the-spire-server LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Network)

add_executable(slay-the-spire-server
  main.cpp
  gameserver.h gameserver.cpp
  AttackCard.cpp AttackCard.h
  BattleManager.cpp BattleManager.h
  BossStruggles.cpp BossStruggles.h
  card.cpp card.h
  character.cpp character.h
  CurseCard.cpp CurseCard.h
  EliteEnemies.cpp EliteEnemies.h
  enemy.cpp enemy.h
  NormalEnemies.cpp NormalEnemies.h
  player.cpp player.h
  PowerCard.cpp PowerCard.h
  SkillCard.cpp SkillCard.h
  StatusCard.cpp StatusCard.h
  StatusEffect.cpp StatusEffect.h
  CardFactory.cpp CardFactory.h
)

target_link_libraries(slay-the-spire-server PRIVATE Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Network)

include(GNUInstallDirs)
install(TARGETS slay-the-spire-server
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)