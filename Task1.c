#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
//created by zanyar erkozan
// starting date: 7.04.2026
/* Directory containing the executable (so maze.txt is found even when cwd differs). */
static char g_exe_dir[1024];

static void init_exe_dir(int argc, char** argv) {
    g_exe_dir[0] = '\0';
    if (argc < 1 || argv == NULL || argv[0] == NULL) return;
    if (strlen(argv[0]) >= sizeof(g_exe_dir)) return;
    strcpy(g_exe_dir, argv[0]);
    char* slash = strrchr(g_exe_dir, '/');
    if (slash) *slash = '\0';
}


#define INF 1000000
#define MAX_GP_STATES 1000
#define MAX_ROOMS 36

// map nodes
// we use a 2d array for the map

#define NODE_S  0   // Start
#define NODE_F  1   // Safe House (Fortress)
#define NODE_C1 2   // Challenge 1
#define NODE_C2 3   // Challenge 2
#define NODE_E  4   // Exit
#define NUM_NODES 5

// Graph adjacency matrix (weighted)
int graph[NUM_NODES][NUM_NODES] = {
    //  S    F    C1   C2    E
    {  0,   80,  99,   0,   0  },  // S
    {  80,   0,   0,  97,   0  },  // F
    {  99,   0,   0,   0, 211  },  // C1
    {   0,  97,   0,   0, 101  },  // C2
    {   0,   0, 211, 101,   0  }   // E
};

// challenge 1 part - guards and prisoners
// A* algorithm structs


typedef struct {
    int g, p, b;        // guards, prisoners on left bank; boat: 0=left shore, 1=right shore
    int cost_g;         // g(n) cost
    int cost_h;         // h(n) heuristic
    int cost_f;         // total cost f(n)
    int parent;         // for printing path later
} GPState;

typedef struct {
    GPState states[MAX_GP_STATES];
    int count;
} GPFrontier;

const int gp_moves[5][2] = {
    {1, 0}, {2, 0}, {0, 1}, {0, 2}, {1, 1}
};

// check if the state is valid
bool gp_isValid(int g, int p) {
    if (g < 0 || g > 3 || p < 0 || p > 3) return false; // checking negative or over 3
    if (g > 0 && g < p) return false;
    if ((3-g) > 0 && (3-g) < (3-p)) return false;
    return true;
}

// heuristic
// ceil((G+P)/2) because boat takes at most 2 people
int gp_heuristic(int g, int p) { return (int)ceil((g + p) / 2.0); }

// adding to frontier array
// sorts by f cost
void gp_addToFrontier(GPFrontier* f, GPState s) {
    int i;
    for (i = f->count - 1; i >= 0; i--) {
        if (f->states[i].cost_f <= s.cost_f) break;
        f->states[i + 1] = f->states[i];
    }
    f->states[i + 1] = s;
    f->count++;
}

// checks if state is already inside the frontier
bool gp_inFrontier(GPFrontier* f, GPState s) {
    for (int i = 0; i < f->count; i++) {
        if (f->states[i].g == s.g && f->states[i].p == s.p && f->states[i].b == s.b) return true;
    }
    return false;
}

bool gp_stateExists(GPState* explored, int count, GPState s) {
    for (int i = 0; i < count; i++) {
        if (explored[i].g == s.g && explored[i].p == s.p && explored[i].b == s.b)
            return true;
    }
    return false;
}

void gp_printState(GPState s) { printf("(%d,%d,%d)", s.g, s.p, s.b); }

void gp_printPath(GPState* explored, GPState goal) {
    if (goal.parent != -1) {
        gp_printPath(explored, explored[goal.parent]);
        printf(" -> ");
    }
    gp_printState(goal);
}

