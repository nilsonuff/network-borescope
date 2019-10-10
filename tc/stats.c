/*
Copyright 2017, 2018, 2019 Nilson Luís Damasceno
Distributed under the GNU General Public License, Version 3.
Last update : October 10th, 2019.
*/

#include <memory.h>
#include <stdio.h>
#include "stats.h"

Stats stats;

/**
 * 
 *  
 */
void stats_reset(void) {
    memset(&stats, 0, sizeof(Stats));
}

void stats_dump(void) {
    printf("_____________________________________\n");
    printf("Total Insertions : %d\n",stats.n_insertions);
    printf("Total Removals : %d\n",stats.n_removals);
    printf("______\n");
    printf("Total Nodes (all): %d\n",stats.n_nodes);
    printf("Total Terminals: %d\n",stats.n_terminals);
    printf("Total Non-Terminals: %d\n",stats.n_nodes - stats.n_terminals);
    printf("______\n");
    printf("Total Primary Terminals: %d\n",stats.n_primary_terminals);
    printf("Total Non-Primary Terminals: %d\n",stats.n_terminals - stats.n_primary_terminals);
    printf("______\n");
    printf("Betas: %d\n",stats.n_betas);
    printf("Betas - Terminal: %d\n",stats.n_beta_terminals);
    printf("Betas - Non-Terminal : %d\n",stats.n_betas - stats.n_beta_terminals);
    printf("______\n");
    printf("Shared Child Edges: %d\n",stats.n_shared_child_edges);
    printf("Proper Child Edges: %d\n",stats.n_proper_child_edges);
    printf("______\n");
    printf("Shared Child Terminals: %d\n",stats.n_shared_child_terminals);
    printf("Proper Child Terminals: %d\n",stats.n_proper_child_terminals);
    printf("Proper Child Primary Terminals: %d\n",stats.n_primary_terminals);
    printf("Proper Child Non-Primary Terminals: %d\n",
        stats.n_proper_child_terminals - stats.n_primary_terminals);
    printf("______\n");
    printf("Proper Child Edges + Betas: %d\n",
        stats.n_proper_child_edges + stats.n_betas);
    printf("Proper Child Terminals + Beta Terminals: %d\n",
        stats.n_proper_child_terminals + stats.n_beta_terminals);
    printf("Shared -> Proper: %d\n",stats.n_shared_to_proper);
    printf("Terminal Shallow Copies: %d\n",stats.terminal_shallow_copies);
    printf("______\n");
    printf("Maximum Reference Count Value: %d\n",stats.max_ref_counter);
    printf("______\n");
    printf("Maximum Used Memory: %lld\n",stats.max_memory_used_kb);
    printf("Average Used Memory: %lld\n",stats.acc_memory_used_kb / stats.count_memory_used);
}

