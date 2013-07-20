#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <mpi.h>

#include "common.h"
#include "lrs/lrslib.h"

int num_vert, dim, tid, nthreads;
//useless
lrs_mp_vector output;

int facet_cnt(vertex *poly, int nv) {
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

		for(i=1,t=0;i<dim+1; i++, t++)
			num[dim+1-i] = (poly[row-1]>>t)&1;

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

void work() {
	int op, size, i, cur, j, k, t, s, bad=10000, minbad = 100000, fac;
	MPI_Status status;
	vertex *rcv, *cp;
	vertex *d;

	lrs_init("");
	output = lrs_alloc_mp_vector(dim+1);

	rcv = (vertex *)malloc(num_vert*RCV_SIZE);
	d = (vertex *)malloc(num_vert-1);

	while(1) {
		MPI_Send(&tid, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
		MPI_Recv(&op, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
		if(op)
			break;

		MPI_Recv(&size, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
		MPI_Recv(rcv, size*num_vert, MPI_BYTE, 0, 0, MPI_COMM_WORLD, &status);

		//number of vertices in num_vert, dimension in dim
		//do stuff with polytopes
		for(i = 0; i<size; i++) {
			//current polytope is in rcv + i*num_vert
			cp = rcv + i*num_vert;
			cur = facet_cnt(cp, num_vert);
			bad = 0;
			for(j = 0; j<num_vert; j++) {
				s = 0;
				for(k = 0; k<num_vert; k++) {
					if(k!=j)
						d[s++] = cp[k];
				}
				fac = facet_cnt(d, num_vert-1);
				if(fac <= cur)
					bad++;
			}
			if(!bad) {
				for(j = 0; j<num_vert; j++) {
					for(k = 0; k<dim; k++)
						printf("%d ", (cp[j]>>(dim-1-k))&1);

					printf("\n");
				}
				printf("\n");
			}
			if(bad < minbad) minbad = bad;
		}
	}
	MPI_Reduce(&minbad, &bad, 1, MPI_INT, MPI_MIN, 0, MPI_COMM_WORLD);
	lrs_clear_mp_vector(output, dim+1);
	lrs_close("");

	free(d);
	free(rcv);
}

int main(int argc, char *argv[]) {

	MPI_Init(&argc, &argv);

	if (argc < 4) {
		printf ("Wrong number of parameters\n");
		MPI_Finalize();
		return 0;
	}

	MPI_Comm_rank(MPI_COMM_WORLD, &tid);
	MPI_Comm_size(MPI_COMM_WORLD, &nthreads);

	num_vert = atoi(argv[2]);
	dim = atoi(argv[3]);

	if(!tid) {
		int num, elem, op, cnt = nthreads-1, bad = 10000, bd;
		long long total = 0;
		vertex *rcv;
		FILE *in;
		MPI_Status status;

		in = fopen(argv[1], "r");
		rcv = (vertex *)malloc(num_vert*RCV_SIZE);

		while(cnt) {
			MPI_Recv(&num, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			elem = fread(rcv, num_vert, RCV_SIZE, in);
			if(!elem) {
				op = 1;
				MPI_Send(&op, 1, MPI_INT, num, 0, MPI_COMM_WORLD);
				cnt--;
			}
			else {
				op = 0;
				MPI_Send(&op, 1, MPI_INT, num, 0, MPI_COMM_WORLD);
				MPI_Send(&elem, 1, MPI_INT, num, 0, MPI_COMM_WORLD);
				MPI_Send(rcv, num_vert*elem, MPI_BYTE, num, 0, MPI_COMM_WORLD);
				total += elem;
				printf("Sent %lld elements\n", total);
			}
		}
		MPI_Reduce(&bad, &bd, 1, MPI_INT, MPI_MIN, 0, MPI_COMM_WORLD);
		printf("%d\n", bd);
		printf("Finished dispatching...\n");
		fclose(in);
	}
	else {
		work();
	}
	MPI_Finalize();
	return 0;
}
