QT += widgets
QT += multimedia

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ../AttackCard.cpp \
    ../BattleManager.cpp \
    ../BossStruggles.cpp \
    ../CampfireScreen.cpp \
    ../CurseCard.cpp \
    ../EliteEnemies.cpp \
    ../NormalEnemies.cpp \
    ../Potion.cpp \
    ../PowerCard.cpp \
    ../Relics.cpp \
    ../SkillCard.cpp \
    ../StatusCard.cpp \
    ../StatusEffect.cpp \
    ../card.cpp \
    ../character.cpp \
    ../enemy.cpp \
    ../player.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    ../AttackCard.h \
    ../BattleManager.h \
    ../BossStruggles.h \
    ../CampfireScreen.h \
    ../CurseCard.h \
    ../EliteEnemies.h \
    ../NormalEnemies.h \
    ../Potion.h \
    ../PowerCard.h \
    ../Relics.h \
    ../SkillCard.h \
    ../StatusCard.h \
    ../StatusEffect.h \
    ../card.h \
    ../character.h \
    ../enemy.h \
    ../player.h \
    mainwindow.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc

DISTFILES += \
    images/cards/Barricade.png \
    images/cards/BloodLetting.png \
    images/cards/Brutality.png \
    images/cards/Burn.png \
    images/cards/CurseOfTheBell.png \
    images/cards/Daze.png \
    images/cards/Defend.png \
    images/cards/DemonForm.png \
    images/cards/Disarm.png \
    images/cards/DualWield.png \
    images/cards/Entrench.png \
    images/cards/Exhume.png \
    images/cards/FeelNoPain.png \
    images/cards/Impervious.png \
    images/cards/Inflame.png \
    images/cards/J.A.X.png \
    images/cards/LimitBreak.png \
    images/cards/Metallicize.png \
    images/cards/Regret.png \
    images/cards/ShrugItOff.png \
    images/cards/Slime.png \
    images/cards/Wound.png \
    images/cards/Writhe.png \
    images/cards/offering.png \
    images/scene.png
