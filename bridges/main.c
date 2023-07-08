#include <stdio.h>
#include <assert.h>
#include <string.h>

#define NROWS 15
#define NCOLS 15

#define MAX_ITERATIONS 100

enum { LEFT, UP, DOWN, RIGHT, };

enum {
    NONE,
    LS = 1,
    LD = 2,
    US = 4,
    UD = 8,
    DS = 16,
    DD = 32,
    RS = 64,
    RD = 128,
};

typedef struct {
    int row;
    int col;
} Location;

#define EXAMPLE1_7x7 { \
    {4, 0, 0, 3, 0, 0, 2}, \
    {0, 0, 0, 0, 0, 0, 0}, \
    {0, 1, 0, 5, 0, 0, 5}, \
    {0, 0, 0, 0, 0, 1, 0}, \
    {3, 0, 1, 0, 0, 0, 1}, \
    {0, 0, 0, 0, 0, 0, 0}, \
    {2, 0, 0, 5, 0, 3, 0}, \
}

#define EXAMPLE2_7x7 { \
    {4, 0, 0, 4, 0, 2, 0}, \
    {0, 0, 0, 0, 0, 0, 2}, \
    {0, 0, 0, 0, 0, 1, 0}, \
    {6, 0, 0, 6, 0, 0, 5}, \
    {0, 0, 0, 0, 0, 0, 0}, \
    {3, 0, 0, 5, 0, 2, 0}, \
    {0, 0, 0, 0, 0, 0, 2}, \
}

#define EXAMPLE3_7x7 { \
    {1, 0, 0, 0, 4, 0, 3}, \
    {0, 0, 0, 0, 0, 0, 0}, \
    {2, 0, 3, 0, 0, 0, 0}, \
    {0, 0, 0, 0, 0, 0, 0}, \
    {0, 0, 0, 0, 0, 0, 0}, \
    {0, 0, 4, 0, 3, 0, 0}, \
    {1, 0, 0, 0, 0, 0, 1}, \
}

#define EXAMPLE1_15x15 { \
    {4, 0, 0, 0, 0, 2, 0, 1, 0, 0, 0, 0, 0, 0, 3}, \
    {0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0}, \
    {0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0}, \
    {3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, \
    {0, 2, 0, 2, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0}, \
    {4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 5}, \
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, \
    {0, 2, 0, 0, 0, 5, 0, 0, 5, 0, 3, 0, 0, 0, 0}, \
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, \
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, \
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 2, 0, 0}, \
    {3, 0, 0, 0, 0, 2, 0, 0, 3, 0, 0, 0, 0, 0, 5}, \
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, \
    {2, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 2, 0}, \
    {0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3}, \
}

static int puzzle[NROWS][NCOLS] = EXAMPLE1_15x15;

static int solution[NROWS][NCOLS] = {0};
static int prev_iteration[NROWS][NCOLS];

static int bridges[NROWS][NCOLS] = {0};

static int example_solution[NROWS][NCOLS] = {
    { RD | DD, 0,  0,  LD | DS,           0,  0,       DD           },
    { 0,       0,  0,  0,                 0,  0,       0            },
    { 0,       RS, 0,  US | LS | RD | DS, 0,  0,       UD | LD | DS },
    { 0,       0,  0,  0,                 0,  DS,      0            },
    { UD | RS, 0,  LS, 0,                 0,  0,       US           },
    { 0,       0,  0,  0,                 0,  0,       0            },
    { RD,      0,  0,  LD | RD | US,      0,  LD | US, 0            },
};

static int
count_connections_dir(int row, int col, int dir)
{
    int n = 0;
    if (solution[row][col] & (1 << (dir*2)))     n += 1;
    if (solution[row][col] & (1 << (dir*2 + 1))) n += 2;
    return n;
}

static int
count_connections(int row, int col)
{
    int n = 0;
    for (int dir = LEFT; dir <= RIGHT; dir++) {
        n += count_connections_dir(row, col, dir);
    }
    return n;
}

// TODO: check that bridges do not cross
// TODO: check that connections are consistent
static int
check_solution(void)
{
    for (int r = 0; r < NROWS; r++) {
        for (int c = 0; c < NCOLS; c++) {
            if (puzzle[r][c] == 0) {
                if (solution[r][c] != NONE)
                    return 0;
            } else  {
                if (puzzle[r][c] != count_connections(r, c))
                    return 0;
            }
        }
    }
    return 1;
}

static Location
find_adjacent(int row, int col, int dir)
{
    Location loc = { .row = row, .col = col };
    int x;
    if (dir == LEFT) {
        for (x = col-1; x >= 0 && puzzle[row][x] == NONE; x--) {
            if (bridges[row][x]) {
                x = -1;
                break;
            }
        }
        loc.col = x;
    } else if (dir == RIGHT) {
        for (x = col+1; x < NCOLS && puzzle[row][x] == NONE; x++) {
            if (bridges[row][x]) {
                x = -1;
                break;
            }
        }
        if (x == NCOLS)
            x = -1;
        loc.col = x;
    } else if (dir == UP) {
        for (x = row-1; x >= 0 && puzzle[x][col] == NONE; x--) {
            if (bridges[x][col]) {
                x = -1;
                break;
            }
        }
        loc.row = x;
    } else if (dir == DOWN) {
        for (x = row+1; x < NROWS && puzzle[x][col] == NONE; x++) {
            if (bridges[x][col]) {
                x = -1;
                break;
            }
        }
        if (x == NROWS)
            x = -1;
        loc.row = x;
    }
    return loc;
}

