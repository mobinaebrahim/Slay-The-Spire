cmake_minimum_required(VERSION 3.16)
project(slay-the-spire VERSION 0.1 LANGUAGES CXX)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets Multimedia MultimediaWidgets Sql Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Multimedia MultimediaWidgets Sql Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Quick)

# ============================================================
# IMPORTANT: resources.qrc is intentionally NOT embedded via
# AUTORCC. It is excluded below and compiled separately into
# a standalone .rcc binary (see "External resource compilation"
# section at the bottom). This avoids generating a huge
# qrc_resources.cpp file that blows up compiler RAM usage.
# ------------------------------------------------------------
set_source_files_properties(resources.qrc PROPERTIES SKIP_AUTORCC ON)

set(PROJECT_SOURCES
        main.cpp
        mainmenuwindow.cpp
        mainmenuwindow.h
        mainmenuwindow.ui
        usermanager.h
        usermanager.cpp
        savemanager.h
        savemanager.cpp
)
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(slay-the-spire
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        staticsitempage.h staticsitempage.cpp staticsitempage.ui
        scoremanager.h scoremanager.cpp
        friendmanager.h friendmanager.cpp
        smtp/emailaddress.cpp smtp/emailaddress.h smtp/mimeattachment.cpp smtp/mimeattachment.h smtp/mimebase64encoder.cpp smtp/mimebase64encoder.h smtp/mimebase64formatter.cpp smtp/mimebase64formatter.h smtp/mimebytearrayattachment.cpp smtp/mimebytearrayattachment.h smtp/mimecontentencoder.cpp smtp/mimecontentencoder.h smtp/mimecontentformatter.cpp smtp/mimecontentformatter.h smtp/mimefile.cpp smtp/mimefile.h smtp/mimehtml.cpp smtp/mimehtml.h smtp/mimeinlinefile.cpp smtp/mimeinlinefile.h smtp/mimemessage.cpp smtp/mimemessage.h smtp/mimemultipart.cpp smtp/mimemultipart.h smtp/mimepart.cpp smtp/mimepart.h smtp/mimeqpencoder.cpp smtp/mimeqpencoder.h smtp/mimeqpformatter.cpp smtp/mimeqpformatter.h smtp/mimetext.cpp smtp/mimetext.h smtp/quotedprintable.cpp smtp/quotedprintable.h smtp/smtpclient.cpp smtp/smtpclient.h smtp/smtpmime_global.h
        authpage.h authpage.cpp authpage.ui
        friendspage.h friendspage.cpp friendspage.ui
        mapnode.cpp mapnode.h
        gamemap.h gamemap.cpp
        mapview.h mapview.cpp
        mappage.h mappage.cpp mappage.ui
        mapnodeitem.h
        networkmanager.h networkmanager.cpp
        audiomanager.h audiomanager.cpp
        settingpage.h settingpage.cpp settingpage.ui
        cursormanager.h cursormanager.cpp
        player.cpp player.h
        enemy.cpp enemy.h
        character.cpp character.h
        BattleManager.cpp BattleManager.h
        card.cpp card.h
        AttackCard.cpp AttackCard.h
        CurseCard.cpp CurseCard.h
        SkillCard.cpp SkillCard.h
        PowerCard.cpp PowerCard.h
        StatusCard.cpp StatusCard.h
        StatusEffect.cpp StatusEffect.h
        BossStruggles.cpp BossStruggles.h
        NormalEnemies.cpp NormalEnemies.h
        EliteEnemies.cpp EliteEnemies.h
        cardfactory.h cardfactory.cpp
        mpcombatwindow.h mpcombatwindow.cpp mpcombatwindow.ui
        mainwindow.ui
        mainwindow.h
        mainwindow.cpp
        combattype.h
        resources.qrc
        CampfireScreen.cpp CampfireScreen.h
        EventScreen.cpp EventScreen.h
        EventScreen.cpp EventScreen.h
        Potion.cpp Potion.h
        ShopScreen.cpp ShopScreen.h
        Relics.cpp Relics.h
        treasurescreen.h treasurescreen.cpp
    )
else()
    if(ANDROID)
        add_library(slay-the-spire SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(slay-the-spire
            ${PROJECT_SOURCES}
        )
    endif()
endif()
target_link_libraries(slay-the-spire PRIVATE
    Qt6::Widgets
    Qt6::Multimedia
    Qt6::MultimediaWidgets
    Qt6::Sql
    Qt6::Network
)
target_link_libraries(slay-the-spire PRIVATE Qt${QT_VERSION_MAJOR}::Quick)
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.slay-the-spire)
endif()
set_target_properties(slay-the-spire PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

# ============================================================
# External resource compilation
# ------------------------------------------------------------
# resources.qrc is compiled into a standalone binary .rcc file
# instead of being embedded into a generated C++ source file.
# This keeps compiler memory usage low regardless of asset size.
# The resulting resources.rcc must be loaded at runtime via
# QResource::registerResource() (see main.cpp).
# ------------------------------------------------------------

get_target_property(QT_RCC_EXECUTABLE Qt${QT_VERSION_MAJOR}::rcc IMPORTED_LOCATION)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/resources.rcc
    COMMAND ${QT_RCC_EXECUTABLE} --binary
            ${CMAKE_CURRENT_SOURCE_DIR}/resources.qrc
            -o ${CMAKE_CURRENT_BINARY_DIR}/resources.rcc
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/resources.qrc
    COMMENT "Compiling resources.qrc -> resources.rcc (external, not embedded in C++)"
    VERBATIM
)

add_custom_target(compile_resources ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/resources.rcc)
add_dependencies(slay-the-spire compile_resources)

add_custom_command(TARGET slay-the-spire POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_BINARY_DIR}/resources.rcc
            $<TARGET_FILE_DIR:slay-the-spire>/resources.rcc
)

include(GNUInstallDirs)
install(TARGETS slay-the-spire
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(slay-the-spire)
endif()