/*
Copyright 2017, 2018, 2019 Nilson Luís Damasceno
Distributed under the GNU General Public License, Version 3.
Last update : October 10th, 2019.
*/

#ifndef NODE_STD_H
#define NODE_STD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "schema.h"
    
void node_std_init_functions(NodeFunctions *pfs);
void node_std_cleanup(void);

#ifdef __cplusplus
}
#endif

#endif /* NODE_STD_H */

