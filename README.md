# Undaunted (Qt/C++)

A 2-player, turn-based tactical board game implemented with **Qt Widgets** and **C++17**.
The project models a card-driven combat system with hex-grid movement, special agent actions, scenario loading, and win-condition evaluation.

## Overview

Each player controls 3 agents:
- Scout
- Sniper
- Sergeant

Each player also has a 10-card deck:
- 4x Scout cards
- 3x Sniper cards
- 3x Sergeant cards

At game start, each deck is shuffled. On each turn:
1. The current player draws the top card.
2. They perform exactly one action with the agent type on that card.
3. The card is returned to the bottom of that player's deck.
4. Turn passes to the opponent automatically.

## Core Rules (Implemented)

### Movement
- All moves are to an adjacent hex only.
- Destination must be empty.
- `Scout`: can move to any adjacent empty cell.
- `Sniper`: destination must already be marked by that player.
- `Sergeant`: destination must already be marked by that player.

### Combat
- Attack threshold: `sum(shields on shortest path excluding endpoints) + target HP`
- Threshold is clamped to `[1, 10]`.
- Dice: `Sniper` uses 3 d10, `Scout` and `Sergeant` use 1 d10.
- Attack is successful if at least one die is `>= threshold`.
- On success, one card of the target agent type is burned from the defender's deck.
- If all cards of that target agent type are gone, that agent is eliminated from the board.

### Special Actions
- `Scout Mark`: marks Scout's current cell for its owner.
- `Sergeant Control`: controls current cell if valid.
- `Sergeant Release`: releases enemy-controlled current cell if valid.

### Win Conditions
A player wins if either:
1. They control at least 7 cells.
2. All 3 enemy agents are eliminated.

## Input Files

The project uses two text formats:

### 1) Board file (`src/assets/boards/*.txt`)
Defines hex cells and shield values.

Example token:
```txt
|A01:2
```
- `A01` is cell ID
- `2` is shield value

### 2) Scenario file (`src/assets/maps/*.txt`)
Defines initial placements/marks/control.

Examples:
```txt
A03:A,scout
A13:A,mark
A15:B,control
```

When a scenario file is selected, the board with the same filename is loaded from `assets/boards`.

## UI Flow

1. Splash screen
2. Login screen: validates both player names (length, upper/lower/digit/special-char constraints) and opens map/scenario selection.
3. Board screen: renders the hex board, shows turn/active-card HUD, and provides actions (`Move`, `Attack`, `Scout Mark`, `Sergeant Control`, `Sergeant Release`).

## Architecture (OOP)

Main gameplay logic is split into focused modules:

- `GameSession`: battle lifecycle and command execution boundary.
- `TurnEngine`: validates whether the current turn can act (game status + active card).
- `ActionCommand` hierarchy: `MoveCommand`, `AttackCommand`, `UseAgentSpecialCommand`; auto-advances turn after a successful action.
- `AgentBehavior` polymorphism: agent-specific movement, attack dice count, and special-action capability.
- `TurnSystem`: deck shuffle, draw, end-turn, card burn.
- `Combat`, `Movement`, `TacticalActions`: isolated game mechanics for easier maintenance/refactoring.
- `Victory`: evaluates and updates game status.
- `ScenarioLoader`: parses scenario files and applies initial board state.

## Build & Run

### Prerequisites
- C++17 compiler
- CMake >= 3.16
- Qt6 Widgets

### Build
Note: `CMakeLists.txt` currently sets `CMAKE_PREFIX_PATH` to `/opt/homebrew/opt/qt` (Apple Silicon Homebrew). Change it for your environment if needed.

```bash
cmake -S . -B build
cmake --build build -j
```

### Run
```bash
./build/QtHello
```

## Project Structure

```txt
src/
  game/
    actions/        # Combat, movement, tactical actions
    agents/         # Agent behavior polymorphism
    board/          # Board graph parsing + BFS/shortest path
    model/          # Core state/types/init
    rules/          # Win condition logic
    scenario/       # Scenario parser and applier
    session/        # Session orchestration + commands + turn validation
    turn/           # Deck/turn card flow
  ui/               # Splash, login, board view
  controllers/      # Navigation between screens
```
