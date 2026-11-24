cmake_minimum_required(VERSION 3.16)
project(PrimitivesApp VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_PREFIX_PATH "C:/Qt/Qt5.14.2/5.14.2/mingw73_64")

find_package(Qt5 REQUIRED COMPONENTS Widgets Sql)

add_executable(PrimitivesApp
    src/main.cpp
    src/db/DatabaseManager.h
    src/db/DatabaseManager.cpp
    src/ui/MainWindow.h
    src/ui/MainWindow.cpp
    src/scene/PrimitiveItem.h
    src/scene/PrimitiveItem.cpp
    src/scene/ConnectionLine.h
    src/scene/ConnectionLine.cpp
    src/scene/GraphicsScene.h
    src/scene/GraphicsScene.cpp
)

target_include_directories(PrimitivesApp PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(PrimitivesApp PRIVATE
    Qt5::Widgets
    Qt5::Sql
)