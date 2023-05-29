#define _POSIX_C_SOURCE 199309L
#include "timer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>

#define NELEM(X) (sizeof(X)/sizeof(X[0]))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#ifndef MAX_DOMINO
#define MAX_DOMINO      7
#endif
#define NDOMINOES       ((MAX_DOMINO+1)*MAX_DOMINO/2)
#define BOARD_WIDTH     (MAX_DOMINO+1)
#define BOARD_HEIGHT    MAX_DOMINO

enum { DIR_NONE, DIR_LEFT, DIR_UP, DIR_DOWN, DIR_RIGHT };

typedef struct {
    int numbers[BOARD_HEIGHT][BOARD_WIDTH];
    int tiles[BOARD_HEIGHT][BOARD_WIDTH];
} Board;

Board board;

typedef struct {
    int row;
    int col;
} Location;

typedef struct {
    int a;
    int b;
} Domino;

#if BOARD_WIDTH == 5
static Board example_5x4_board = {
    .numbers = {
        { 2, 2, 1, 3, 0 },
        { 1, 0, 2, 3, 1 },
        { 3, 0, 3, 0, 2 },
        { 3, 2, 0, 1, 1 },
    },
};

#define example_board example_5x4_board
#endif

#if BOARD_WIDTH == 6
static Board example_6x5_board = {
    .numbers = {
        { 2, 3, 0, 0, 1, 1 },
        { 0, 1, 2, 1, 2, 3 },
        { 0, 0, 3, 4, 0, 4 },
        { 2, 3, 2, 4, 1, 4 },
        { 4, 1, 2, 4, 3, 3 },
    },
};

#define example_board example_6x5_board
#endif

#if BOARD_WIDTH == 8
static Board example1_8x7_board = {
    .numbers = {
        { 2, 2, 6, 1, 4, 0, 2, 5 },
        { 2, 3, 3, 6, 4, 0, 1, 6 },
        { 5, 0, 0, 6, 3, 4, 1, 1 },
        { 1, 4, 3, 5, 4, 5, 0, 1 },
        { 3, 0, 2, 5, 3, 5, 2, 4 },
        { 2, 5, 1, 4, 4, 3, 0, 1 },
        { 6, 3, 6, 6, 5, 6, 0, 2 },
    },
};

static Board example2_8x7_board = {
    .numbers = {
        { 0, 3, 4, 1, 4, 3, 2, 0 },
        { 3, 1, 0, 1, 2, 4, 5, 5 },
        { 3, 5, 4, 1, 6, 2, 4, 5 },
        { 1, 6, 4, 3, 2, 5, 5, 3 },
        { 2, 6, 4, 5, 3, 2, 6, 4 },
        { 2, 0, 0, 1, 1, 0, 0, 6 },
        { 0, 5, 6, 3, 6, 6, 1, 2 },
    },
};

#define example_board example_8x7_board
#endif

#if BOARD_WIDTH == 11
static Board example_11x10_board = {
    .numbers = {
        { 2, 7, 7, 4, 1, 9, 7, 3, 1, 1, 7 },
        { 3, 1, 8, 2, 4, 8, 8, 9, 3, 8, 0 },
        { 8, 0, 6, 3, 4, 0, 5, 3, 6, 3, 3 },
        { 0, 7, 7, 4, 2, 8, 5, 4, 4, 0, 4 },
        { 2, 7, 5, 1, 1, 0, 5, 9, 6, 2, 4 },
        { 0, 6, 3, 3, 5, 6, 9, 1, 3, 8, 8 },
        { 6, 6, 1, 0, 1, 2, 2, 5, 5, 5, 9 },
        { 9, 0, 9, 7, 7, 6, 3, 0, 0, 2, 4 },
        { 8, 8, 1, 8, 2, 6, 5, 9, 9, 1, 9 },
        { 2, 9, 5, 2, 6, 6, 5, 7, 4, 4, 7 },
    },
};

#define example_board example_11x10_board
#endif

static void
swap(void * a, void * b, size_t size)
{
    char * a_cp = a, * b_cp = b;
    for (size_t i = 0; i < size; i++) {
        char t = a_cp[i];
        a_cp[i] = b_cp[i];
        b_cp[i] = t;
    }
}

static void
fisher_yates_shuffle(void * arr, size_t nmemb, size_t size)
{
    char * arr_cp = arr;
    for (size_t i = 0; i < nmemb; i++) {
        int r = (rand() % (nmemb-i))+i;
        swap(arr_cp + i*size, arr_cp + r*size, size);
    }
}

#define fisher_yates_shuffle_arr(arr) fisher_yates_shuffle(arr, NELEM(arr), sizeof(arr[0]))

