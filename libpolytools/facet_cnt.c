#include <polytool.h>
#include <common.h>
#include <lrs/lrslib.h>

//not used here, but required by lrs_getsolution
static lrs_mp_vector output;

void init_lrs() {
	lrs_init("");
	output = lrs_alloc_mp_vector(15);
}

void free_lrs() {
	lrs_clear_mp_vector(output, 15);
	lrs_close("");
}

//returns the number of facets in a polytope with nv vertices in dimension dim
int facet_cnt(vertex *poly, int nv, int dim) {
	lrs_dic *P;
	lrs_dat *Q;
	long col, row, i, t;
	int ans;
	lrs_mp_matrix Lin;
	long num[24];
	long den[24];

	Q = lrs_alloc_dat ("LRS globals");
	
	Q->m = nv;
	Q->n = dim+1;
	Q->hull = TRUE;
	Q->polytope = TRUE;
	Q->getvolume = FALSE;

	P = lrs_alloc_dic(Q);
	
	for(i = 0; i<Q->n; i++)
		den[i] = 1;
	
	for(row = 1; row<=nv; row++) {
		num[0] = 1;

		for(i=1,t=0;i<dim+1; i++, t++) {
			num[dim+1-i] = (poly[row-1]>>t)&1;
		}

		lrs_set_row(P, Q, row, num, den, GE);
	}
	
	lrs_getfirstbasis(&P, Q, &Lin, TRUE);

	do {
		for(col = 0; col <= P->d; col++)
			lrs_getsolution(P, Q, output, col);
	} while(lrs_getnextbasis(&P, Q, FALSE));
	
	ans = Q->count[0];

	lrs_free_dic (P,Q);
	lrs_free_dat (Q);
	return ans;
}