bool challenge1_solve() {
    printf("\n  ======= Challenge 1: Guards and Prisoners (A*) =======\n");
    printf("  Initial: (3,3,0)  |  Goal: (0,0,1)\n\n");

    GPFrontier frontier;
    frontier.count = 0;
    GPState explored[MAX_GP_STATES];
    int explored_count = 0;
    int total = 0;

    GPState start = {3, 3, 0, 0, 3, 3, -1};
    gp_addToFrontier(&frontier, start);

    while (frontier.count > 0) {
        // get the smallest f
        GPState cur = frontier.states[0];
        for (int i = 0; i < frontier.count - 1; i++)
            frontier.states[i] = frontier.states[i + 1];
        frontier.count--;

        if (gp_stateExists(explored, explored_count, cur)) continue;

        explored[explored_count] = cur;
        explored_count++;

        // printf("debug: pushing to explored array\n");

        printf("  Expanding: ");
        gp_printState(cur);
        printf("  [g=%d, h=%d, f=%d]\n", cur.cost_g, cur.cost_h, cur.cost_f);

        /* Goal: nobody left on the left bank (all crossed) and boat position matches (0,0,1). */
        if (cur.g == 0 && cur.p == 0 && cur.b == 1) {
            printf("\n  Goal reached!\n  Solution path: ");
            gp_printPath(explored, cur);
            printf("\n  Total moves: %d\n", cur.cost_g);
            printf("  States generated: %d | States explored: %d\n", total, explored_count);
            printf("  ======= Challenge 1 Completed! =======\n\n");
            return true;
        }

        for (int i = 0; i < 5; i++) {
            int ng = cur.g, np = cur.p;
            if (cur.b == 0) { ng -= gp_moves[i][0]; np -= gp_moves[i][1]; }
            else             { ng += gp_moves[i][0]; np += gp_moves[i][1]; }

            if (gp_isValid(ng, np)) {
                GPState ns;
                ns.g = ng; ns.p = np; ns.b = 1 - cur.b;
                ns.cost_g = cur.cost_g + 1;
                ns.cost_h = gp_heuristic(ng, np);
                ns.cost_f = ns.cost_g + ns.cost_h;
                ns.parent = explored_count - 1;
                if (!gp_stateExists(explored, explored_count, ns) && !gp_inFrontier(&frontier, ns)) {
                    gp_addToFrontier(&frontier, ns);
                    total++;
                }
            }
        }
    }
    printf("  No solution found!\n");
    return false;
}

// challenge 2 part - maze
// Greedy best first search


typedef struct {
    char name[4];
    int row, col;
    char connections[4][4];
    int num_connections;
} Room;

typedef struct {
    int nodes[MAX_ROOMS];
    int heuristics[MAX_ROOMS];
    int count;
} MazeFrontier;

Room rooms[MAX_ROOMS];
int num_rooms = 0;
bool adj_matrix[MAX_ROOMS][MAX_ROOMS];
char scientist_loc[4], exit_loc[4];
int scientist_idx = -1, exit_idx = -1;
bool maze_loaded = false;

// manhattan distance
int getManhattan(Room* a, Room* b) {
    return abs(a->row - b->row) + abs(a->col - b->col);
}

int getRoomIdx(const char* name) {
    for (int i = 0; i < num_rooms; i++)
        if (strcmp(rooms[i].name, name) == 0) return i;
    return -1;
}

void parseLine(char* line) {
    Room room;
    room.num_connections = 0;
    char* tok = strtok(line, " ");
    if (!tok) return;
    strcpy(room.name, tok);
    tok = strtok(NULL, " "); if (!tok) return; room.row = atoi(tok);
    tok = strtok(NULL, " "); if (!tok) return; room.col = atoi(tok);
    while ((tok = strtok(NULL, " \n")) != NULL && room.num_connections < 4)
        strcpy(room.connections[room.num_connections++], tok);
    rooms[num_rooms++] = room;
}

