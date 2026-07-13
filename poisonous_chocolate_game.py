# Zanyar Erkozan
# CNG462 AI Assignment 2
# Poisonous Chocolate Bar Game with Minimax
# May 2026
#
# We represented the board as a 2D list because it was the easiest way
# to track which pieces are eaten. We used minimax with memoization
# because without caching the same board states, bigger boards like
# 5x5 were taking way too long.

import random
import copy
import sys
import time


# -- Board functions --
# We decided to keep the board as a simple 2D list of 0s and 1s.
# 1 means the piece is still there, 0 means it was eaten.

def create_board(m, n):
    # we fill every cell with 1 at the start since all pieces exist
    board = []
    for i in range(m):
        row = []
        for j in range(n):
            row.append(1)
        board.append(row)
    return board


def print_board(board, pr, pc):
    # we wanted the board to look clear so we used P for poison,
    # # for chocolate, and . for eaten squares. also added row/col numbers
    # so the player knows which coordinates to type
    m = len(board)
    n = len(board[0])
    
    # print column numbers
    print()
    header = "    "
    for c in range(n):
        header += " " + str(c+1) + " "
    print(header)
    print("    " + "---" * n)
    
    for r in range(m):
        line = " " + str(r+1) + " |"
        for c in range(n):
            if board[r][c] == 1:
                if r == pr and c == pc:
                    line += " P "  # poison
                else:
                    line += " # "  # normal chocolate
            else:
                line += " . "  # eaten
        line += "|"
        print(line)
    print("    " + "---" * n)
    print()


def get_moves(board, pr, pc):
    # we go through every cell and collect the ones that still have chocolate.
    # we skip the poison square because in chomp you shouldn't willingly pick it.
    # but if poison is the ONLY piece left, the player has no choice so we add it.
    moves = []
    m = len(board)
    n = len(board[0])
    
    for r in range(m):
        for c in range(n):
            if board[r][c] == 1:
                if r == pr and c == pc:
                    continue  # skip poison square
                moves.append((r, c))
    
    # if no other moves, player must eat poison
    if len(moves) == 0 and board[pr][pc] == 1:
        moves.append((pr, pc))
    
    return moves


def apply_move(board, row, col):
    # according to the chomp rules, when you pick a piece you also eat
    # everything that is below AND to the right of it. so we use deepcopy
    # to not mess up the original board, then set all those cells to 0
    new_board = copy.deepcopy(board)
    m = len(new_board)
    n = len(new_board[0])
    
    for r in range(row, m):
        for c in range(col, n):
            new_board[r][c] = 0
    
    return new_board


def check_terminal(board, pr, pc):
    # the game ends when only the poison is left on the board.
    # we check by collecting all remaining pieces and seeing if
    # the only one left is the poison square
    pieces = []
    for r in range(len(board)):
        for c in range(len(board[0])):
            if board[r][c] == 1:
                pieces.append((r, c))
    
    if len(pieces) == 1 and pieces[0] == (pr, pc):
        return True
    if len(pieces) == 0:
        return True
    return False


def total_pieces(board):
    # helper to count how many pieces are left, we use this to check
    # if the player is trying to pick poison when other options exist
    total = 0
    for row in board:
        for cell in row:
            total += cell
    return total


# -- Minimax Algorithm --
# We implemented minimax as described in the lecture slides.
# It works like DFS - goes deep into the tree first, then backtracks.
# Player 1 is max (tries to maximize), Player 2 is min (tries to minimize).

node_count = 0  # we increment this every time minimax visits a new node
cache = {}  # we added memoization here because without it 4x4+ boards were very slow


def board_key(board):
    # lists cant be dictionary keys so we convert to tuple for caching
    return tuple(tuple(row) for row in board)


