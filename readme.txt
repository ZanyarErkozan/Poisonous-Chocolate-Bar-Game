Zanyar Erkozan -2584985
Boğaçhan Ayar - 2486967
CNG462 - AI Assignment 2

Poisonous Chocolate Bar Game

This program is about the Poisonous Chocolate Bar Game (Chomp game).
The game has a m x n chocolate board and one of the squares is poisonous.
Players take turns picking a square, and when you pick one it removes
that piece and everything below and to the right. The player who has
to eat the poisonous piece loses.

I implemented the Minimax algorithm so the AI can play optimally.
The program also counts how many nodes the minimax visits.

Game Modes:
- Human vs Human: two people play on the same computer
- Human vs AI: you play against the minimax AI, you can choose to go first or second
- Lara Simulation: runs 100 games automatically where Lara plays random moves against
  the AI. Board size is random between 2-5. Reports win percentages at the end.

How to run:
  python3 poisonous_chocolate_game.py

Then pick from the menu (1, 2, 3, or 4).
For moves enter row and column numbers like "2 3".

No external libraries needed, just Python 3.

Files:
  poisonous_chocolate_game.py - main program
  readme.txt - this file

Implementation Notes:
- Poison Location: The assignment says "one of the squares is poisonous" but usually it's considered (0,0). I made the program flexible so the user can choose ANY coordinate for the poison. The Minimax algorithm natively handles this because `get_moves()` dynamically skips the `(pr, pc)` coordinate, meaning the AI plays optimally no matter where the poison is.
- Node Count: The `node_count` only increments when Minimax expands a NEW node (cache miss). If a state is retrieved from memoization, it is not counted again, which gives a true reflection of the tree search size.
