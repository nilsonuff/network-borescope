/*
Copyright 2017, 2018, 2019 Nilson Luís Damasceno
Distributed under the GNU General Public License, Version 3.
Last update : October 10th, 2019.
*/

#ifndef __STATS
#define __STATS

//***************************************************************************
//
//    Statistics
//
//***************************************************************************

#define __STATS__ 

typedef struct s_stats{
    int n_nodes;      // including betas (and terminals)

    int n_insertions;
    int n_distinct_insertions;
    int n_removals; 

    int n_betas;      // including terminals
    int n_beta_terminals;
    int n_terminals;
    int n_primary_terminals;

    int n_shared_child_terminals;
    int n_proper_child_terminals;

    int n_shared_child_edges;
    int n_proper_child_edges;

    int n_shared_to_proper;

    int max_ref_counter;

    int terminal_shallow_copies;

    long long max_memory_used_kb;
    long long acc_memory_used_kb;
    long long count_memory_used;


} Stats, *PStats;

extern Stats stats;

void stats_reset(void);
void stats_dump(void);


#endif