# Poisonous Chocolate Bar Game (Chomp)

**Author:** Zanyar Erkozan

---

## 📋 Overview

A Python implementation of the **Poisonous Chocolate Bar Game (Chomp)** with an optimal AI opponent powered by the **Minimax algorithm** and memoization.

Players take turns picking a square on an `m × n` chocolate board. Picking a square removes it **and everything below and to the right**. The player forced to eat the **poisonous square** loses.

---

## 🎮 Game Modes

| Mode | Description |
|---|---|
| **Human vs Human** | Two players take turns on the same keyboard |
| **Human vs AI** | Play against the Minimax AI — choose who goes first |
| **Lara Simulation** | 100 automated games: Lara (random moves) vs AI (optimal). Reports win percentages |

---

## 🧠 How the AI Works

The AI uses **Minimax with memoization**:

- **Minimax**: Recursively explores the game tree — Player 1 maximizes, Player 2 minimizes
- **Memoization**: Board states are cached as tuple keys to avoid recomputing the same positions (critical for 4×4+ boards)
- **Early termination**: If a winning move is found (`val = 1` for max, `val = -1` for min), the search stops immediately
- **Node count**: Reported after each AI move — only counts cache misses (true new nodes explored)

---

## 🔧 Rules

1. Board starts fully filled with chocolate
2. One square is designated as **poisonous** (default: top-left)
3. On your turn, pick any remaining square — it and everything **below + right** of it is eaten
4. The player forced to eat the poison **loses**
5. Poison can only be picked when it is the only piece remaining

---

## 🛠️ Run

```bash
python3 poisonous_chocolate_game.py
```

> Requires **Python 3** only — no external libraries needed.

For the Lara simulation directly:
```bash
python3 poisonous_chocolate_game.py --simulation
```

---

## 📁 Files

| File | Description |
|---|---|
| `poisonous_chocolate_game.py` | Full Python source (555 lines) |
| `Task1.c` | C integration — Agent Lara's escape triggers the simulation |
| `maze.txt` | Maze file used by Task1.c |

---

## 📚 Concepts Demonstrated

- Minimax game tree search
- Memoization / dynamic programming for state caching
- Chomp game state representation (2D board as nested lists)
- Priority-ordered move generation with poison handling
- Win rate analysis via automated simulation (100 games)