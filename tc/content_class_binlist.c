/*
Copyright 2017, 2018, 2019 Nilson Luís Damasceno
Distributed under the GNU General Public License, Version 3.
Last update : October 10th, 2019.
*/

#include <stdlib.h>
#include "metadata.h"
#include "register.h"
#include "query.h"

//=============================================================================================
//
//    
//
//=============================================================================================

typedef struct s_bnode *PBinNode;
typedef unsigned long bin_t;

typedef struct s_bnode {
    bin_t bin;
    int count;
    PBinNode  next;
    Byte data [];
} BinNode;

typedef struct {
    PBinNode first; 
    PBinNode last; 
    int bin_size;
    int n_nodes;
    int node_size;
    int shallow;
    __int64_t count;
} BinList, *PBinList;

typedef struct {
    unsigned int bin_size;
} BParams, *PBinListParams;

int total_n_nodes = 0;
int max_blist_n_nodes = 0;
int hist[25001];

static Byte shallow_data[1024];

//-----------------------
//
//
//
//
static void *binlist_insert(PRecord record, void *data, PContentData pcd) {
    PBinList pbl = (PBinList) data;
    PBinListParams pblp = (PBinListParams) pcd->params;
    pbl->count ++;
    
    if (pbl->bin_size != pblp->bin_size) 
        fatal("BIN_SIZE: [%p %d] != [%p %d]", pbl, pbl->bin_size, pblp, pblp->bin_size);

#if 0    
    if (!max_blist_n_nodes) {
        memset(hist,0,sizeof(int) * 25000);
    }
#endif

    if(pbl->shallow) return &shallow_data;

    __int64_t value = get_record_value(record, pcd,0);
    value /= pblp->bin_size;
    if (value > __INT32_MAX__) fatal("BINLINST: overflow\n");
    bin_t bin = (bin_t) value;

    PBinNode p = pbl->last;
    //printf("pbl = %p  l = %p\n", pbl, p);
    if (!p) {
        pbl->node_size = sizeof(BinNode) + pcd->total_data_size;
    } else {
        //printf("BINLIST: found bin %d %d  Count: %d\n", p->bin, bin, p->count);
        //printf("Epoch %d  Bin = %d Last = %p\n", value, bin, p );
        if (bin == p->bin) {
            p->count ++;
            return p->data;
        } else if (bin < p->bin) {
            fatal("Backwarding time is not allowed");
        } else {
            //printf("x");
        }
    }
    // printf("BINLIST: new bin %d  %d  %p\n", bin, value, pbl);

    // allocates space to store subchannel data
    p = (PBinNode) calloc(1, pbl->node_size);        
    p->bin = bin;
    p->count = 1;

    // encadeia no final da lista
    if (!pbl->last) { 
        pbl->first = p; 
    } else { 
        pbl->last->next = p; 
    } 
    pbl->last = p;
    pbl->n_nodes ++;

    total_n_nodes ++;
    hist[pbl->n_nodes] ++;
    if (pbl->n_nodes > max_blist_n_nodes) max_blist_n_nodes = pbl->n_nodes;

    return p->data;
}


//-----------------------
//
//
//
//
static void *binlist_remove(PRecord record, void *data, PContentData pcd) {
    PBinList pbl = (PBinList) data;
    PBinListParams pblp = (PBinListParams) pcd->params;
    pbl->count --;
    
    if(pbl->shallow) return &shallow_data;

    __int64_t value = get_record_value(record, pcd,0);
    value /= pblp->bin_size;
    bin_t bin = (bin_t) value;

    PBinNode prev = NULL;
    PBinNode p = pbl->first;
    for ( ; p; prev = p, p = p->next) {
        if (p->bin == bin) {
            p->count --;   // pode ter zerado
            if (p->count) return p->data;
            if (prev) { 
                prev->next = p->next; 
            } else {
                pbl->first = p->next;
            }
            free(p);
            pbl->n_nodes --;
            return NULL;
        } else if (p->bin > bin) {
            return NULL;
        }
    }
    return NULL;
}



//----------------------
//
//
//
//
static void _shallow_copy(PByte dst, PByte src, int n_bytes) {    
    PBinList s = (PBinList) src;
    PBinList d = (PBinList) dst;
    PBinNode p, q;

    //d->shallow = 1;
    //return;

    memset(d,0,sizeof(BinList));
    d->bin_size = s->bin_size;
    d->node_size = s->node_size;

    // printf(" SHALLOW %d ",d->bin_size); fflush(stdout);
    
    for (p = s->first; p; p = p->next) {
        q = calloc(1, d->node_size);
        
        // deveria fazer um shallow copy recursivo
        memcpy(q->data,p->data,s->node_size - sizeof(BinNode));
        
        q->count = p->count;
        q->bin = p->bin;

        if (!d->last) { d->first = q; } else { d->last->next = q; }
        d->last = q;

        d->count += p->count;
        d->n_nodes ++;
        total_n_nodes ++;
    }

    for (int i = 1; i<=s->n_nodes; i++) {
        hist[i] += s->n_nodes;
    }
}