bool loadMaze(const char* filename) {
    if (maze_loaded) return true;
    FILE* f = fopen(filename, "r");
    char alt[1100];
    if (!f && g_exe_dir[0] != '\0') {
        snprintf(alt, sizeof(alt), "%s/%s", g_exe_dir, filename);
        f = fopen(alt, "r");
    }
    if (!f) { printf("Error: Could not open %s\n", filename); return false; }
    num_rooms = 0;
    memset(adj_matrix, false, sizeof(adj_matrix));
    char line[100];
    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, "mad scientest", 13) == 0) { sscanf(line, "mad scientest %s", scientist_loc); continue; }
        if (strncmp(line, "mad scientist", 13) == 0) { sscanf(line, "mad scientist %s", scientist_loc); continue; }
        if (strncmp(line, "Exit", 4) == 0) { sscanf(line, "Exit %s", exit_loc); continue; }
        parseLine(line);
    }
    fclose(f);
    for (int i = 0; i < num_rooms; i++) {
        for (int j = 0; j < rooms[i].num_connections; j++) {
            int ci = getRoomIdx(rooms[i].connections[j]);
            if (ci != -1) { adj_matrix[i][ci] = true; adj_matrix[ci][i] = true; }
        }
    }
    scientist_idx = getRoomIdx(scientist_loc);
    exit_idx = getRoomIdx(exit_loc);
    maze_loaded = true;
    return true;
}

void maze_addFrontier(MazeFrontier* f, int node, int h) {
    int i;
    for (i = f->count - 1; i >= 0; i--) {
        if (f->heuristics[i] <= h) break;
        f->nodes[i + 1] = f->nodes[i];
        f->heuristics[i + 1] = f->heuristics[i];
    }
    f->nodes[i + 1] = node;
    f->heuristics[i + 1] = h;
    f->count++;
}

// checks if node is already inside the frontier queue to prevent path overwrites
bool maze_inFrontier(MazeFrontier* f, int node) {
    for (int i = 0; i < f->count; i++) {
        if (f->nodes[i] == node) return true;
    }
    return false;
}

void maze_printPath(int* parent, int start, int end) {
    if (end == start) { printf("%s", rooms[end].name); return; }
    maze_printPath(parent, start, parent[end]);
    printf(" -> %s", rooms[end].name);
}

// greedy search algorithm
bool greedyBFS(int start, int goal, const char* target) {
    MazeFrontier frontier;
    frontier.count = 0;
    bool visited[MAX_ROOMS];
    int parent[MAX_ROOMS];
    int steps = 0;
    memset(visited, false, sizeof(visited));
    for (int i = 0; i < MAX_ROOMS; i++) parent[i] = -1;

    maze_addFrontier(&frontier, start, getManhattan(&rooms[start], &rooms[goal]));
    printf("  Searching for %s (room %s)...\n", target, rooms[goal].name);

    while (frontier.count > 0) {
        int cur = frontier.nodes[0];
        for (int i = 0; i < frontier.count - 1; i++) {
            frontier.nodes[i] = frontier.nodes[i + 1];
            frontier.heuristics[i] = frontier.heuristics[i + 1];
        }
        frontier.count--;

        if (visited[cur]) continue;
        visited[cur] = true;
        steps++;

        printf("  Expanding: %s (%d,%d)  h=%d\n", rooms[cur].name, rooms[cur].row, rooms[cur].col,
               getManhattan(&rooms[cur], &rooms[goal]));

        if (cur == goal) {
            printf("  Path found: ");
            maze_printPath(parent, start, goal);
            printf("\n  Rooms expanded: %d\n", steps);
            return true;
        }

        for (int i = 0; i < num_rooms; i++) {
            if (adj_matrix[cur][i] && !visited[i] && !maze_inFrontier(&frontier, i)) {
                parent[i] = cur;
                maze_addFrontier(&frontier, i, getManhattan(&rooms[i], &rooms[goal]));
            }
        }
    }
    printf("  No path found!\n");
    return false;
}

bool challenge2_solve() {
    printf("\n  ======= Challenge 2: Labyrinth Search (Greedy Best-First) =======\n");
    if (!loadMaze("maze.txt")) return false;

    int start = getRoomIdx("A2");
    printf("  --- Finding the Mad Scientist ---\n");
    if (!greedyBFS(start, scientist_idx, "Mad Scientist")) return false;
    printf("\n  --- Finding the Exit ---\n");
    if (!greedyBFS(scientist_idx, exit_idx, "Exit")) return false;

    printf("  ======= Challenge 2 Completed! =======\n\n");
    return true;
}

// =====================================================================
//  TASK 1:  Main Map - BFS & UCS
// =====================================================================

