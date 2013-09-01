#include <stdio.h>
#include "mathlink.h"
#include "lrs/lrslib.h"

void HRepresentation(void) {
	char **heads;
	int *dims;
	int *a, depth, i;
	long col, *num, *den, row;
	lrs_dic *P;
	lrs_dat *Q;
	lrs_mp_matrix Lin;
	lrs_mp_vector output;

	if(!MLGetInteger32Array(stdlink, &a, &dims, &heads, &depth)) {
		return ;
	}
	lrs_init("");

	Q = lrs_alloc_dat ("LRS globals");

	Q->m = dims[0];
	Q->n = dims[1]+1;
	Q->hull = TRUE;
	Q->polytope = TRUE;
	Q->getvolume = FALSE;

	P = lrs_alloc_dic(Q);

	output = lrs_alloc_mp_vector(Q->n);
	num = malloc((dims[1]+1)*sizeof(long));
	den = malloc((dims[1]+1)*sizeof(long));

	for(row = 1; row <= Q->m; row++) {
		num[0] = den[0] = 1;

		for(i = 1; i<Q->n; i++) {
			num[i] = a[i-1 + (row-1)*dims[1]], den[i] = 1;
		}

		lrs_set_row(P, Q, row, num, den, GE);
	}

	lrs_getfirstbasis(&P, Q, &Lin, TRUE);
	MLPutFunction(stdlink, "List", 1);
	char *res = malloc(10000);
	do {
		for(col = 0; col <= P->d; col++) {
			if(lrs_getsolution(P, Q, output, col)) {
				MLPutFunction(stdlink, "Sequence", 2);
				MLPutFunction(stdlink, "List", Q->n);
				for(i = 0; i<Q->n; i++) {
					spmp(res, output[i]);
					MLPutFunction(stdlink, "ToExpression", 1);
					MLPutString(stdlink, res);
				}
			}
		}
	} while(lrs_getnextbasis(&P, Q, FALSE));
	MLPutFunction(stdlink, "Sequence", 0);
	free(res);
	lrs_clear_mp_vector(output, Q->n);
	lrs_free_dic (P,Q);
	lrs_free_dat (Q);
	lrs_close("");
	/*MLPutFunction(stdlink, "List", dims[0]);
	for(i = 0; i<dims[0]; i++) {
		MLPutInteger32List(stdlink, a+i*dims[1], dims[1]);
	}*/
	MLReleaseInteger32Array(stdlink, a, dims, heads, depth);
	return ;
}


int main(int argc, char* argv[])
{
	return MLMain(argc, argv);
}