def minimax(board, maximizing, pr, pc):
    # this is the core minimax function from the pseudocode in the assignment.
    # it recursively tries every possible move and returns +1 if max wins, -1 if min wins.
    # we also added early termination - if max already found a winning move (val=1)
    # there's no point checking the rest, same idea for min with val=-1
    global node_count
    
    # before doing any work, check if we already computed this state
    key = (board_key(board), maximizing)
    if key in cache:
        return cache[key]
    
    # if not in cache, we are actually visiting a new node
    node_count += 1
    
    # base case: if only the poison is left, whoever's turn it is loses
    if check_terminal(board, pr, pc):
        if maximizing:
            cache[key] = -1
            return -1  # max has to eat poison, max loses
        else:
            cache[key] = 1
            return 1   # min has to eat poison, max wins
    
    moves = get_moves(board, pr, pc)
    
    if maximizing:
        best = -999
        for move in moves:
            new_board = apply_move(board, move[0], move[1])
            val = minimax(new_board, False, pr, pc)
            if val > best:
                best = val
            if best == 1:  # cant do better than winning
                break
        cache[key] = best
        return best
    else:
        best = 999
        for move in moves:
            new_board = apply_move(board, move[0], move[1])
            val = minimax(new_board, True, pr, pc)
            if val < best:
                best = val
            if best == -1:  # cant do worse than losing
                break
        cache[key] = best
        return best


def ai_best_move(board, maximizing, pr, pc):
    # this function tries each possible move, runs minimax on the result,
    # and picks the one with the best value for the AI.
    # we reset the node counter and cache each time so we get accurate counts
    global node_count, cache
    node_count = 0
    cache = {}
    
    moves = get_moves(board, pr, pc)
    best_move = moves[0]
    
    if maximizing:
        best_val = -999
        for move in moves:
            new_board = apply_move(board, move[0], move[1])
            val = minimax(new_board, False, pr, pc)
            if val > best_val:
                best_val = val
                best_move = move
    else:
        best_val = 999
        for move in moves:
            new_board = apply_move(board, move[0], move[1])
            val = minimax(new_board, True, pr, pc)
            if val < best_val:
                best_val = val
                best_move = move
    
    return best_move, node_count


# -- Human input --

def get_player_move(board, name, pr, pc):
    # keeps asking until the player gives a valid move.
    # we convert from 1-indexed input to 0-indexed because
    # our board uses 0-based indices internally
    m = len(board)
    n = len(board[0])
    
    while True:
        try:
            raw = input("  " + name + ", enter move (row col): ").strip().split()
            if len(raw) != 2:
                print("  Please enter two numbers: row col")
                continue
            
            r = int(raw[0]) - 1
            c = int(raw[1]) - 1
            
            if r < 0 or r >= m or c < 0 or c >= n:
                print("  Out of bounds! Row: 1-" + str(m) + ", Col: 1-" + str(n))
                continue
            
            if board[r][c] == 0:
                print("  That square is already eaten!")
                continue
            
            if r == pr and c == pc and total_pieces(board) > 1:
                print("  You cant pick the poison while other pieces exist!")
                continue
            
            return (r, c)
        except ValueError:
            print("  Enter valid numbers please.")
        except (EOFError, KeyboardInterrupt):
            print("\nExiting...")
            sys.exit(0)


# -- Game Modes --

def play_human_vs_human(m, n, pr, pc):
    # basic mode where two people take turns on the same keyboard.
    # we show the board after every move so both players can see what happened
    print("\n--- Human vs Human ---")
    print("Board: " + str(m) + "x" + str(n) + ", Poison at (" + str(pr+1) + "," + str(pc+1) + ")")
    print("Pick a square to eat it + everything below-right.")
    print("Whoever eats the poison (P) loses!\n")
    
    board = create_board(m, n)
    turn = 1  # player 1 starts
    moves_played = 0
    
    while True:
        print_board(board, pr, pc)
        
        if check_terminal(board, pr, pc):
            loser = turn
            winner = 2 if turn == 1 else 1
            print("  Player " + str(loser) + " must eat the poison!")
            print("  PLAYER " + str(winner) + " WINS!")
            print("  Moves played: " + str(moves_played))
            return winner
        
        moves_played += 1
        name = "Player " + str(turn)
        print("  " + name + "'s turn (Move #" + str(moves_played) + ")")
        move = get_player_move(board, name, pr, pc)
        print("  " + name + " picks (" + str(move[0]+1) + ", " + str(move[1]+1) + ")")
        
        board = apply_move(board, move[0], move[1])
        turn = 2 if turn == 1 else 1


