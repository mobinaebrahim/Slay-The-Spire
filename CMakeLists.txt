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

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        usermanager.h
        usermanager.cpp
        resources.qrc
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(slay-the-spire
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}


        usermanager.h usermanager.cpp
        staticsitempage.h staticsitempage.cpp staticsitempage.ui
        scoremanager.h scoremanager.cpp
        friendmanager.h friendmanager.cpp
        smtp/emailaddress.cpp smtp/emailaddress.h smtp/mimeattachment.cpp smtp/mimeattachment.h smtp/mimebase64encoder.cpp smtp/mimebase64encoder.h smtp/mimebase64formatter.cpp smtp/mimebase64formatter.h smtp/mimebytearrayattachment.cpp smtp/mimebytearrayattachment.h smtp/mimecontentencoder.cpp smtp/mimecontentencoder.h smtp/mimecontentformatter.cpp smtp/mimecontentformatter.h smtp/mimefile.cpp smtp/mimefile.h smtp/mimehtml.cpp smtp/mimehtml.h smtp/mimeinlinefile.cpp smtp/mimeinlinefile.h smtp/mimemessage.cpp smtp/mimemessage.h smtp/mimemultipart.cpp smtp/mimemultipart.h smtp/mimepart.cpp smtp/mimepart.h smtp/mimeqpencoder.cpp smtp/mimeqpencoder.h smtp/mimeqpformatter.cpp smtp/mimeqpformatter.h smtp/mimetext.cpp smtp/mimetext.h smtp/quotedprintable.cpp smtp/quotedprintable.h smtp/smtpclient.cpp smtp/smtpclient.h smtp/smtpmime_global.h
        authpage.h authpage.cpp authpage.ui
        friendspage.h friendspage.cpp friendspage.ui








        #resources.qrc
)
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET slay-the-spire APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(slay-the-spire SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
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
# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
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

include(GNUInstallDirs)
install(TARGETS slay-the-spire
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(slay-the-spire)
endif()
