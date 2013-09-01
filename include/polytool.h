// polytool.h -- function prototypes and definitions for libpolytools

#ifndef _POLYTOOL_H
#define _POLYTOOL_H

#include <common.h>

//FACET COUNTING SUBROUTINES
//-------------------------------------------------------------

//init lrs, required before calling facet_cnt
void init_lrs();

//invokes lrs_close and frees output vector
void free_lrs();

//returns facet count of the polytope specified by vertex *poly with int nv vertices
int facet_cnt(vertex *poly, int nv, int dim);

//-------------------------------------------------------------

//ADJACENCY CHECKS
//-------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif

//fast check for 2-neighborly polytope, if it returns 1, call check2neighb_inc
int check2neighb_fast(vertex *tested, int numv, int dim);
//clp solver check for 2-neighborly polytope
int check2neighb_inc(vertex *tested, int numv, int dim);
//init 2-neighborly polytope checker
void init_adj(int num_vert);
//free allocated ram from init_adj
void free_adj();
//resize clp model so that it is suitable for checking polytopes with num_vert vertices
void resize_model(int num_vert);

#ifdef __cplusplus
}
#endif

#endif
