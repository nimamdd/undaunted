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
    src/game/GameModel.h
    src/game/model/Types.h
    src/game/model/Init.h
    src/game/model/Init.cpp
    src/game/agents/AgentBehavior.h
    src/game/agents/AgentBehavior.cpp
    src/game/rules/Victory.h
    src/game/rules/Victory.cpp
    src/game/board/BoardGraph.h
    src/game/board/BoardGraph.cpp
    src/game/actions/Combat.h
    src/game/actions/Combat.cpp
    src/game/actions/Movement.h
    src/game/actions/Movement.cpp
    src/game/actions/TacticalActions.h
    src/game/actions/TacticalActions.cpp
    src/game/scenario/ScenarioLoader.h
    src/game/scenario/ScenarioLoader.cpp
    src/game/session/SessionTypes.h
    src/game/session/TurnEngine.h
    src/game/session/TurnEngine.cpp
    src/game/session/GameSession.h
    src/game/session/GameSession.cpp
    src/game/session/ActionCommand.h
    src/game/session/ActionCommand.cpp
    src/game/turn/TurnSystem.h
    src/game/turn/TurnSystem.cpp
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
