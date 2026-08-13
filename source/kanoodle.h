
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <string.h>
#include "polyomino.h"
#include "dlx.h"

/**
 * For the game of Kanoodle, we have 12 pieces "noodles"
 * that reside on a 11x5 board. In order to find solutions
 * for Kanoodle, we may define the set of all possible
 * placements of all pieces in all orientations,
 * and run this well-defined move-space through an
 * exact-cover algorithm such as Dancing Links (DLX).
 *
 * In order to fit the move-space of Kanoodle into DLX-ready
 * matrix rows, we can define each move as one matrix row
 * consisting of 55 columns (deconstruction of the 11x5 grid)
 * plus 12 additional columns to denote which piece is
 * actively used by this move/row. From here, we can
 * simply churn our rows through DLX to find all solutions
 * for the puzzle Kanoodle.
 *
 * For purposes of human gameplay, we arrange DLX to become
 * a random solution generator. In pursuit of efficiency,
 * one approach to achieve this is to provide a randomized
 * subset of k moves from the absolute move-space to DLX 
 * upon each run. This adds randomness to which solution is
 * found first, as well as lowering runtime and memory cost.
 */

#define NOODLE_A (Polyomino){ .id = 'A', .size = 4, .cell = {{0,0},{1,0},{2,0},{0,1}} }
#define NOODLE_B (Polyomino){ .id = 'B', .size = 5, .cell = {{0,0},{1,0},{2,0},{0,1},{1,1}} }
#define NOODLE_C (Polyomino){ .id = 'C', .size = 5, .cell = {{0,0},{1,0},{2,0},{3,0},{0,1}} }
#define NOODLE_D (Polyomino){ .id = 'D', .size = 5, .cell = {{0,0},{1,0},{2,0},{3,0},{1,1}} }
#define NOODLE_E (Polyomino){ .id = 'E', .size = 5, .cell = {{0,0},{1,0},{2,0},{2,1},{3,1}} }
#define NOODLE_F (Polyomino){ .id = 'F', .size = 3, .cell = {{0,0},{1,0},{0,1}} }
#define NOODLE_G (Polyomino){ .id = 'G', .size = 5, .cell = {{0,0},{1,0},{2,0},{0,1},{0,2}} }
#define NOODLE_H (Polyomino){ .id = 'H', .size = 5, .cell = {{0,0},{1,0},{1,1},{2,1},{2,2}} }
#define NOODLE_I (Polyomino){ .id = 'I', .size = 5, .cell = {{0,0},{1,0},{2,0},{0,1},{2,1}} }
#define NOODLE_J (Polyomino){ .id = 'J', .size = 4, .cell = {{0,0},{1,0},{2,0},{3,0}} }
#define NOODLE_K (Polyomino){ .id = 'K', .size = 4, .cell = {{0,0},{1,0},{0,1},{1,1}} }
#define NOODLE_L (Polyomino){ .id = 'L', .size = 5, .cell = {{1,0},{0,1},{1,1},{2,1},{1,2}} }

static const Polyomino NOODLES[] = {NOODLE_A, NOODLE_B, NOODLE_C, NOODLE_D, NOODLE_E, NOODLE_F, NOODLE_G, NOODLE_H, NOODLE_I, NOODLE_J, NOODLE_K, NOODLE_L};
#define NOODLE_COUNT sizeof(NOODLES) / sizeof(Polyomino)
#define MAX_CELLS MAX_POLY_CELLS // mirror of polyomino.h (must be >= 5 to support kanoodle pieces)

#define BOARD_WIDTH 11
#define BOARD_HEIGHT 5
#define MAX_MOVES MAX_ROWS // mirror of dlx.h (must be <= cardinality of move space which is ~1800 in this puzzle arrangement)

