#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <mpi.h>
#include <zlib.h>
#include <polytool.h>
#include <common.h>

int num_vert, dim, tid, nthreads;

void work() {
	int op, size, i, min = 100000, max = 0, cur, fmin, fmax;
	MPI_Status status;
	vertex *rcv;

	init_lrs();

	rcv = (vertex *)malloc(num_vert*RCV_SIZE);
	
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
			cur = facet_cnt(rcv+i*num_vert, num_vert, dim);
			if(cur > max)
				max = cur;
			if(cur < min)
				min = cur;
		}
	}

	MPI_Reduce(&min, &fmin, 1, MPI_INT, MPI_MIN, 0, MPI_COMM_WORLD);
	MPI_Reduce(&max, &fmax, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);

	free_lrs();

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
		int num, elem, op, cnt = nthreads-1, min = 1000000, max = 0, fmin, fmax, len, usegz = 0;
		char name[200];
		long long total = 0;
		vertex *rcv;
		FILE *in, *out;
		gzFile gin;
		MPI_Status status;

		len = strlen(argv[1]);
		if(argv[1][len-1]=='z' && argv[1][len-2] == 'g' && argv[1][len-3] == '.') 
			usegz = 1;
		
		if(usegz)
			gin = gzopen(argv[1], "r");
		else
			in = fopen(argv[1], "r");
		rcv = (vertex *)malloc(num_vert*RCV_SIZE);

		while(cnt) {
			MPI_Recv(&num, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			if(usegz)
				elem = (gzread(gin, rcv, num_vert*RCV_SIZE))/num_vert;
			else
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
		printf("Finished dispatching...\n");
		MPI_Reduce(&min, &fmin, 1, MPI_INT, MPI_MIN, 0, MPI_COMM_WORLD);
		MPI_Reduce(&max, &fmax, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);

		sprintf(name, "%s.cnt", argv[1]);
		out = fopen(name, "w");
		fprintf(out, "%d %d\n", fmin, fmax);
		fclose(out);

		if(usegz)
			gzclose(gin);
		else
			fclose(in);
	}
	else {
		work();
	}
	MPI_Finalize();
	return 0;
}
