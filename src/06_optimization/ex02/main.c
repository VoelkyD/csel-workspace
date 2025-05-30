#include <stdio.h>
#include <stdlib.h>

#define SIZE 65536

static int compare (const void* a, const void* b)
{
    return *(short*)a - *(short*)b;
}

int main()
{
 	// generate data
	short data[SIZE];
	for (int i = 0; i < SIZE; i++) {
		data[i] = rand() % 512;
	}

	qsort(data, SIZE, sizeof(data[0]), compare);

	long long sum = 0;
	for (int j = 0; j < 10000; j++) {
		for (int i = 0; i < SIZE; i++) {
			if (data[i] >= 256) {
				sum += data[i];
			}
		}
	}
	printf ("sum=%lld\n", sum);
}