def play_human_vs_ai(m, n, human_first, pr, pc):
    # in this mode the human plays against our minimax AI.
    # if human goes first they are Player 1 (maximizer),
    # if AI goes first then AI is the maximizer instead.
    # we also print the node count after each AI move as required
    print("\n--- Human vs AI ---")
    print("Board: " + str(m) + "x" + str(n) + ", Poison at (" + str(pr+1) + "," + str(pc+1) + ")")
    if human_first:
        print("You go first (Player 1)")
    else:
        print("AI goes first (Player 1)")
    print()
    
    board = create_board(m, n)
    ai_maximizing = not human_first  # player1=max, player2=min
    human_turn = human_first
    total_nodes = 0
    moves_played = 0
    
    while True:
        print_board(board, pr, pc)
        
        if check_terminal(board, pr, pc):
            if human_turn:
                print("  You must eat the poison... AI WINS!")
            else:
                print("  AI must eat the poison... YOU WIN!")
            print("  Total nodes visited by AI: " + str(total_nodes))
            print("  Moves played: " + str(moves_played))
            result = "AI" if human_turn else "Human"
            return result, total_nodes
        
        moves_played += 1
        
        if human_turn:
            print("  Your turn (Move #" + str(moves_played) + ")")
            move = get_player_move(board, "You", pr, pc)
            print("  You pick (" + str(move[0]+1) + ", " + str(move[1]+1) + ")")
        else:
            print("  AI is thinking... (Move #" + str(moves_played) + ")")
            t0 = time.time()
            move, nodes = ai_best_move(board, ai_maximizing, pr, pc)
            t1 = time.time()
            total_nodes += nodes
            print("  AI picks (" + str(move[0]+1) + ", " + str(move[1]+1) + ")")
            print("  Nodes visited: " + str(nodes) + " | Time: " + str(round(t1-t0, 3)) + "s")
        
        board = apply_move(board, move[0], move[1])
        human_turn = not human_turn


# -- Lara Simulation (Assignment 1 integration) --
# For task 7, we simulate Lara playing against the AI after she escapes the maze.
# Since it's a simulation, Lara just picks random legal moves.
# We run 100 games with random board sizes between 2 and 5.

def simulate_one_game(m, n, pr, pc, show_boards=False):
    # runs a single game between Lara and AI.
    # Lara goes first so she is the maximizer, AI is minimizer
    board = create_board(m, n)
    nodes_total = 0
    lara_turn = True  # Lara starts
    move_num = 0
    
    if show_boards:
        print("\n--- NEW GAME ---")
        print("Board: " + str(m) + "x" + str(n) + ", Poison: (" + str(pr+1) + "," + str(pc+1) + ")")
    
    while True:
        if show_boards:
            print_board(board, pr, pc)
            
        if check_terminal(board, pr, pc):
            if lara_turn:
                if show_boards: print("  Lara must eat poison... AI Wins!")
                return "AI", nodes_total
            else:
                if show_boards: print("  AI must eat poison... Lara Wins!")
                return "Lara", nodes_total
        
        move_num += 1
        
        if lara_turn:
            # Lara picks a random move
            moves = get_moves(board, pr, pc)
            move = random.choice(moves)
            if show_boards:
                print("  Move #" + str(move_num) + " - Lara (random) picks (" + str(move[0]+1) + "," + str(move[1]+1) + ")")
        else:
            # AI plays optimally with minimax
            move, nodes = ai_best_move(board, False, pr, pc)  # AI is min
            nodes_total += nodes
            if show_boards:
                print("  Move #" + str(move_num) + " - AI (minimax) picks (" + str(move[0]+1) + "," + str(move[1]+1) + "), nodes visited: " + str(nodes))
        
        board = apply_move(board, move[0], move[1])
        lara_turn = not lara_turn


