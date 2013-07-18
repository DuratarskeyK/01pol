#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
	int c, i;
	FILE *a, *b;
	char *t1, *t2;
	c = atoi(argv[1]);

	a = fopen(argv[2], "r");
	b = fopen(argv[3], "r");
	t1 = malloc(c);
	t2 = malloc(c);


	while(!feof(a)) {
		fread(t1, c, 1, a);
		fread(t2, c, 1, b);

		if(memcmp(t1,t2,c)) {
			printf("Diff:\n");
			for(i = 0; i<c; i++)
				printf("%x ", t1[i]);
			printf("\n");
			for(i = 0; i<c; i++)
				printf("%x ", t2[i]);
			printf("\n");
			return 0;
		}
	}

}