//----------------------
//
//
//
//
static void _prepare(int op, void *content,  PContentData pcd) {
    PBinList pbl = (PBinList) content;
    PBinListParams pblp = (PBinListParams) pcd->params;

    if (op == TERMINAL_OP_INIT) {
        pbl->bin_size = pblp->bin_size;
        // printf(" [PREPARE %p %d] ", pbl, pbl->bin_size); fflush(stdout);
    }
}

//----------------------
//
//
//
//
static void _parse_param(int id_param, char *s, PByte params) {
    // printf("BINLIST ParseParam:\n");
    PBinListParams pblp = (PBinListParams) params;
    if (id_param == 1) {
        pblp->bin_size = atoi(s);
        // printf("BIN LIST: BinSize %d\n", pblp->bin_size); fflush(stdout);
        if (!pblp->bin_size) fatal("BINLIST: BinSize Error. ");
    } else {
        fatal("BINLIST: Extra parameter 1 %d\n",id_param);        
    }
}

/*-----------------
*
*
*
*/
static int binlist_find_bounds(PByte pdata, int *bounds) {
    PBinList pbl = (PBinList) pdata;

    PBinNode p = pbl->first;
    if (!p) return 0; 

    bounds[0] = p->bin  * pbl->bin_size;

    while (p->next) p = p->next;
    bounds[1] = p->bin * pbl->bin_size;

    return 1;
}


//----------------------
//
//
//
//
static void register_content_binlist(PClass pclass) {

    static ContentFunctions cfs;
    cfs.parse_param = _parse_param;
    cfs.prepare = _prepare;
    cfs.shallow_copy = _shallow_copy;
    cfs.insert = &binlist_insert;
    cfs.remove = &binlist_remove;
    cfs.find_bounds = &binlist_find_bounds;
    // cfs.remove

    static ContentInfo ci;
    memset(&ci, 0, sizeof(ContentInfo));

    ci.n_param_fields = 1;
    ci.min_params = 1;
    ci.max_params = 1;
    ci.params_size = sizeof(BParams);
    ci.private_size = sizeof(BinList);
    ci.pclass = pclass;

    ci.pcfs = &cfs;

    register_content("binlist", &ci);
}

//=============================================================================================
//
//    
//
//=============================================================================================

static int end_of_data;
#define END_OF_DATA (&end_of_data)

/*-----------------
*
*
*
*/
static void *cop_between(PQArgs pa, PQOpData pdata, void **ppstate) {
    //printf("COP BETWEEN: First %p\n",pdata);
     if (*ppstate == END_OF_DATA) return NULL;

    PBinList pbl = (PBinList) pdata;
    PBinNode p = *ppstate;
    if (!p) {
        // printf("BETWEEN: First\n");
        p =  pbl->first;
    } else {
        p = p->next;
        if (!p) { *ppstate = END_OF_DATA; return NULL; }
    }

    //printf("1"); fflush(stdout);
    bin_t t0 = pa->args[0].i/pbl->bin_size;
    bin_t t1 = pa->args[1].i/pbl->bin_size;
    if (t0 == t1) t1 ++;
//    printf("t0 = %ld\n",t0);

    //printf("X"); fflush(stdout);
    // printf("BETWEEN: %d %d %p BIN: %d COunt: %d PBL Count %d  %d\n", t0, t1, p, p->bin, p->count, pbl->count, pbl->n_nodes);

    // localiza o inicio do between
    if (p->bin < t0) {
        do {
            // printf("P.BIN %d\n",p->bin);
            p = p->next;
        } while (p && (p->bin < t0));
        if (!p) { *ppstate = END_OF_DATA; return NULL; }
    }

    // enquanto estiver 
    if ( (p->bin >= t0) && (p->bin <= t1)) {
        // printf("P.BIN FOUND %d\n",p->bin);
        *ppstate = p;
        return p->data;
    }

    // printf("END OF DATA\n");
    *ppstate = END_OF_DATA;
    return NULL;
}


