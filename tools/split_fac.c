#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <mpi.h>
#include <common.h>
#include <polytool.h>

int num_vert, dim, tid, nthreads;

FILE *open_file(int facet) {
	static char name[50];
	FILE *res;

	sprintf(name, "buff%d_%d.pol", tid, facet);

	res = fopen(name, "w");

	return res;
}

void work() {
	int op, size, i, cnt;
	MPI_Status status;
	vertex *rcv, *cur;
	FILE *f[300];

	for(i = 0; i<300; i++)
		f[i] = NULL;

	rcv = (vertex *)malloc(num_vert*RCV_SIZE);
	init_lrs();

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
			cur = rcv + i*num_vert;
			cnt = facet_cnt(cur, num_vert, dim);

			if(!f[cnt])
				f[cnt] = open_file(cnt);

			fwrite(cur, num_vert, 1, f[cnt]); 
		}
	}

	for(i = 0; i<300; i++)
		if(f[i])
			fclose(f[i]);

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
		int num, elem, op, cnt = nthreads-1;
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
		printf("Finished dispatching...\n");
		fclose(in);
	}
	else {
		work();
	}
	MPI_Finalize();
	return 0;
}
