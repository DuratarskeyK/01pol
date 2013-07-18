#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	FILE *b1, *b2, *out;
	unsigned char *a, *b;
	int num_vert, ret;
	if(argc < 4)
		return 0;
		
	num_vert = atoi(argv[1]);
	b1 = fopen(argv[2], "r");
	b2 = fopen(argv[3], "r");
	out = fopen(argv[4], "w");
	a = malloc(num_vert);
	b = malloc(num_vert);
	
	fread(a, num_vert, 1, b1);
	fread(b, num_vert, 1, b2);
	
	while(!feof(b1) && !feof(b2)) {
		ret = memcmp(a,b,num_vert);
		if(ret > 0) {
			fwrite(b, num_vert, 1, out);
			fread(b, num_vert, 1, b2);
		}
		else if(ret < 0) {
			fwrite(a, num_vert, 1, out);
			fread(a, num_vert, 1, b1);
		}
		else {
			fwrite(b, num_vert, 1, out);
			fread(b, num_vert, 1, b2);
			fread(a, num_vert, 1, b1);
		}
	}
	
	int no_write = 0;

	if(!memcmp(a,b,num_vert))
		no_write = 1;

	if(feof(b1)) {
		fclose(b1);
		if(!no_write)
			fwrite(b, num_vert, 1, out);
		b1 = b2;
	}
	else {
		if(!no_write)
			fwrite(a, num_vert, 1, out);
		fclose(b2);
	}
	
	while(1) {
		fread(a, num_vert, 1, b1);
		if(feof(b1))
			break;
		fwrite(a, num_vert, 1, out);
	}
	
	fclose(b1);
	fclose(out);
	free(a);
	free(b);
	return 0;
}
