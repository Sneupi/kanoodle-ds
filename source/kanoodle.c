
#include "kanoodle.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include "dlx.h"

/**
 * For the game of Kanoodle, we have 12 pieces "noodles"
 * that reside on a 11x5 board. In order to find solutions
 * for Kanoodle, we may define the set of all possible
 * placements in all orientations in all placements,
 * and run this well-defined "move space" through an
 * exact-cover algorithm such as Dancing Links (DLX).
 *
 * In order to fit the move space of Kanoodle into DLX-ready
 * matrix rows, we can define each move as one matrix row
 * consisting of 55 columns (deconstruction of the 11x5 grid)
 * plus 12 additional columns to denote which piece is
 * actively used by this move/row. From here, we can
 * simply churn our rows through DLX to find the entire
 * solution space of Kanoodle.
 *
 * For purposes of human gameplay, we arrange DLX to become
 * a random solution generator. In pursuit of efficiency,
 * one approach to achieve this is to provide a randomized
 * subset of k moves in the "move space" to DLX upon each run.
 * Adding randomness to which solution is found first,
 * as well as lowering runtime and memory cost.
 */

void sample(int *population, int n, int *sample, int k, unsigned int seed)
{
    int *copy = malloc(n * sizeof(int));
    for (int i = 0; i < n; i++)
        copy[i] = population[i];

    // Fisher-Yates shuffle
    srand(seed);
    for (int i = 0; i < k; i++)
    {
        int j = i + rand() % (n - i);
        int swap = copy[i];
        copy[i] = copy[j];
        copy[j] = swap;
        sample[i] = copy[i];
    }
    free(copy);
}

int compare_ints(const void *a, const void *b)
{
    int arg1 = *(const int *)a;
    int arg2 = *(const int *)b;
    if (arg1 < arg2)
        return -1;
    if (arg1 > arg2)
        return 1;
    return 0;
}

// Generate a dlx matrix with a random subset of move-space for the game Kanoodle
int random_matrix(struct dlx_matrix *dlx)
{
    const Polyomino noodles[12] = {NOODLE_A, NOODLE_B, NOODLE_C, NOODLE_D, NOODLE_E, NOODLE_F, NOODLE_G, NOODLE_H, NOODLE_I, NOODLE_J, NOODLE_K, NOODLE_L};

    // all row indices in move space
    int n_mi = 1789;
    int move_indices[n_mi];
    for (int i = 0; i < n_mi; i++)
        move_indices[i] = i;

    // sample row indices in move space
    int n_si = 900;
    int sample_indices[n_si];
    sample(move_indices, n_mi, sample_indices, n_si, time(NULL));
    qsort(sample_indices, n_si, sizeof(int), compare_ints);

    // determine moves & node count
    Polyomino moves[n_si];
    size_t n_row = n_si;
    size_t n_col = (11 * 5) + 12; // 11x5 board + 12 pieces
    size_t n_nodes = 0;

    int i_si = 0; // iterator on sample indices
    int i_p = 0;  // iterator on placements
    for (int i = 0; i < 12; i++)
    {
        // gen orientations
        Polyomino *ori_arr = NULL;
        int n_oa = get_orientations(noodles[i], &ori_arr);

        for (int j = 0; j < n_oa; j++)
        {
            // gen placements
            Polyomino ori = ori_arr[j];
            int x_range = 11 - width(ori) + 1;
            int y_range = 5 - height(ori) + 1;

            for (int dx = 0; dx < x_range; dx++)
            {
                for (int dy = 0; dy < y_range; dy++)
                {
                    // if move included, incr node count & save move
                    if (i_p == sample_indices[i_si])
                    {
                        moves[i_si] = shifted(ori, dx, dy);
                        n_nodes += ori.size + 1;
                        i_si++;
                    }
                    i_p++;
                }
            }
        }
        free(ori_arr);
    }

    // allocate matrix
    if (!(dlx->headers = calloc(n_col, sizeof(*dlx->headers))) ||
        !(dlx->nodes = calloc(n_nodes, sizeof(*dlx->nodes))) ||
        !(dlx->row_off = calloc(n_row + 1, sizeof(*dlx->row_off))))
    {
        free(dlx->headers);
        free(dlx->nodes);
        free(dlx->row_off);
        return -1; // bad alloc
    }
    dlx_make_header_row(&dlx->root, dlx->headers, n_col);
    dlx->n_col = n_col;
    dlx->n_row = n_row;

    // populate matrix with moves
    size_t row_idx = 0;
    size_t node_off = 0;
    for (int i = 0; i < n_si; i++)
    {
        Polyomino move = moves[i];

        // declare node array to load move into
        struct dlx_hnode *columns[6];
        size_t n = 0;

        // init the physical offset of this row's location in dlx matrix
        dlx->row_off[row_idx] = node_off;

        // load move into node array
        for (int c = 0; c < move.size; c++)
        {
            columns[n++] = dlx->headers + (move.cell[c].x * 5 + move.cell[c].y);
        }
        columns[n++] = dlx->headers + (55 + (move.id - 'A'));

        // load node array into matrix
        dlx_make_row(dlx->nodes + node_off, dlx->row_off + row_idx, n);
        dlx_add_row(dlx->nodes + node_off, columns, n);

        node_off += n;
        row_idx++;
    }
    dlx->row_off[row_idx] = node_off;

    return 0;
}