def lara_simulation(num_games=100):
    # runs the full simulation - 100 games with random board sizes.
    # at the end we print a table of all results and the win percentages.
    # the assignment said to report Lara's and AI's winning percentages
    print("\n--- Lara vs AI Simulation ---")
    print("Running " + str(num_games) + " games...")
    print("Board sizes randomized in range [2-5]")
    print("Lara plays randomly, AI uses Minimax")
    print()
    
    try:
        show_ans = input("  Show board for every single move? (y/n - warning, very long output): ").strip().lower()
        show_boards = (show_ans == 'y')
    except (EOFError, KeyboardInterrupt):
        # if run automatically from C without terminal input, default to False
        show_boards = False
    
    lara_wins = 0
    ai_wins = 0
    all_nodes = 0
    results = []
    
    for i in range(num_games):
        # random board size between 2 and 5
        rows = random.randint(2, 5)
        cols = random.randint(2, 5)
        
        winner, nodes = simulate_one_game(rows, cols, 0, 0, show_boards)
        all_nodes += nodes
        
        if winner == "Lara":
            lara_wins += 1
        else:
            ai_wins += 1
        
        results.append((i+1, str(rows)+"x"+str(cols), winner, nodes))
        
        if (i+1) % 10 == 0:
            print("  " + str(i+1) + "/" + str(num_games) + " done...")
    
    # print results table
    print()
    print("Game   Board   Winner   Nodes")
    print("-" * 38)
    for game_num, board_size, winner, nodes in results:
        print(str(game_num).ljust(7) + board_size.ljust(8) + winner.ljust(9) + str(nodes))
    
    # print summary
    print()
    print("=== RESULTS ===")
    print("Total games: " + str(num_games))
    print("Lara wins:   " + str(lara_wins) + " (" + str(round(lara_wins/num_games*100, 1)) + "%)")
    print("AI wins:     " + str(ai_wins) + " (" + str(round(ai_wins/num_games*100, 1)) + "%)")
    print("Total nodes: " + str(all_nodes))
    print("Avg nodes/game: " + str(round(all_nodes/num_games, 1)))
    
    return lara_wins, ai_wins


# -- Main Menu --

def get_board_input():
    # asks the user for board dimensions and where the poison is.
    # we made the poison default to top-left (1,1) since that's the
    # standard position, but the assignment said it should be configurable
    while True:
        try:
            m = int(input("  Rows (m): "))
            n = int(input("  Cols (n): "))
            if m < 2 or n < 2:
                print("  Board must be at least 2x2")
                continue
            break
        except ValueError:
            print("  Enter valid numbers")
    
    while True:
        print("  Poison location (default: 1 1)")
        p_input = input("  Press enter for default or type 'row col': ").strip()
        
        if p_input == "":
            pr, pc = 0, 0
            break
        else:
            try:
                parts = p_input.split()
                pr = int(parts[0]) - 1
                pc = int(parts[1]) - 1
                if pr < 0 or pr >= m or pc < 0 or pc >= n:
                    print("  Invalid location! Row must be 1-" + str(m) + " and Col 1-" + str(n))
                    continue
                break
            except:
                print("  Invalid input format! Please enter numbers like '1 1'")
                continue
    
    return m, n, pr, pc


def main():
    # If run with --simulation flag, it means it was called from Assignment 1 C code
    # where Lara just escaped the maze and now must play the game.
    if len(sys.argv) > 1 and sys.argv[1] == "--simulation":
        print("\n*** ASSIGNMENT 1 INTEGRATION: LARA ESCAPED THE MAZE! ***")
        print("*** SHE NOW HAS TO PLAY THE POISONOUS CHOCOLATE GAME ***")
        lara_simulation(100)
        return
        
    print()
    print("================================")
    print(" Poisonous Chocolate Bar Game")
    print(" Zanyar Erkozan")
    print("================================")
    
    while True:
        print()
        print("Menu:")
        print("  1) Human vs Human")
        print("  2) Human vs AI")
        print("  3) Lara Simulation (100 games)")
        print("  4) Quit")
        
        try:
            choice = input("  Choice: ").strip()
        except (EOFError, KeyboardInterrupt):
            print("\nBye!")
            break
        
        if choice == "1":
            m, n, pr, pc = get_board_input()
            play_human_vs_human(m, n, pr, pc)
        
        elif choice == "2":
            m, n, pr, pc = get_board_input()
            
            # we noticed minimax gets slow on big boards so we warn the user
            if m * n > 20:
                print("  Warning: big board, AI might be slow!")
                c = input("  Continue? (y/n): ").strip().lower()
                if c != "y":
                    continue
            
            order = input("  Go first? (y/n): ").strip().lower()
            human_first = (order == "y")
            play_human_vs_ai(m, n, human_first, pr, pc)
        
        elif choice == "3":
            lara_simulation(100)
        
        elif choice == "4":
            print("Bye!")
            break
        
        else:
            print("  Invalid choice")


if __name__ == "__main__":
    main()