static Domino
get_domino(Location loc)
{
    Domino domino;
    int a = board.numbers[loc.row][loc.col];
    int b;
    switch (board.tiles[loc.row][loc.col]) {
        case DIR_LEFT:  b = board.numbers[loc.row][loc.col-1]; break;
        case DIR_RIGHT: b = board.numbers[loc.row][loc.col+1]; break;
        case DIR_UP:    b = board.numbers[loc.row-1][loc.col]; break;
        case DIR_DOWN:  b = board.numbers[loc.row+1][loc.col]; break;
        default: assert(0);
    }
    domino.a = MIN(a, b);
    domino.b = MAX(a, b);
    return domino;
}

static int
can_place_domino(int dir, Location loc)
{
    // check for edge of board
    if ((loc.row == 0                 && dir == DIR_UP    ) ||
        (loc.col == 0                 && dir == DIR_LEFT  ) ||
        (loc.row == BOARD_HEIGHT-1    && dir == DIR_DOWN  ) ||
        (loc.col == BOARD_WIDTH-1     && dir == DIR_RIGHT )) {
        return 0;
    }
    // check for neighboring dominoes
    if ((dir == DIR_LEFT    && board.tiles[loc.row][loc.col-1] != DIR_NONE) ||
        (dir == DIR_RIGHT   && board.tiles[loc.row][loc.col+1] != DIR_NONE) ||
        (dir == DIR_UP      && board.tiles[loc.row-1][loc.col] != DIR_NONE) ||
        (dir == DIR_DOWN    && board.tiles[loc.row+1][loc.col] != DIR_NONE)) {
        return 0;
    }
    return 1;
}

static void
place_domino(int dir, Location loc)
{
    board.tiles[loc.row][loc.col] = dir;
    switch (dir) {
        case DIR_LEFT:  board.tiles[loc.row][loc.col-1] = 5 - dir; break;
        case DIR_RIGHT: board.tiles[loc.row][loc.col+1] = 5 - dir; break;
        case DIR_UP:    board.tiles[loc.row-1][loc.col] = 5 - dir; break;
        case DIR_DOWN:  board.tiles[loc.row+1][loc.col] = 5 - dir; break;
        default: assert(0);
    }
}

static void
remove_domino(Location loc)
{
    int dir = board.tiles[loc.row][loc.col];
    board.tiles[loc.row][loc.col] = DIR_NONE;
    switch (dir) {
        case DIR_LEFT:  board.tiles[loc.row][loc.col-1] = DIR_NONE; break;
        case DIR_RIGHT: board.tiles[loc.row][loc.col+1] = DIR_NONE; break;
        case DIR_UP:    board.tiles[loc.row-1][loc.col] = DIR_NONE; break;
        case DIR_DOWN:  board.tiles[loc.row+1][loc.col] = DIR_NONE; break;
        default: assert(0);
    }
}

static void
print_board()
{
    //printf("\033c");
    for (int r = 0; r < BOARD_HEIGHT; r++) {
        for (int c = 0; c < BOARD_WIDTH; c++) {
            printf("%2d%c", board.numbers[r][c], (board.tiles[r][c] == DIR_RIGHT) ? '-' : ' ');
        }
        printf("\n");
        for (int c = 0; c < BOARD_WIDTH; c++) {
            printf(" %c ", (board.tiles[r][c] == DIR_DOWN) ? '|' : ' ');
        }
        printf("\n");
    }
    printf("\n");
}

static Location
find_next_empty_location(Location curr_loc)
{
    Location next_loc = curr_loc;
    while (1) {
        next_loc.col++;
        if (next_loc.col == BOARD_WIDTH) {
            next_loc.col = 0;
            next_loc.row++;
            if (next_loc.row == BOARD_HEIGHT)
                break;
        }
        if (board.tiles[next_loc.row][next_loc.col] == DIR_NONE)
            break;
    }
    return next_loc;
}

static int
randomly_tile_board(Location loc)
{
    int dirs[4] = { DIR_LEFT, DIR_RIGHT, DIR_UP, DIR_DOWN };
    fisher_yates_shuffle_arr(dirs);

    int d = 0;
    while (1) {
        for (; d < 4; d++) {
            if (can_place_domino(dirs[d], loc))
                break;
        }
        if (d == 4)
            return 0; // could not place domino

        place_domino(dirs[d], loc);

        Location next_loc = find_next_empty_location(loc);
        if (next_loc.row == BOARD_HEIGHT)
            return 1; // board is full

        assert(board.tiles[next_loc.row][next_loc.col] == DIR_NONE);
        if (randomly_tile_board(next_loc))
            return 1; // tail recursion

        remove_domino(loc);
        d++;
    }
}

