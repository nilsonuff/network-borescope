/*
Copyright 2017, 2018, 2019 Nilson Luís Damasceno
Distributed under the GNU General Public License, Version 3.
Last update : October 10th, 2019.
*/

#ifndef __POOL__
#define __POOL__

#include <time.h>

typedef struct s_free_item {
    struct s_free_item *next;
} FreeItem, *PFreeItem;;

typedef struct s_free_pool {
    PFreeItem  first;
    int        n_items;
    int        item_sz;
    int        clear;
    clock_t    clock;
} FreePool, *PFreePool;

PFreePool pool_create(int item_sz, int clear);
void pool_reset(PFreePool pfp);
void pool_destroy(PFreePool pfp);
void *pool_alloc_item(PFreePool pfp);
void pool_free_item(PFreePool pfp, void *item);
void pool_free_percentage(PFreePool pfp, int percent);


#endif