static int
blocked_by_bridge(Location l1, Location l2)
{
    enum { VERTICAL, HORIZONTAL };
    int dir = VERTICAL;
    if (l1.row == l2.row) {
        dir = HORIZONTAL;
    } else if (l1.row > l2.row || l1.col > l2.col) {
        Location tmp = l1;
        l1 = l2;
        l2 = tmp;
    }
    if (dir == HORIZONTAL) {
        int r = l1.row;
        for (int c = l1.col+1; c < l2.col; c++) {
            if (bridges[r][c])
                return 1;
        }
        return 0;
    } else {
        int c = l1.col;
        for (int r = l1.row+1; r < l2.row; r++) {
            if (bridges[r][c])
                return 1;
        }
        return 0;
    }
    assert(0);
    return 0;
}

static void
connect(int row, int col, int dir, int n)
{
    //printf("connect %d %d %d %d\n", row, col, dir, n);
    Location adj = find_adjacent(row, col, dir);
    assert(adj.row >= 0 && adj.col >= 0);
    solution[row][col] |= 1 << (dir*2 + n-1);
    int opposite_dir = 3-dir;
    solution[adj.row][adj.col] |= 1 << (opposite_dir*2 + n-1);

    int x;
    if (dir == LEFT) {
        for (x = col-1; x >= 0 && puzzle[row][x] == NONE; x--)
            bridges[row][x] = 1;
    } else if (dir == RIGHT) {
        for (x = col+1; x < NCOLS && puzzle[row][x] == NONE; x++)
            bridges[row][x] = 1;
    } else if (dir == UP) {
        for (x = row-1; x >= 0 && puzzle[x][col] == NONE; x--)
            bridges[x][col] = 1;
    } else if (dir == DOWN) {
        for (x = row+1; x < NROWS && puzzle[x][col] == NONE; x++)
            bridges[x][col] = 1;
    }
}

static void
solve(void)
{
    //memcpy(solution, example_solution, sizeof(solution));

    int niterations = 0;
    while (niterations < MAX_ITERATIONS) {
        niterations++;
        //printf("iteration %d\n", niterations);
        memcpy(prev_iteration, solution, sizeof(solution));
        for (int r = 0; r < NROWS; r++) {
            for (int c = 0; c < NCOLS; c++) {
                Location loc = { .row = r, .col = c };
                if (puzzle[r][c] != 0) {
                    int nremain = puzzle[r][c] - count_connections(r, c);
                    int adj_total = 0;
                    int adj_nremain[4] = {0};
                    if (nremain == 0)
                        continue;
                    for (int dir = LEFT; dir <= RIGHT; dir++) {
                        Location adj = find_adjacent(r, c, dir);
                        if (blocked_by_bridge(loc, adj))
                            continue;
                        if (adj.row == -1 || adj.col == -1)
                            continue;
                        if (count_connections_dir(r, c, dir) > 0)
                            continue;
                        adj_nremain[dir] = puzzle[adj.row][adj.col] - count_connections(adj.row, adj.col);
                        if (adj_nremain[dir] > 2)
                            adj_nremain[dir] = 2;
                        adj_total += adj_nremain[dir];
                    }
                    if (adj_total == nremain) {
                        for (int dir = LEFT; dir <= RIGHT; dir++) {
                            if (adj_nremain[dir] > 0)
                                connect(r, c, dir, adj_nremain[dir]);
                        }
                    } else {
                        int nzero = 0;
                        for (int dir = LEFT; dir <= RIGHT; dir++) {
                            if (adj_nremain[dir] == 0)
                                nzero++;
                        }
                        if (nzero == 3) {
                            for (int dir = LEFT; dir <= RIGHT; dir++) {
                                if (adj_nremain[dir] > 0)
                                    connect(r, c, dir, nremain);
                            }
                        }
                    }
                }
            }
        }
        if (memcmp(prev_iteration, solution, sizeof(solution)) == 0) {
            printf("stopped after %d iterations\n", niterations);
            break;
        }
    }

    if (niterations >= MAX_ITERATIONS) {
        printf("failed to find solution after %d iterations\n", MAX_ITERATIONS);
    }
}

static void
print_solution(void)
{
    int next_row[NCOLS] = {0};
    for (int r = 0; r < NROWS; r++) {
        int curr_row = 0;
        for (int c = 0; c < NCOLS; c++) {
            if (puzzle[r][c] == 0) {
                if (next_row[c] & DS)
                    printf("\u2502");
                else if (next_row[c] & DD)
                    printf("\u2551");
                else if (curr_row & RS)
                    printf("\u2500");
                else if (curr_row & RD)
                    printf("\u2550");
                else
                    printf(" ");
            } else {
                printf("%d", puzzle[r][c]);
                curr_row = solution[r][c];
                next_row[c] = solution[r][c];
            }
        }
        printf("\n");
    }
}

int main()
{
    solve();
    print_solution();
#if 0
    for (int r = 0; r < NROWS; r++) {
        for (int c = 0; c < NCOLS; c++) {
            printf("%d ", count_connections(r, c));
        }
        printf("\n");
    }
#endif
    assert(check_solution());
    return 0;
}