typedef struct {
    int node;
    int cost;
    int parent;
} MapNode;

typedef struct {
    MapNode nodes[50];
    int count;
} MapFrontier;

void map_initFrontier(MapFrontier* f) { f->count = 0; }

const char* nodeName(int n) {
    switch(n) {
        case NODE_S:  return "S";
        case NODE_F:  return "F";
        case NODE_C1: return "C1";
        case NODE_C2: return "C2";
        case NODE_E:  return "E";
        default: return "?";
    }
}

void map_printPath(int* parent, int node) {
    if (parent[node] == -1) { printf("%s", nodeName(node)); return; }
    map_printPath(parent, parent[node]);
    printf(" -> %s", nodeName(node));
}

int map_getPathCost(int* parent, int node) {
    if (parent[node] == -1) return 0;
    return map_getPathCost(parent, parent[node]) + graph[parent[node]][node];
}

// Store results for answers
char bfs_path_str[100], ucs_path_str[100];
int bfs_cost_val, ucs_cost_val;
int bfs_expansions, ucs_expansions;

void buildPathStr(int* parent, int node, char* buf) {
    if (parent[node] == -1) { strcpy(buf, nodeName(node)); return; }
    buildPathStr(parent, parent[node], buf);
    strcat(buf, " -> ");
    strcat(buf, nodeName(node));
}

// bfs implementation
// note: BFS doesn't check path weights, it assumes hop cost is always 1
void run_bfs(int start) {
    MapFrontier frontier;
    map_initFrontier(&frontier);
    bool reached[NUM_NODES] = {false};
    int parent[NUM_NODES];
    for (int i = 0; i < NUM_NODES; i++) parent[i] = -1;
    int expansions = 0;

    frontier.nodes[frontier.count++] = (MapNode){start, 0, -1};
    reached[start] = true;

    while (frontier.count > 0) {
        // Dequeue (FIFO)
        MapNode cur = frontier.nodes[0];
        for (int i = 0; i < frontier.count - 1; i++)
            frontier.nodes[i] = frontier.nodes[i + 1];
        frontier.count--;
        expansions++;

        // Print expansion
        if (cur.node == NODE_C1) {
            printf("Expanding %s.. Moving to challenge 1\n", nodeName(cur.node));
            if (!challenge1_solve()) {
                printf("Lara did not survive the challenge! Game Over.\n");
                return;
            }
        } else if (cur.node == NODE_C2) {
            printf("Expanding %s.. Moving to challenge 2\n", nodeName(cur.node));
            if (!challenge2_solve()) {
                printf("Lara did not survive the challenge! Game Over.\n");
                return;
            }
        } else {
            printf("Expanding %s\n", nodeName(cur.node));
        }

        // Goal test
        if (cur.node == NODE_E) {
            printf("Solution path: ");
            map_printPath(parent, cur.node);
            int cost = map_getPathCost(parent, cur.node);
            printf("\nPath cost: %d\n", cost);
            bfs_cost_val = cost;
            bfs_expansions = expansions;
            buildPathStr(parent, cur.node, bfs_path_str);
            return;
        }

        // Expand neighbors
        for (int i = 0; i < NUM_NODES; i++) {
            if (graph[cur.node][i] > 0 && !reached[i]) {
                reached[i] = true;
                parent[i] = cur.node;
                frontier.nodes[frontier.count++] = (MapNode){i, 0, cur.node};
            }
        }
    }
}

