#include <stdio.h>
#include "lrs/lrslib.h"

int is_ort(int len, lrs_mp_vector a, lrs_mp_vector b) {
	lrs_mp tmp, tmp2, tmp3;
	int i;
	tmp[0] = 0;

	for(i = 0; i<len; i++) {
		mulint(a[i], b[i], tmp2);
		addint(tmp2, tmp, tmp3);
		copy(tmp, tmp3);
	}

	return zero(tmp);
}

int main(int argc, char *argv[]) {
	FILE *fp;
	lrs_dic *P;
	lrs_mp_vector output;
	lrs_mp_matrix Lin, A;
	lrs_dat *Q;
	int sgn, switch_sign;
	long rows, dim, *num, *den, i, j;
	if(argc != 2) {
		printf("Usage: incidence [filename]\n");
		return 1;
	}

	fp = fopen(argv[1], "r");

	if(fp == NULL) {
		printf("Something went wrong. Aborting.\n");
		return 1;
	}

	if(!lrs_init(""))
		return 1;

	Q = lrs_alloc_dat("LRS globals");

	if(Q == NULL)
		return 1;

	fscanf(fp, "%ld%ld", &rows, &dim);
	num = malloc(dim * sizeof(long));
	den = malloc(dim * sizeof(long));

	A = lrs_alloc_mp_matrix(rows, dim);

	Q->m = rows;
	Q->n = dim;
	Q->hull = TRUE;
	Q->polytope = TRUE;
	Q->getvolume = FALSE;

	P = lrs_alloc_dic(Q);

	if(!P)
		return 1;

	output = lrs_alloc_mp_vector(Q->n);

	for(i = 0; i<dim; i++)
		den[i] = 1;
	printf("Vertices:\n");
	for(i = 0; i<rows; i++) {
		for(j = 0; j<dim; j++) {
			fscanf(fp, "%ld", &num[j]);
			if(j > 0)
				printf("%ld ", num[j]);
			itomp(num[j], A[i][j]);
		}
		printf("\n");
		lrs_set_row(P, Q, i+1, num, den, GE);
	}
	printf("\n");
	if(!lrs_getfirstbasis(&P, Q, &Lin, FALSE))
		return 1;

	do {
		for(i = 0; i <= P->d; i++)
			if(lrs_getsolution(P, Q, output, i)) {
				sgn = 0, switch_sign = 0;
				if(!zero(output[0])) {
					pmp("", output[0]);
					sgn = 1;
				}
				for(j = 1; j<dim; j++) {
					if(zero(output[j]))
						continue;
					if(sgn)
						printf(" %c", positive(output[i])?'+':'-');
					if(!sgn)
						sgn = 1;
					if(negative(output[j])) {
						output[j][0] *= -1;
						switch_sign = 1;
					}
					if(!one(output[j]))
						pmp("", output[j]);
					if(!one(output[j]))
						printf("*");
					if(switch_sign)
						output[j][0] *= -1, switch_sign = 0;
					printf(" x%ld", j);
				}
				printf(" >= 0\n");
				for(j = 0; j<rows; j++) {
					printf("%d\t", is_ort(dim, output, A[j]));
				}
				printf("\n");
			}
	} while(lrs_getnextbasis(&P, Q, FALSE));

	lrs_clear_mp_vector(output, Q->n);
	lrs_free_dic(P,Q);
	lrs_free_dat(Q);
	fclose(fp);
	lrs_close("");
	return 0;
}
