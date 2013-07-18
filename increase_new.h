#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <mpi.h>

#define BUFF_SIZE 90000000
#define RCV_SIZE 350
typedef unsigned char sint;
const char CHAR_BIT = 8; // Число бит в байте

sint vert_limit[11] = {0, 2, 3, 4, 6, 10, 13, 20, 30, 40, 50};