// Generate a random solution to the game of Kanoodle
KanoodleSol random_solution()
{
    KanoodleSol ksol = {{0}};
    struct dlx_matrix dlx;
    struct dlx_srow *solutions = NULL;
    size_t n;
    size_t i = 1;

    if (random_matrix(&dlx) < 0)
        return ksol; // bad alloc

    if ((solutions = calloc(dlx.n_row, sizeof(*solutions))) == NULL)
        goto cleanup; // bad alloc

    if ((n = dlx_exact_cover(solutions, &dlx.root, 0, &i)) == 0 && dlx.n_col != 0)
        goto cleanup; // no solution

    // convert solution rows to kanoodle solution
    for (i = 0; i < n; i++)
    {
        struct dlx_node *start = solutions[i].row_node;
        struct dlx_node *node = start;
        char piece = 0;

        if (!start)
            continue;

        // first find the piece column (55..66 => A..L)
        do
        {
            ptrdiff_t col = node->header - dlx.headers;
            if (col >= 55)
            {
                piece = (char)(col - 55);
                break;
            }
            node = node->right;
        } while (node != start);

        // then assign every board cell in this row to that piece
        node = start;
        do
        {
            ptrdiff_t col = node->header - dlx.headers;
            if (col >= 0 && col < 55)
            {
                int x = (int)(col / 5);
                int y = (int)(col % 5);
                ksol.grid[x][y] = piece;
            }
            node = node->right;
        } while (node != start);
    }

cleanup:
    // mem cleanup
    free(dlx.headers);
    free(dlx.nodes);
    free(dlx.row_off);
    free(solutions);
    return ksol;
}

// Print a Kanoodle solution board as text
void print_solution(KanoodleSol sol)
{
    static const char *colors[] = {
        "\x1b[38;5;208m", // A - orange
        "\x1b[31m",       // B - red
        "\x1b[34m",       // C - blue
        "\x1b[95m",       // D - pink
        "\x1b[32m",       // E - darkgreen
        "\x1b[97m",       // F - white
        "\x1b[94m",       // G - lightblue
        "\x1b[91m",       // H - hotpink
        "\x1b[93m",       // I - yellow
        "\x1b[35m",       // J - purple
        "\x1b[92m",       // K - green
        "\x1b[90m",       // L - grey
    };
    const char *reset = "\x1b[0m";

    for (int y = 0; y < 5; y++)
    {
        for (int x = 0; x < 11; x++)
        {
            char v = sol.grid[x][y];
            printf("%s%c", colors[v], 'A' + v);
        }
        printf("\n");
    }
    printf("%s", reset);
}