// ucs implementation with priority queue
void run_ucs(int start) {
    MapFrontier frontier;
    map_initFrontier(&frontier);
    bool visited[NUM_NODES] = {false};
    int parent[NUM_NODES];
    int dist[NUM_NODES];
    for (int i = 0; i < NUM_NODES; i++) { parent[i] = -1; dist[i] = INF; }
    dist[start] = 0;
    int expansions = 0;

    // Insert start (sorted by cost)
    frontier.nodes[frontier.count++] = (MapNode){start, 0, -1};

    while (frontier.count > 0) {
        // Dequeue min cost
        int minIdx = 0;
        for (int i = 1; i < frontier.count; i++)
            if (frontier.nodes[i].cost < frontier.nodes[minIdx].cost) minIdx = i;
        MapNode cur = frontier.nodes[minIdx];
        for (int i = minIdx; i < frontier.count - 1; i++)
            frontier.nodes[i] = frontier.nodes[i + 1];
        frontier.count--;

        if (visited[cur.node]) continue;
        visited[cur.node] = true;
        expansions++;

        // Print expansion
        if (cur.node == NODE_C1) {
            printf("Expanding %s.. Moving to challenge 1\n", nodeName(cur.node));
            if (!challenge1_solve()) {
                printf("Lara did not survive the challenge! Game Over.\n");
                return;
            }
        } else if (cur.node == NODE_C2) {
            printf("Expanding %s.. Moving to challenge 2\n", nodeName(cur.node));
            if (!challenge2_solve()) {
                printf("Lara did not survive the challenge! Game Over.\n");
                return;
            }
        } else {
            printf("Expanding %s\n", nodeName(cur.node));
        }

        // Goal test
        if (cur.node == NODE_E) {
            printf("Solution path: ");
            map_printPath(parent, cur.node);
            int cost = map_getPathCost(parent, cur.node);
            printf("\nTotal cost: %d\n", cost);
            ucs_cost_val = cost;
            ucs_expansions = expansions;
            buildPathStr(parent, cur.node, ucs_path_str);
            return;
        }

        // Expand neighbors
        for (int i = 0; i < NUM_NODES; i++) {
            if (graph[cur.node][i] > 0) {
                int nd = dist[cur.node] + graph[cur.node][i];
                if (nd < dist[i]) {
                    dist[i] = nd;
                    parent[i] = cur.node;
                    frontier.nodes[frontier.count++] = (MapNode){i, nd, cur.node};
                }
            }
        }
    }
}

