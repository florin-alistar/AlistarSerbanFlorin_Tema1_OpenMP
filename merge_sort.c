#include <omp.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

int *nums;

// pentru Merge Sort
int *temp;

int PROBLEM_SIZE = 0;

void printArray(int *nums)
{
    for (int i = 0; i < PROBLEM_SIZE; i++)
    {
        printf("%d\n", nums[i]);
    }
    printf("\n");
}

void generateRandomArray()
{
    nums = (int*)malloc(PROBLEM_SIZE * sizeof(int));
    temp = (int*)malloc(PROBLEM_SIZE * sizeof(int));
    for (int i = 0; i < PROBLEM_SIZE; i++)
    {
        nums[i] = rand();
    }
}

// interclasare nums[0...size/2] cu nums[size/2+1...size]
//    unde size va fi dimensiunea subvectorului la fiecare pas recursiv
// rezultatul se va construi in vectorul temp, care va fi apoi copiat in nums
void merge(int *nums, int size, int *temp)
{
    int i = 0;
    int j = size / 2;
    int pos = 0;
    while (i < size / 2 && j < size)
    {
        if (nums[j] >= nums[i])
        {
            temp[pos++] = nums[i++];
        }
        else
	{
	    temp[pos++] = nums[j++];
	}
    }

    // aducem restul
    while (i < size / 2)
    {
        temp[pos++] = nums[i++];
    }

    while (j < size)
    {
        temp[pos++] = nums[j++];
    }

    memcpy(nums, temp, size * sizeof(int));
}

void mergeSortSerial(int *a, int size, int *temp)
{

    if (size < 2) return;

    if (size == 2)
    {
        if (a[0] <= a[1]) return;
	else
        {
            int aux = a[0]; 
	    a[0] = a[1];
	    a[1] = aux;
            return;
        }
    }

        int half = size / 2;
        mergeSortSerial(a, half, temp);
        mergeSortSerial(a + half, size - half, temp);
        merge(a, size, temp);
}

// V2 din documentatie
void mergeSortParallelWithTasks(int *a, int size, int *temp, int divFactor, int noThreads)
{
    if (divFactor >= noThreads)
    {
        mergeSortSerial(a, size, temp);
        return;
    }

    if (size < 2) return;

#pragma omp task
		{
         		mergeSortParallelWithTasks(a, size / 2, temp, divFactor * 2, noThreads);
		}

#pragma omp task
		{
         		mergeSortParallelWithTasks(a + size / 2 , size - size / 2, temp + size / 2, 
			divFactor * 2, noThreads);
		}
#pragma omp taskwait
	merge(a, size, temp);
}

// V1 din documentatie
void mergeSortParallelWithSections(int *a, int size, int *temp, int divFactor, int noThreads)
{
    if (divFactor >= noThreads)
    {
        mergeSortSerial(a, size, temp);
        return;
    }

    if (size < 2) return;

    omp_set_nested(1);
    omp_set_dynamic(0);

#pragma omp parallel sections num_threads(2)
	{
#pragma omp section
		{
         		mergeSortParallelWithSections(a, size / 2, temp, divFactor * 2, noThreads);
		}

#pragma omp section
		{
         		mergeSortParallelWithSections(a + size / 2 , size - size / 2, temp + size / 2, 
							divFactor * 2, noThreads);
		}
	}
	merge(a, size, temp);
}

void generateFiles()
{
     int lengths[] = {1000000, 2000000, 3000000, 5000000, 7000000, 10000000, 
			30000000};
     for (int i = 0; i < 7; i++)
     {
	 char fileName[50] = {0};
         sprintf(fileName, "numere%d.txt", i);
         FILE *f = fopen(fileName, "w");
         for (int j = 0; j < lengths[i]; j++)
	 {
              int x = rand();
	      fwrite(&x, sizeof(int), 1, f);
	 }
         fclose(f);
     }
}

void readArray(int num)
{
    int lengths[] = {1000000, 2000000, 3000000, 5000000, 7000000, 10000000, 
			30000000};
    nums = (int*)malloc(lengths[num] * sizeof(int));
    temp = (int*)malloc(lengths[num] * sizeof(int));
    char fileName[50] = {0};
    sprintf(fileName, "numere%d.txt", num);
    FILE *f = fopen(fileName, "r");
    for (int j = 0; j < lengths[num]; j++)
    { 
        int x = 0;
        fread(&x, sizeof(int), 1, f);
        nums[j] = x;
    }
    fclose(f);
}

int main(int argc, char **argv)
{
    int workerThreads = atoi(argv[1]);
    int fileNumber = atoi(argv[2]);

    int lengths[] = {1000000, 2000000, 3000000, 5000000, 7000000, 10000000, 
			30000000};
    PROBLEM_SIZE = lengths[fileNumber];
    srand(time(NULL));
   
    //readArray(fileNumber);
    
    generateRandomArray();

    omp_set_num_threads(workerThreads);

    double start, end;
    start = omp_get_wtime();


// OBLIGATORIU ce scrie mai jos
//    daca nu, mergeSortParallelWithTasks va fi apelata de mai multe ori
//	 (deci practic se va face merge sort de nr_threaduri ori)
//    functia cu tasks TREBUIE sa fie in directiva #single (si in #parallel)

// DACA FOLOSIM TAKS-URI, TREBUIE SA AVEM single
//   daca folosim sections, apelam functia de sortare in afara blocului paralel
//	, si nu mai avem nici single, nici parallel

//#pragma omp parallel
    {
        //int id = omp_get_thread_num();
        //printf("thread %d\n", id);
//#pragma omp single
        //mergeSortParallelWithTasks(nums, PROBLEM_SIZE, temp, 1, workerThreads);
    }


     // IN AFARA!
    mergeSortParallelWithSections(nums, PROBLEM_SIZE, temp, 1, workerThreads);


    end = omp_get_wtime();

    //printArray(nums);

    for (int i = 0; i < PROBLEM_SIZE - 1; i++)
    {
	int x = nums[i];
        int y = nums[i + 1];
        if (x > y) { printf("Array-ul nu e ordonat!\n"); return 1;}
    }
    printf("Array-ul este ordonat\n");
    printf("Timp scurs merge sort : %lf sec\n", end - start);


    free(nums);
    free(temp);
}







