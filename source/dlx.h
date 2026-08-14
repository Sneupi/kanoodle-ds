#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct
{
    uint16_t left, right, up, down;
    uint16_t col; // Points to column header node index
    uint16_t row; // Row identifier (0 for header nodes)
} DLXNode;

typedef struct
{
    DLXNode *nodes;     // (Dynamic alloc) Array of nodes
    uint16_t *col_size; // (Dynamic alloc) Array for node counts per column
    uint16_t node_count;
    uint16_t *solution; // (Dynamic alloc) Array of solution row ids
    uint16_t solution_size;
} DLXSolver;

/**
 * @brief Initializes the solver by allocating exact node and column capacity.
 * @param max_nodes Total node capacity (Root + Headers + Data nodes)
 * @param num_cols Total number of active columns
 */
void dlx_init(DLXSolver *s, uint16_t max_nodes, uint16_t num_cols);
void dlx_free(DLXSolver *s);
void dlx_add_row(DLXSolver *s, uint16_t row_id, const uint16_t *cols, uint16_t col_cnt);
bool dlx_search(DLXSolver *s, int k);