static void
generate_random_puzzle()
{
    board = (Board) {0};
    Location upper_left = {0, 0};
    randomly_tile_board(upper_left);

    Domino dominoes[NDOMINOES];
    // generate list of dominoes from (0,0) to (n,n)
    {
        int di = 0;
        for (int i = 0; i < MAX_DOMINO; i++) {
            for (int j = i; j < MAX_DOMINO; j++) {
                assert(di < NELEM(dominoes));
                dominoes[di++] = (Domino) { .a = i, .b = j };
            }
        }
        assert(di == NELEM(dominoes));
    }
    fisher_yates_shuffle_arr(dominoes);

    // assign the dominoes to the board
    {
        int di = 0;
        for (int r = 0; r < BOARD_HEIGHT; r++) {
            for (int c = 0; c < BOARD_WIDTH; c++) {
                int dir = board.tiles[r][c];
                if (dir == DIR_LEFT || dir == DIR_UP)
                    continue;
                board.numbers[r][c] = dominoes[di].a;
                if (dir == DIR_RIGHT) board.numbers[r][c+1] = dominoes[di].b;
                if (dir == DIR_DOWN)  board.numbers[r+1][c] = dominoes[di].b;
                di++;
            }
        }
    }
}

static void
clear_dominoes()
{
    for (int r = 0; r < BOARD_HEIGHT; r++)
        for (int c = 0; c < BOARD_WIDTH; c++)
            board.tiles[r][c] = DIR_NONE;
}

static int
domino_already_used(Domino domino, Domino dominoes[], int ndominoes)
{
    assert(domino.a <= domino.b);
    for (int i = 0; i < ndominoes; i++) {
        if (dominoes[i].a == domino.a && dominoes[i].b == domino.b)
            return 1;
    }
    return 0;
}

static long undo_count;

static int
solve_recursive(Location loc, Domino dominoes[], int ndominoes)
{
    int dir = DIR_LEFT;
    while (1) {
        for (; dir <= DIR_RIGHT; dir++) {
            if (can_place_domino(dir, loc))
                break;
        }
        if (dir > DIR_RIGHT)
            return 0; // could not place domino

        place_domino(dir, loc);

        // check if domino has already been placed
        Domino domino = get_domino(loc);
        if (domino_already_used(domino, dominoes, ndominoes)) {
            remove_domino(loc);
            dir++;
            continue; // found duplicate domino
        }

        dominoes[ndominoes++] = domino;
        assert(ndominoes <= NDOMINOES);

        Location next_loc = find_next_empty_location(loc);
        if (next_loc.row == BOARD_HEIGHT)
            return 1; // board is full

        assert(board.tiles[next_loc.row][next_loc.col] == DIR_NONE);
        if (solve_recursive(next_loc, dominoes, ndominoes))
            return 1; // tail recursion

        remove_domino(loc);
        ndominoes--;
        undo_count++;
        dir++;
    }
}

static int
solve()
{
    Location upper_left = {0, 0};
    Domino dominoes[NDOMINOES];
    int ndominoes = 0;
    undo_count = 0;
    return solve_recursive(upper_left, dominoes, ndominoes);
}

static int
check_solution()
{
    Domino dominoes[NDOMINOES];
    int di = 0;
    for (int r = 0; r < BOARD_HEIGHT; r++) {
        for (int c = 0; c < BOARD_WIDTH; c++) {
            int dir = board.tiles[r][c];
            if (dir == DIR_NONE)
                return 0; // board isn't fully tiled
            if (dir == DIR_LEFT || dir == DIR_UP)
                continue;
            Location loc = {r, c};
            Domino domino = get_domino(loc);
            for (int i = 0; i < di; i++) {
                if (dominoes[i].a == domino.a && dominoes[i].b == domino.b)
                    return 0; // found duplicate domino
            }
            dominoes[di++] = domino;
        }
    }
    return 1;
}

#ifndef NSOLVE
#define NSOLVE 1
#endif
int main()
{
    //board = example_board;
    long max_solve_time = 0;
    long total_solve_time = 0;
    Timer solve_timer;

    for (int i = 0; i < NSOLVE; i++) {
        generate_random_puzzle();
        clear_dominoes();

        timer_start(&solve_timer);
        solve();
        timer_stop(&solve_timer);

        int solved = check_solution();
        assert(solved);

        long solve_time_ns = timer_get_elapsed(&solve_timer, TIMER_MS);
        total_solve_time += solve_time_ns;
        if (solve_time_ns > max_solve_time)
            max_solve_time = solve_time_ns;
        printf("solve time: %5ldms\tundos: %ld\n", solve_time_ns, undo_count);
    }
    printf("max solve time: %5ldms\n", max_solve_time);
    printf("avg solve time: %5ldms\n", total_solve_time/NSOLVE);

    print_board();

    return 0;
}
