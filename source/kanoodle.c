
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "kanoodle.h"
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

const Polyomino NOODLES[] = {
    {.id = 'A', .size = 4, .cell = {{0, 0}, {1, 0}, {2, 0}, {0, 1}}},
    {.id = 'B', .size = 5, .cell = {{0, 0}, {1, 0}, {2, 0}, {0, 1}, {1, 1}}},
    {.id = 'C', .size = 5, .cell = {{0, 0}, {1, 0}, {2, 0}, {3, 0}, {0, 1}}},
    {.id = 'D', .size = 5, .cell = {{0, 0}, {1, 0}, {2, 0}, {3, 0}, {1, 1}}},
    {.id = 'E', .size = 5, .cell = {{0, 0}, {1, 0}, {2, 0}, {2, 1}, {3, 1}}},
    {.id = 'F', .size = 3, .cell = {{0, 0}, {1, 0}, {0, 1}}},
    {.id = 'G', .size = 5, .cell = {{0, 0}, {1, 0}, {2, 0}, {0, 1}, {0, 2}}},
    {.id = 'H', .size = 5, .cell = {{0, 0}, {1, 0}, {1, 1}, {2, 1}, {2, 2}}},
    {.id = 'I', .size = 5, .cell = {{0, 0}, {1, 0}, {2, 0}, {0, 1}, {2, 1}}},
    {.id = 'J', .size = 4, .cell = {{0, 0}, {1, 0}, {2, 0}, {3, 0}}},
    {.id = 'K', .size = 4, .cell = {{0, 0}, {1, 0}, {0, 1}, {1, 1}}},
    {.id = 'L', .size = 5, .cell = {{1, 0}, {0, 1}, {1, 1}, {2, 1}, {1, 2}}},
};

const size_t NOODLES_SIZE = sizeof(NOODLES) / sizeof(NOODLES[0]);

size_t sample_move_space(Polyomino sample[], int k)
{
    srand(time(NULL));
    size_t n = 0; // sampling counter

    // for each noodle
    for (int ndl = 0; ndl < NOODLES_SIZE; ndl++)
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
            int x_range = BOARD_WIDTH - (x1 - x0 + 1) + 1;  // ways to shift piece horizontally
            int y_range = BOARD_HEIGHT - (y1 - y0 + 1) + 1; // ways to shift piece vertically

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

void print_solution(const Polyomino *solution, size_t size)
{
    char board[BOARD_HEIGHT][BOARD_WIDTH];

    for (uint16_t i = 0; i < size; i++)
    {
        const Polyomino p = solution[i];
        char char_id = p.id;

        for (uint8_t c = 0; c < p.size; c++)
        {
            int16_t x = p.cell[c].x;
            int16_t y = p.cell[c].y;
            board[y][x] = char_id;
        }
    }

    for (int y = 0; y < BOARD_HEIGHT; y++)
    {
        for (int x = 0; x < BOARD_WIDTH; x++)
        {
            char c = board[y][x];
            printf("%c ", c);
        }
        printf("\n");
    }
}

size_t random_solution(Polyomino *solution, int k)
{
    // allocate sample move set (save limited stack size by alloc heap)
    Polyomino *moves = (Polyomino *)malloc(k * sizeof(Polyomino));
    if (!moves)
        return 0;

    // get random subset moves
    sample_move_space(moves, k);

    uint16_t num_cols = (BOARD_WIDTH * BOARD_HEIGHT) + NOODLES_SIZE;

    // calculate node allocation size: Root (1) + Column Headers (num_cols) + Data nodes
    uint16_t num_data_nodes = 0;
    for (uint16_t i = 0; i < k; i++)
        num_data_nodes += (1 + moves[i].size); // noodle id + cells

    uint16_t all_nodes = 1 + num_cols + num_data_nodes;

    // init solver dynamically (saves stack by using heap)
    DLXSolver solver = (DLXSolver){0};
    dlx_init(&solver, all_nodes, num_cols);

    // load moves into dlx
    for (uint16_t i = 0; i < k; i++)
    {
        uint16_t dlx_cols[1 + moves[i].size]; // noodle-id bit + cell cols
        uint16_t col_cnt = 0;

        // load piece-identifying node (1-indexed bc 0 is root)
        dlx_cols[col_cnt++] = 1 + (moves[i].id - 'A');

        // load cell-identifying nodes
        for (uint8_t c = 0; c < moves[i].size; c++)
        {
            int16_t x = moves[i].cell[c].x;
            int16_t y = moves[i].cell[c].y;

            uint16_t cell_index = (y * BOARD_WIDTH) + x;
            uint16_t cell_col = 1 + NOODLES_SIZE + cell_index; // board indices start after root & piece cols

            dlx_cols[col_cnt++] = cell_col;
        }

        // Add move row to matrix
        dlx_add_row(&solver, i, dlx_cols, col_cnt);
    }

    size_t result_size = 0;

    // solve dlx and get result
    if (dlx_search(&solver, 0))
    {
        for (uint16_t i = 0; i < solver.solution_size; i++)
        {
            uint16_t move_idx = solver.solution[i];
            solution[i] = moves[move_idx];
        }
        result_size = solver.solution_size;
    }

    // cleanup allocations
    dlx_free(&solver);
    free(moves);

    return result_size;
}
