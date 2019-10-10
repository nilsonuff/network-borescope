/*
Copyright 2017, 2018, 2019 Nilson Luís Damasceno
Distributed under the GNU General Public License, Version 3.
Last update : October 10th, 2019.
*/

#ifndef NODESET_H
#define NODESET_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct s_hash_node {
    void   *ptr;
    int     counter; 
    struct s_hash_node *next;
} *PHashNode, HashNode;


typedef struct s_hash {
    struct s_hash *next;
    PHashNode     first;
	int           sz;
} *PHashEntry, HashEntry;

typedef struct s_nodes {	
	int sz;
	PHashEntry entries;
	PHashEntry used_entries;
} *PPointers, Pointers;

void   pointers_cleanup(void);

PPointers pointers_create(int capacity);
void   pointers_destroy(PPointers pns);


int    pointers_count(PPointers pns, void *ptr);
int    pointers_insert(PPointers pns, void *ptr);
int    pointers_remove(PPointers pns, void *ptr);

#ifdef __cplusplus
}
#endif

#endif /* NODESET_H */