// printing answers
void printAllAnswers() {
    printf("\n============================================================\n");
    printf("                       *** Answers ***\n");
    printf("============================================================\n");

    // Task 1 Questions
    printf("\n--- Task 1 Questions ---\n\n");

    printf("Q1: Which path passes through the safe house (F)?\n");
    printf("A1: The UCS path passes through the safe house: %s (cost %d).\n",
           ucs_path_str, ucs_cost_val);
    printf("    The BFS path does not pass through F: %s (cost %d).\n\n",
           bfs_path_str, bfs_cost_val);

    printf("Q2: Which algorithm (BFS or UCS) reaches the goal with fewer expansions?\n");
    printf("A2: BFS expanded %d nodes and UCS expanded %d nodes.\n", bfs_expansions, ucs_expansions);
    printf("    Both algorithms expand the same number of nodes in this graph.\n");
    printf("    However, UCS finds the cost-optimal path while BFS finds the\n");
    printf("    shallowest (fewest edges) path.\n\n");

    printf("Q3: Does passing through the safe house always lead to an optimal solution?\n");
    printf("A3: Not always. Whether passing through the safe house leads to an optimal\n");
    printf("    solution depends on the edge weights. In this specific graph, the path\n");
    printf("    through F (S->F->C2->E, cost %d) IS the optimal path found by UCS.\n", ucs_cost_val);
    printf("    However, in general, a safe house detour could increase the total cost\n");
    printf("    if the edge weights were different.\n\n");

    printf("Q4: Compare the solution paths returned by BFS and UCS.\n");
    printf("A4: BFS path: %s (cost %d, %d expansions)\n", bfs_path_str, bfs_cost_val, bfs_expansions);
    printf("    UCS path: %s (cost %d, %d expansions)\n", ucs_path_str, ucs_cost_val, ucs_expansions);
    printf("    BFS finds the path with the fewest edges (hops) since it ignores\n");
    printf("    edge weights. UCS finds the path with the lowest total cost.\n");
    printf("    In this case, UCS finds the optimal path through the safe house.\n\n");

    // Challenge 1 Questions
    printf("--- Challenge 1 Questions (Guards and Prisoners) ---\n\n");

    printf("Q1: What makes a state invalid in this problem?\n");
    printf("A1: A state is invalid when prisoners outnumber guards on either bank\n");
    printf("    (when guards are present on that bank). For state (G,P,B): if G > 0\n");
    printf("    and P > G on the left bank, or if (3-G) > 0 and (3-P) > (3-G) on\n");
    printf("    the right bank. Also invalid if G or P is outside [0,3].\n\n");

    printf("Q2: Why must the heuristic be admissible?\n");
    printf("A2: The heuristic must be admissible (never overestimate) to guarantee\n");
    printf("    A* finds the optimal solution. Our heuristic h(n) = ceil((G+P)/2)\n");
    printf("    is admissible because the boat carries max 2 people, so at least\n");
    printf("    ceil((G+P)/2) one-way trips are needed to transport everyone.\n\n");

    printf("Q3: What happens if the heuristic overestimates?\n");
    printf("A3: If the heuristic overestimates, A* loses its optimality guarantee.\n");
    printf("    It may return a suboptimal (longer) solution because it would\n");
    printf("    deprioritize nodes that are actually closer to the goal, expanding\n");
    printf("    states that seem better but are not.\n\n");

    // Challenge 2 Questions
    printf("--- Challenge 2 Questions (Labyrinth Search) ---\n\n");

    printf("Q1: Is Greedy Best-First Search guaranteed to find the optimal path?\n");
    printf("A1: No. Greedy Best-First Search only considers h(n), not g(n). It may\n");
    printf("    find a path quickly but it is not guaranteed to be the shortest or\n");
    printf("    least-cost path.\n\n");

    printf("Q2: How does the heuristic affect node expansion?\n");
    printf("A2: The heuristic determines expansion order. Nodes with smaller h(n)\n");
    printf("    (closer to goal) are expanded first. Manhattan distance guides\n");
    printf("    the search toward the goal efficiently, reducing unnecessary\n");
    printf("    expansions compared to uninformed search.\n\n");

    printf("Q3: What happens if nodes are expanded more than once?\n");
    printf("A3: If nodes are expanded more than once (Tree Search), the algorithm\n");
    printf("    may loop infinitely in cyclic graphs and waste resources. Graph\n");
    printf("    Search prevents this by tracking visited nodes in a closed set,\n");
    printf("    ensuring each node is expanded at most once.\n");
}

// main function
int main(int argc, char* argv[]) {
    init_exe_dir(argc, argv);
    printf("Welcome to Agent Lara's Rescue Mission\n\n");

    printf("Choose the Algorithm to Apply:\n");
    printf("1) UCS\n");
    printf("2) BFS\n");
    printf("Choice: ");

    int choice;
    scanf("%d", &choice);

    memset(bfs_path_str, 0, sizeof(bfs_path_str));
    memset(ucs_path_str, 0, sizeof(ucs_path_str));

    if (choice == 1) {
        printf("\n--- Applying UCS ---\n");
        run_ucs(NODE_S);
        printf("\n*** Now the second algorithm will be applied ***\n\n");
        printf("--- Applying BFS ---\n");
        maze_loaded = false;  // Reset maze for second run
        run_bfs(NODE_S);
    } else if (choice == 2) {
        printf("\n--- Applying BFS ---\n");
        run_bfs(NODE_S);
        printf("\n*** Now the second algorithm will be applied ***\n\n");
        printf("--- Applying UCS ---\n");
        maze_loaded = false;  // Reset maze for second run
        run_ucs(NODE_S);
    } else {
        printf("Invalid choice!\n");
        return 1;
    }

    printAllAnswers();

    printf("\n============================================================\n");
    printf("              Lara has escaped the island!\n");
    printf("============================================================\n");

    // ASSIGNMENT 2 INTEGRATION
    printf("\n*** LARA MUST NOW PLAY THE POISONOUS CHOCOLATE BAR GAME ***\n");
    // We try running the script from the current directory, or parent directories if not found
    system("python3 poisonous_chocolate_game.py --simulation || python3 ../poisonous_chocolate_game.py --simulation || python3 ../../poisonous_chocolate_game.py --simulation");

    return 0;
}