#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
        int num_vert, t;
        unsigned char *a, *b, *c;

	if(argc != 3) {
		printf("Usage: dupe [file] [vertex number]\n");
		return 1;
	}

	FILE *fp = fopen(argv[1], "r");
	num_vert = atoi(argv[2]);
        a = (unsigned char *)malloc(num_vert);
        b = (unsigned char *)malloc(num_vert);
        c = (unsigned char *)malloc(num_vert);
        fread(a, num_vert, 1, fp);

        while(1) {
                t = fread(b, num_vert, 1, fp);
                if(!t)
                        break;
                if(!memcmp(a,b,num_vert)) {
                        printf("FOUND DUPE!\n");
                        return 0;
                }

                memcpy(a,b,num_vert);
        }

        fclose(fp);

        return 0;
}
