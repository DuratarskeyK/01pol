#include "increase_new.h"

long int poly_num, input_num;

int tid, num_recs, nthreads, buf_i, len;
char dim;

// Массивы для промежуточных вычислений
char need2cmp; // Флаг для выполнения сравнений в permutation()
sint num_vert;
sint *repres_optimum; // Текущий оптимум
sint *optimum; // Текущий оптимум c числом вершин в первом байте
sint *cur_rang_vertices; // Содержит вершины текущего ранга
sint **rang_table;	// Буферный массив для упорядочения вершин
sint ***recursive_sort_array; // Размеры [dim+1][2][num_vert] 

int array_alloc(char dim, sint max_num_vertices)
{
//	int max_num_vertices = 1 << dim;

	// Выделяем память для промежуточных вычислений
	optimum = (sint*)malloc(sizeof(sint)*(max_num_vertices+1));
	if (optimum == NULL) return 1;
	repres_optimum = optimum + 1;

	cur_rang_vertices = (sint*)malloc(sizeof(sint)*max_num_vertices);
	if (cur_rang_vertices == NULL) return 1;

	rang_table = (sint**)malloc(sizeof(sint*)*(CHAR_BIT+1));
	if (rang_table == NULL) return 1;
	long int i, j;
	for (i = 0; i <= CHAR_BIT; i++)
	{
		rang_table[i] = (sint*)malloc(sizeof(sint)*max_num_vertices);
		if (rang_table[i] == NULL) return 1;
	}

	recursive_sort_array = (sint***)malloc(sizeof(sint**)*(dim+1));
	if (recursive_sort_array == NULL) return 1;
	for (i = 0; i <= dim; i++)
	{
		recursive_sort_array[i] = (sint**)malloc(sizeof(sint*)*2);
		if (recursive_sort_array[i] == NULL) return 1;
		recursive_sort_array[i][0] = (sint*)malloc(sizeof(sint)*max_num_vertices);
		recursive_sort_array[i][1] = (sint*)malloc(sizeof(sint)*max_num_vertices);
		if (recursive_sort_array[i][0] == NULL || recursive_sort_array[i][1] == NULL) 
			return 1;
	}

	return 0;
}

// Освобождаем память
int array_free(char dim)
{
	free (optimum);
	free (cur_rang_vertices);
	long int i;
	for (i = 0; i <= CHAR_BIT; i++)
		free(rang_table[i]);
	free (rang_table);
	
	for (i = 0; i <= dim; i++)
	{
		free(recursive_sort_array[i][0]);
		free(recursive_sort_array[i][1]);
		free(recursive_sort_array[i]);
	}
	free (recursive_sort_array);

	return 0;
}

