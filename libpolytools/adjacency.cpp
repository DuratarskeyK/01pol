#include <polytool.h>
#include <stdlib.h>
#include "ClpSimplex.hpp"
#include "CoinTime.hpp"
#include "CoinModel.hpp"

typedef unsigned char sint;
static sint vert_limit[11] = {0, 2, 3, 4, 6, 10, 13, 20, 30, 40, 50};

static unsigned long int *mask_array; // Ускоряет работу фильтра 2-neighborly
static sint neighb_array[256];
static int rowIndex[16][128];
static double rowValue[16][128];
static ClpSimplex modelSample;

int alloc_flag(int dim, int num_vert) {
	mask_array = (unsigned long int *)calloc((num_vert*(num_vert+1))/2, sizeof(unsigned long int));
	if (mask_array == NULL) return 1;
	return 0;
}

extern "C" void init_adj(int num_vert) {
	int i,j;

	alloc_flag(8, 22);

	for(i = 0; i<16; i++)
		for(j = 0; j<128; j++)
			rowValue[i][j] = 1.0;

	modelSample.allSlackBasis();
	// Switch off messages
	modelSample.setLogLevel(0); 
	// Create space for ncols columns
	resize_model(num_vert);
}

extern "C" void resize_model(int num_vert) {
	if(num_vert <= 2)
		return;

	modelSample.resize(0, num_vert-2);
	// Fill in

	modelSample.setObjectiveCoefficient(0, 1.0);
	for (int i = 0; i < num_vert-2; i++) 
	{
		modelSample.setColumnUpper(i, COIN_DBL_MAX);
	}
}

// Число единичек в битовом представлении m
inline int bitnum (sint m, int dim) {
	int cnt = 0, i;
	for(i = 0; i<dim; i++)
		cnt+=(m>>i)&1;

	return cnt;
}

int alfa(int ncols, int dim, int nrows) {
		// Empty model
		ClpSimplex  model = modelSample;
		// Now use model
		CoinModel modelObject;
		int i;
		sint m = 1 << (dim-1);
		for (i = 0; m; m >>= 1) 
		{
			if (m & neighb_array[1])
			{
				int n_one = 0;
				for (int j = 0; j < ncols; j++)
					if (m & neighb_array[j+2])
					{
						rowIndex[i][n_one] = j;
						rowValue[i][n_one] = 1.0;
						n_one++;
					}
				if (n_one == 0 || n_one == ncols) 
					return 0;
				modelObject.addRow(n_one, rowIndex[i], rowValue[i], 1.0, 1.0);
				i++;
			}
		}

		model.addRows(modelObject);
		if (model.dual() == 1)
			return 0;
		return 1;
}

extern "C" int check2neighb_inc(vertex *tested, int numv, int dim) {
	int j = numv - 1; 
	for (int i = 0; i < numv-1; i++) 
	{
		neighb_array[0] = 0;//tested[i];
		neighb_array[1] = tested[j]^tested[i];
		sint m = ~(neighb_array[1]) & ((1 << dim) - 1);
		int diff = dim - bitnum (m, dim);
		int p;

		if (diff > 2)
		{
			p = 2;
			sint tb;
			for (int k = 0; k < numv; k++)
			{
				tb = tested[i]^tested[k];
				if ( (tb & m) == 0 && k != i && k != j)
						neighb_array[p++] = tb;
			}
			if (p > 4)
			{
				if (p > vert_limit[diff])
					return 0;
				if ( alfa (p-2, dim, diff) )
					return 0;
			}
		}
	}

	for (int i = 0; i < numv-2; i++) 
	{
		for (j = i+1; j < numv-1; j++) 
		{
			neighb_array[0] = 0;//tested[i];
			neighb_array[1] = tested[j]^tested[i];
			sint m = ~(neighb_array[1]) & ((1 << dim) - 1);
			int diff = dim - bitnum (m, dim);
			int p;

			if (diff > 2)
			{
				int is_test = 0;
				p = 2;
				sint tb;
				for (int k = 0; k < numv; k++)
				{
					tb = tested[i]^tested[k];
					if ( (tb & m) == 0 && k != i && k != j)
					{
						neighb_array[p++] = tb;
						if (k == numv-1)
							is_test = 1;
					}
				}
				if (p > 4)
				{
					if (p > vert_limit[diff])
						return 0;
					if (is_test)
					{	
						if ( alfa (p-2, dim, diff) )
							return 0;
					}
				}
			}
		}	
	}
	return 1;
}

extern "C" void free_adj() {
	free (mask_array);
}

void exchange (int i, int j) {
	unsigned long int buf_m = mask_array[i];
	mask_array[i] = mask_array[j];
	mask_array[j] = buf_m;
}

int is_cmpexchange (int i, int j) {
	if (mask_array[i] > mask_array[j]) exchange (i, j);
	return (mask_array[j] - mask_array[i] == 0);
}

int is_sort3 (int l, int r) {
	if (r-l <= 0) return 0;

	if (is_cmpexchange (l, l+1)) return 1;
	if (r-l == 2)
	{
		if (is_cmpexchange (l, r)) return 1;
		if (is_cmpexchange (l+1, r)) return 1;
	}
	return 0;
}

int is_coincide_mask (int l, int r) {
	if (r-l < 3) {return is_sort3 (l, r);}
	int j;
	exchange ((l+r)/2, r-1);
	if (is_cmpexchange (l, r)) return 1;
	if (is_cmpexchange (l, r-1)) return 1;
	if (is_cmpexchange (r, r-1)) return 1;
	// Теперь медиана в mask_array[r]
	unsigned long int b = mask_array[r];
	j = r;
	int i = l-1;
	for (;;)
	{
		while (mask_array[++i] < b) ;
		if (mask_array[i] == b) return 1;
		while (mask_array[--j] > b) 
			if (j <= i) break;
		if (j <= i) break;
		if (mask_array[j] == b) return 1;
		exchange (i, j);
	}
	exchange (r, i);
	if (is_coincide_mask (l, i-1)) return 1;
	if (is_coincide_mask (i+1, r)) return 1;
	return 0;
}

extern "C" int check2neighb_fast(vertex *tested, int numv, int dim) {
	int len = 0, i, j;
	for (i = 0; i < numv-1; i++)
		for (j = i+1; j < numv; j++)
		{
			unsigned long int b = ~(tested[i]^tested[j]) & ((1 << dim) - 1);
			mask_array[len++] = (b << (8)) + (b & tested[i]);
		}
	if (is_coincide_mask (0, len-1)) return 0;

	return 1;
}

