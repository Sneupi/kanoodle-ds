#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define MAX_COLS 67
#define MAX_ROWS 900
#define NODES_PER_ROW 6
#define MAX_NODES (1 + MAX_COLS + (MAX_ROWS * NODES_PER_ROW)) // Root + Headers + Data

typedef struct {
    uint16_t left, right, up, down;
    uint16_t col; // Points to column header node index
    uint16_t row; // Row identifier (0 for header nodes)
} DLXNode;

typedef struct {
    DLXNode nodes[MAX_NODES];
    uint16_t col_size[MAX_COLS + 1]; // Node counts per column
    uint16_t node_count;
    uint16_t solution[MAX_ROWS];
    uint16_t solution_size;
} DLXSolver;

static DLXSolver solver; // Reserve memory statically

void dlx_init(DLXSolver *s, uint16_t num_cols) {
    s->node_count = num_cols + 1; // 0 is root, 1..num_cols are column headers
    s->solution_size = 0;

    // Initialize root and column headers
    for (uint16_t i = 0; i <= num_cols; i++) {
        s->nodes[i].left = (i == 0) ? num_cols : i - 1;
        s->nodes[i].right = (i == num_cols) ? 0 : i + 1;
        s->nodes[i].up = i;
        s->nodes[i].down = i;
        s->nodes[i].col = i;
        s->nodes[i].row = 0;
        s->col_size[i] = 0;
    }
}

void dlx_add_row(DLXSolver *s, uint16_t row_id, const uint16_t *cols, uint16_t col_cnt) {
    uint16_t first_node = s->node_count;

    for (uint16_t i = 0; i < col_cnt; i++) {
        uint16_t c = cols[i];
        uint16_t new_node = s->node_count++;

        s->nodes[new_node].row = row_id;
        s->nodes[new_node].col = c;

        // Insert into column doubly linked list (at bottom)
        s->nodes[new_node].down = c;
        s->nodes[new_node].up = s->nodes[c].up;
        s->nodes[s->nodes[c].up].down = new_node;
        s->nodes[c].up = new_node;
        s->col_size[c]++;

        // Insert into row doubly linked list
        if (i == 0) {
            s->nodes[new_node].left = new_node;
            s->nodes[new_node].right = new_node;
        } else {
            s->nodes[new_node].left = s->nodes[first_node].left;
            s->nodes[new_node].right = first_node;
            s->nodes[s->nodes[first_node].left].right = new_node;
            s->nodes[first_node].left = new_node;
        }
    }
}

static inline void cover(DLXSolver *s, uint16_t c) {
    // Unlink column header from horizontal list
    s->nodes[s->nodes[c].right].left = s->nodes[c].left;
    s->nodes[s->nodes[c].left].right = s->nodes[c].right;

    // Unlink all rows in this column
    for (uint16_t i = s->nodes[c].down; i != c; i = s->nodes[i].down) {
        for (uint16_t j = s->nodes[i].right; j != i; j = s->nodes[j].right) {
            s->nodes[s->nodes[j].down].up = s->nodes[j].up;
            s->nodes[s->nodes[j].up].down = s->nodes[j].down;
            s->col_size[s->nodes[j].col]--;
        }
    }
}

static inline void uncover(DLXSolver *s, uint16_t c) {
    // Relink all rows in this column (in reverse)
    for (uint16_t i = s->nodes[c].up; i != c; i = s->nodes[i].up) {
        for (uint16_t j = s->nodes[i].left; j != i; j = s->nodes[j].left) {
            s->col_size[s->nodes[j].col]++;
            s->nodes[s->nodes[j].down].up = j;
            s->nodes[s->nodes[j].up].down = j;
        }
    }

    // Relink column header
    s->nodes[s->nodes[c].right].left = c;
    s->nodes[s->nodes[c].left].right = c;
}

bool dlx_search(DLXSolver *s) {
    // If root.right == 0, matrix is empty; exact cover found
    if (s->nodes[0].right == 0) {
        return true;
    }

    // Choose column deterministically with Minimum Remaining Values heuristic
    uint16_t c = s->nodes[0].right;
    uint16_t min_size = s->col_size[c];

    for (uint16_t i = s->nodes[c].right; i != 0; i = s->nodes[i].right) {
        if (s->col_size[i] < min_size) {
            min_size = s->col_size[i];
            c = i;
        }
    }

    cover(s, c);

    for (uint16_t r = s->nodes[c].down; r != c; r = s->nodes[r].down) {
        s->solution[s->solution_size++] = s->nodes[r].row;

        for (uint16_t j = s->nodes[r].right; j != r; j = s->nodes[j].right) {
            cover(s, s->nodes[j].col);
        }

        if (dlx_search(s)) return true; // Solution found

        // Backtrack
        s->solution_size--;
        for (uint16_t j = s->nodes[r].left; j != r; j = s->nodes[j].left) {
            uncover(s, s->nodes[j].col);
        }
    }

    uncover(s, c);
    return false;
}