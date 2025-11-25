cmake_minimum_required(VERSION 3.16)
project(QtHello LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_PREFIX_PATH "/opt/homebrew/opt/qt")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Widgets REQUIRED)

add_executable(QtHello
    main.cpp
    src/ui/SplashScreen.cpp
    src/ui/SplashScreen.h
    src/ui/LoginScreen.cpp
    src/ui/LoginScreen.h
    src/controllers/Navigation.cpp
    src/controllers/Navigation.h
    src/ui/BoardView.cpp
    src/ui/BoardView.h
)

target_include_directories(QtHello PRIVATE src)
target_link_libraries(QtHello PRIVATE Qt6::Widgets)
