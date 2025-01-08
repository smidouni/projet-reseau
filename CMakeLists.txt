# CMakeLists.txt

cmake_minimum_required(VERSION 3.16)

project(projet-reseau VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Location Positioning Graphs Quick Network QuickWidgets Qml)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Location Positioning Graphs Quick Network QuickWidgets Qml)

set(PROJECT_SOURCES
    main.cpp
    MainWindow.cpp
    MainWindow.h
    MainWindow.ui
    MapView.qml
    resources.qrc
    osmimporter.cpp
    osmimporter.h
    graph.cpp
    graph.h
    path.cpp
    path.h
    simulationmanager.cpp
    simulationmanager.h
    vehicle.cpp
    vehicle.h
    edge.h
    node.h
    blockededgesmodel.cpp
    blockededgesmodel.h
    communicationlinksmodel.h
    communicationlinksmodel.cpp
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(projet-reseau
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
else()
    if(ANDROID)
        add_library(projet-reseau SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(projet-reseau
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(projet-reseau PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Location
    Qt${QT_VERSION_MAJOR}::Positioning
    Qt${QT_VERSION_MAJOR}::Graphs
    Qt${QT_VERSION_MAJOR}::Quick
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::QuickWidgets
    Qt${QT_VERSION_MAJOR}::Qml
)

set_target_properties(projet-reseau PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_finalize_executable(projet-reseau)
endif()