/*-----------------
*
*
*
*/
static void *cog_between(PQArgs pa, PQOpData pdata, int *indexes, void **ppstate) {
    if (*ppstate == END_OF_DATA) return NULL;

    PBinList pbl = (PBinList) pdata;
    // printf("BETWEEN 0: %d %d %p %d\n", pa->args[0].i, pa->args[1].i,pdata, pbl->n_nodes);

    PBinNode p = *ppstate;
    if (!p) {
        // printf("BETWEEN: First\n");
        p =  pbl->first;
        // printf("Count: %d Size: %d\n", pbl->count, pbl->bin_size);
    } else {
        p = p->next;
    }
    
    if (!p) { 
        // printf("X\n");
        *ppstate = END_OF_DATA; 
        return NULL; 
    }
  
    //printf(" G %d ",pbl->bin_size);fflush(stdout);
    bin_t t0 = pa->args[0].i/pbl->bin_size;
    bin_t t1 = pa->args[1].i/pbl->bin_size;
    if (t0 == t1) t1 ++;

    // printf("COG t0 = %ld\n",t0);
    //printf("X");fflush(stdout);
    
    // printf("BETWEEN: %d %d\n", t0, t1);

    // localiza o inicio do between
    while (p) {
        // printf("%d\n",p->bin);
        if ((p->bin >= t0)) break;
        p = p->next;
    }
    if (!p) { *ppstate = END_OF_DATA; return NULL; }

    // printf("BETWEEN: %d %d\n", pa->args[0].i, pa->args[1].i);

    // enquanto estiver 
    if ( (p->bin >= t0) && (p->bin < t1)) {
        *ppstate = p;
        if (indexes) { 
            //indexes[0] = (p->bin   - t0) * pbl->bin_size; 
            indexes[0] = (p->bin   - t0);  // Posicao relativa aos bins
  //          printf("BINLIST: Index %d \n", indexes[0]);
        }
        // printf("OFFS: %d \n", p->bin);
        return p->data;
    }

    *ppstate = END_OF_DATA;
    return NULL;
}

#define REGISTER_BINLIST_OP(X,S,Y)  \
    static QOpInfo qoi_##X;\
    qoi_##X.op = cop_##X;\
    qoi_##X.op_group_by = cog_##X;\
    qoi_##X.min_args = Y;\
    qoi_##X.max_args = MAX_ARGS;\
    qoi_##X.fmt = "II"   ;\
    register_op("binlist", S, &qoi_##X)

/*-----------------
*
*
*
*/
static void register_ops(void) {
    REGISTER_BINLIST_OP(between,"between",2);
    qoi_between.max_args = 2;
}


//=============================================================================================
//
//    CLASS 
//
//=============================================================================================

/*--------------------------
*
*
*
*/
static int get_distinct_info(void * cop, PQArgs pa, PByte params, int *distinct_count, int *distinct_base) {
    //printf("Z1\n"); fflush(stdout);
    PBinListParams pblp = (PBinListParams) params;

    if (cop == cop_between) {
        int begin = pa->args[0].i;
        int end   = pa->args[1].i;
        // printf("Z1 begin %d end %d\n", begin, end); fflush(stdout);
        
        //printf("BINLIST: %d\n", pblp->bin_size); fflush(stdout);

        *distinct_count = ((end - begin) / pblp->bin_size); 
        if (!*distinct_count) *distinct_count = 1;
        *distinct_base = begin;
        // printf("Z1\n"); fflush(stdout);

        return 1;
    }
    return 0;
}


/*--------------------------
*
*
*
*/
static void *create_op_data(void * cop, PQArgs pa) {

    if (cop == cop_between) {
        return calloc(1,sizeof(BinList));
    } 
    return NULL;
}

/*--------------------------
*
*
*
*/
static void destroy_op_data(void *cop, void *op_data) {
    // assume que o conteudo foi liberado
    if (op_data) free(op_data);
}

/*--------------------------
*
*
*
*/
void register_content_class_binlist(void) {

    // prevention against reinitialization during execution
    static int done = 0; 
    if (!done) done = 1; else return;
    
    static Class c;
    memset(&c, 0, sizeof(c));

    c.name = "binlist";
    c.class_type = CONTAINER_CLASS;

    c.n_group_by_indexes = 1;
    c.group_by_requires_rule = 1;  // was 0

    c.get_distinct_info = &get_distinct_info;

    c.create_op_data = &create_op_data;
    c.destroy_op_data = &destroy_op_data;

#if CLASS_PARSE
    c.n_fields = 1;
    c.n_params = 1;
    c.parse_param = _parse_param;
#endif

    register_class(&c);
    register_content_binlist(&c);
    register_ops();
}