// Рекурсивная процедура по поиску оптимальной перестановки координат
// dim - размерность, num_vert - число вершин
// rest - число неотсортированных вершин
// sorted_b - число отсортированных координат
// repres_optimum - массив для представителя
int permutation(char dim, sint rest, char sorted_b)
{
	sint **tosort = recursive_sort_array[sorted_b];
	// tosort - массив из двух строк, 
	// tosort[0] - список вершин, tosort[1] - число единичек
	sint ones_list;
	sint maska;
	sint rang = tosort[1][0];
	// rang - число неотсортированных единичек
/*	maska = ((1 << rang) - 1) << sorted_b;
	if (need2cmp && repres_optimum[num_vert-rest] < maska )
		return 0; // Заведомо неоптимальный вход
*/
	maska = (1 << sorted_b) - 1;
	// maska содержит единички для первых sorted_b бит
/*	if (rest == 1) // Сортировать нечего
	{
		ones_list = (1 << rang) - 1;
		// Прижимаем все tosort[1][0] единичек вправо
		ones_list = (tosort[0][0])&maska | (ones_list << sorted_b);
		if (repres_optimum[num_vert-1] > ones_list || !need2cmp)
			repres_optimum[num_vert-1] = ones_list;
		need2cmp = 1; // Достигнут локальный оптимум
		return 0;
	}
*/
	sint j;


	ones_list = tosort[0][0];
	// ones_list - расположение неотсортированных единичек
	sint maskb = ones_list&maska;
	// Перебираем одноранговые вершины
	for (j = 1; j < rest; j++)
	{
		if (tosort[1][j] != rang || ((tosort[0][j])&maska) != maskb)
			break;
		ones_list |= tosort[0][j];
	}
	// j - число вершин с минимальным рангом

	// Массив со следующим шагом сортировки
	sint **new_sort = recursive_sort_array[sorted_b+1];
	sint q1, q2, shift, l;
	q1 = 1 << sorted_b;
	q2 = q1;

	for (shift = 0; shift < dim - sorted_b; shift++, q2 <<= 1)
	{
		if (!(ones_list&q2)) // Переставлять нечего
			continue;

		sint new_rest = rest;
		sint q3 = 255 - (q1|q2); // Два нуля в массиве из единиц
		sint v, new_v, new_rang;
		sint cur_rang = 0, beg_block = 0, end_block = 0;
		//sint cut_branch = 0;

		for (j = 0; j < rest; j++) 
		{
			v = tosort[0][j];
			new_v = (v & q3) | (((v & q2) >> shift) | ((v & q1) << shift));
			// Вычисляем новый ранг вершины
			new_rang = tosort[1][j] - ((new_v >> sorted_b) & 1);

			if (new_rang == 0)
			{// Добавляем в repres_optimum отсортированную вершину
				if (repres_optimum[num_vert-new_rest] > new_v || !need2cmp)
				{ // Обновляем оптимум
					repres_optimum[num_vert-new_rest] = new_v;
					need2cmp = 0; // В этой ветке сравнения не потребуются
				}
				else
				{
					if (repres_optimum[num_vert-new_rest] < new_v)
//					{// Текущее значение хуже оптимума
//						cut_branch = 1;
						break;
//					}
				}
				new_rest--;
			}
			else
			{
				if (new_rang >= cur_rang)
				{
					if (new_rang > cur_rang) beg_block = end_block;
					new_sort[0][end_block] = new_v;
					new_sort[1][end_block] = new_rang;
					cur_rang = new_rang; 
				}
				else	// if (new_rang < cur_rang)
				{
					l = end_block;
					new_sort[1][l] = cur_rang;
					for ( ; l > beg_block; l--) 
						new_sort[0][l] = new_sort[0][l-1];
					new_sort[0][beg_block] = new_v;
					new_sort[1][beg_block] = new_rang;
					beg_block++;
					// cur_rang не меняем
				}
				end_block++;
			}
		}

		if (j != rest) continue;

//		if (rang1 != new_rest)
//			{printf("ERROR: rang1 != new_rest\n"); getchar(); return 2;}

		if (new_rest > 0)
		{
			// Проверка 1
			maska = ((1 << new_sort[1][0]) - 1) << (sorted_b+1);
			if (need2cmp && repres_optimum[num_vert-new_rest] < maska )
				continue; // Заведомо неоптимальный вход

			// Проверка 2
			if (new_rest == 1) // Сортировать нечего
			{
				maskb = (1 << new_sort[1][0]) - 1;
				// Прижимаем все tosort[1][0] единичек вправо
				maska = (1 << (sorted_b+1)) - 1;
				maskb = ((new_sort[0][0])&maska) | (maskb << (sorted_b+1));
				if (repres_optimum[num_vert-1] > maskb || !need2cmp)
					repres_optimum[num_vert-1] = maskb;
				need2cmp = 1; // Достигнут локальный оптимум
				continue;
			}

			// Все проверки пройдены
			permutation(dim, new_rest, sorted_b + 1);
		}
		else
			need2cmp = 1; // Достигнут локальный оптимум
	}
	return 0;
}

// Для массива вершин vertices находим 
// лексикографически минимального представителя
// dim - размерность
// Первый байт представителя содержит число вершин 
// Второй байт - начало координат (всегда 0)
int representative(char dim, sint *vertices)
{
	optimum[1] = 0; // Первая вершина всегда 0

	if (num_vert < 2) 
	{
		if (num_vert < 0)
		{
			printf("ERROR: num_vert < 0\n"); 
			getchar(); 
			num_vert = 0;
		}
		return 0;
	}

//	optimum[0] = num_vert; // Число вершин

	sint **sort = recursive_sort_array[0];   
	// Основной массив: 
	// sort[0][i] - координаты i-й вершины 
	// sort[1][i] - число единичек

	sint i, j;
	sint v, q, rang;
	sint mask;
	need2cmp = 0;   // В начале сравнивать не с чем
	for (i = 0; i < num_vert; i++)
	{
		mask = vertices[i];
		
		for (j = 0; j <= dim; j++) // Обнуляем сортировочный массив
			rang_table[j][0] = 0;
		
		for (j = 0; j < num_vert; j++) // Переключение с помощью mask
		{
			v = (vertices[j])^mask;
			// Подсчитываем число единичек в v
			for (q = v, rang = 0; q > 0; q >>= 1)
				rang += q & 1;
			// Добавляем в соответствующую строку ранговой таблицы
			rang_table[rang][0] += 1;
			rang_table[rang][rang_table[rang][0]] = v;
		}

		// Собираем строки ранговой таблицы в массив sort,
		// rang_table[0] содержит начало координат
		int l, k = 0;
		for (j = 1; j <= dim; j++)
		{
			for (l = 1; l <= rang_table[j][0]; l++)
			{
				sort[1][k] = j;
				sort[0][k] = rang_table[j][l];
				k++;
			}
		}

		if (k!=num_vert-1) 
		{// В списке присутствуют дубликаты
			printf("ERROR: k!=num_vert-1\n"); 
			printf ("%d", rang_table[0][0]);
			for (j = 1; j <= dim; j++)
				printf (" + %d", rang_table[j][0]);
			printf (" != %d\n", num_vert);
			return 1;
		}

		if ( need2cmp && (repres_optimum[num_vert-k] < (1 << sort[1][0]) - 1) )
			continue; // Заведомо неоптимальный вход
		// Находим оптимальную битовую перестановку 
		permutation(dim, k, 0);

	}
//	optimum[1] = 0; // Первая вершина всегда 0
	return 0;
}