// Get random sample of Kanoodle's entire move space
size_t sample_move_space(Polyomino sample[], int k)
{
    srand(time(NULL));
    size_t n = 0; // sampling counter

    // for each noodle
    for (int ndl = 0; ndl < NOODLE_COUNT; ndl++)
    {
        // gen orientations
        Polyomino orients[8]; // upto 8 possible ways
        int n_orients = get_orientations(NOODLES[ndl], orients);

        // for each orientation
        for (int ori = 0; ori < n_orients; ori++)
        {
            // gen placements
            Polyomino orient = orients[ori];
            int x0, y0, x1, y1;
            bounds(orient, &x0, &y0, &x1, &y1);
            int x_range = BOARD_WIDTH - (x1 - x0 + 1) + 1; // ways to shift piece horizontally
            int y_range = BOARD_HEIGHT - (y1 - y0 + 1) + 1;  // ways to shift piece vertically

            // for each placement
            for (int dy = 0; dy < y_range; dy++)
            {
                for (int dx = 0; dx < x_range; dx++)
                {
                    // Reservoir Sampling (Algorithm R)
                    // https://en.wikipedia.org/wiki/Reservoir_sampling

                    n++;
                    if (n <= k)
                    {
                        // Fill the reservoir for the first 'k' elements
                        sample[n - 1] = shifted(orient, dx, dy);
                    }
                    else
                    {
                        // Random index from 0 to n - 1 inclusive
                        size_t j = (size_t)rand() % n;

                        // If j falls within [0, k-1], replace that element
                        if (j < k)
                        {
                            sample[j] = shifted(orient, dx, dy);
                        }
                    }
                }
            }
        }
    }
    return n; // return population size
}

void print_solution(const Polyomino solution[12]) {

    static const char *colors[] = {
        "\x1b[31m", "\x1b[32m", "\x1b[33m", "\x1b[34m", "\x1b[35m", "\x1b[36m", "\x1b[37m", "\x1b[90m", "\x1b[91m", "\x1b[92m", "\x1b[93m", "\x1b[94m", "\x1b[95m", "\x1b[96m", "\x1b[97m"
    };

    char board[BOARD_HEIGHT][BOARD_WIDTH];

    for (uint16_t i = 0; i < 12; i++) {
        const Polyomino p = solution[i];
        char char_id = p.id;

        for (uint8_t c = 0; c < p.size; c++) {
            int16_t x = p.cell[c].x;
            int16_t y = p.cell[c].y;
            board[y][x] = char_id;
        }
    }

    for (int y = 0; y < BOARD_HEIGHT; y++) {
        for (int x = 0; x < BOARD_WIDTH; x++) {
            char c = board[y][x];
            const char *color = colors[c-NOODLE_A.id];
            printf("%s%c ", color, c);
        }
        printf("\n");
    }
    printf("\x1b[37m");
}

static Polyomino moves[MAX_MOVES]; // statically define to not consume very limited stack space

int random_solution(Polyomino solution[12])
{
    // get random subset moves
    sample_move_space(moves, MAX_MOVES);

    // init dlx (board size + piece count)
    dlx_init(&solver, (BOARD_WIDTH*BOARD_HEIGHT)+NOODLE_COUNT);

    // load moves into dlx
    for (uint16_t i = 0; i < MAX_MOVES; i++) {
        uint16_t dlx_cols[1+MAX_CELLS]; // 1 piece id bit + cell cols
        uint16_t col_cnt = 0;

        // load piece-identifying node (1-indexed bc 0 is root)
        dlx_cols[col_cnt++] = 1 + (moves[i].id - NOODLES[0].id); 

        // load cell-identifying nodes
        for (uint8_t c = 0; c < moves[i].size; c++) {
            int16_t x = moves[i].cell[c].x;
            int16_t y = moves[i].cell[c].y;

            uint16_t cell_index = (y * BOARD_WIDTH) + x;
            uint16_t cell_col = 1 + NOODLE_COUNT + cell_index; // board indices start after root & piece cols
            
            dlx_cols[col_cnt++] = cell_col; 
        }

        // Add move row to matrix
        dlx_add_row(&solver, i, dlx_cols, col_cnt);
    }

    // solve dlx and return
    if (dlx_search(&solver)) {
        for (uint16_t i = 0; i < solver.solution_size; i++) {
            uint16_t move_idx = solver.solution[i];
            solution[i] = moves[move_idx];
        }
        return solver.solution_size; // solution found
    }
    return 0; // no solution
}