int cmpbuff(const void *a, const void *b) {
	return memcmp(a, b, num_vert);
}

void increase ()
{
	int i, j, k, l = 1, s, cnt = 0, b, op, size, tt;
	long long total, ub;
	MPI_Status status;
	struct stat file_info;
	int *ones;
	char name[20];
	sint *buffer, *rcv, vert[30];
	FILE *out;
	char *vert_flag;
	
	array_alloc(dim, num_vert+1);	

	init_adj(num_vert+1);
	return;
	buffer = (sint *)malloc((num_vert+1)*1ULL*BUFF_SIZE);
	rcv = (sint *)malloc(num_vert*RCV_SIZE);
	ones = (int *)malloc(dim*sizeof(int));
	vert_flag = (char *)calloc(1 << dim, 1);

	k = 0;
	
	num_vert++;
	
	while(1) {
		MPI_Send(&tid, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
		MPI_Recv(&op, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
		if(op == 2) {
			sleep(80);
			continue;
		}
		else if(op == 1)
			break;

		MPI_Recv(&size, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
		MPI_Recv(rcv, size*(num_vert-1), MPI_BYTE, 0, 0, MPI_COMM_WORLD, &status);

		for(tt = 0; tt<size; tt++) {
			memcpy(vert, rcv + tt*(num_vert-1), num_vert-1);
			cnt++;
			for(i = 0; i<dim; i++)
				ones[i] = 0;
	
			for (i = 0, j = 0; i < (1 << dim); i++)
			{
				vert_flag[i] = 1;
				if (vert[j] == i)
				{
					vert_flag[i] = 0;

					for(s = 0; s<dim; s++)
						ones[s] += (i>>s)&1;
					
					if(j < num_vert-2)
						j++;
				}
			}

			for (i = 0; i < (1 << dim); i++)
			{
				if (vert_flag[i])
				{
					vert[num_vert-1] = i;
					int is_write = 1;

					for(s = 0; s<dim; s++) {
						b = ones[s] + ((i<<s)&1);
						if(b > vert_limit[dim-1] || num_vert-b > vert_limit[dim-1]) {
							is_write = 0;
							break;
						}
					}
					if (is_write) 
						is_write = check2neighb_fast (vert, num_vert, dim);
					
					if (is_write && check2neighb_inc (vert, num_vert, dim))
					{
						representative(dim, vert);
						memcpy(buffer+k, optimum+1, num_vert);
						k+=num_vert;

						if(k / num_vert == BUFF_SIZE) {
							//dump
							qsort(buffer, BUFF_SIZE, num_vert, cmpbuff);
							sprintf(name, "buff%d_%d", tid, l);
							out = fopen(name, "w");
							for(s = 0; s<k; ) {
								fwrite(buffer+s, num_vert, 1, out);
								while(!memcmp(buffer+s, buffer+s+num_vert,num_vert))
									s+=num_vert;
								s+=num_vert;
							}
							fclose(out);
							k = 0;
							l++;
						}
					}
				}
			}
		}
	}

	qsort(buffer, k/num_vert, num_vert, cmpbuff);
	sprintf(name, "buff%d_%d", tid, l);
	out = fopen(name, "w");
	for(s = 0; s<k; ) {
		fwrite(buffer+s, num_vert, 1, out);
		while(!memcmp(buffer+s, buffer+s+num_vert,num_vert))
			s+=num_vert;
		s+=num_vert;
	}
	fclose(out);
	free(buffer);
	free(rcv);
	free_adj();

	array_free(dim);
}

//////////////
//
//    MAIN
//
/////////////

// вход программы - исходное число вершин num_vert
// выход программы - многогранники на num_vert+1 вершинах

int main(int argc, char *argv[])
{
	MPI_Init(&argc, &argv);

	if (argc < 4) 
		{printf ("Wrong number of parameters\n"); return 0;}

	MPI_Comm_rank(MPI_COMM_WORLD, &tid);
	MPI_Comm_size(MPI_COMM_WORLD, &nthreads);
	
	num_vert = atoi(argv[2]);
	dim = atoi(argv[3]);

	if(!tid) {
		int num, elem, op, cnt = nthreads-1;
		long long total = 0;
		char tt[100];
		sint *rcv;
		FILE *in;
		MPI_Status status;
		gethostname(tt, 80);
		printf("%s\n", tt);
		in = fopen(argv[1], "r");
		rcv = (sint *)malloc(num_vert*RCV_SIZE);

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
		free(rcv);
		fclose(in);
	}
	else {
		increase();
	}
	MPI_Finalize();
	return 0;
}
