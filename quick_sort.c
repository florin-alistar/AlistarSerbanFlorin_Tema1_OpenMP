#include <omp.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

int *nums;

int PROBLEM_SIZE = 0;

// ultimul element ca si pivot
int sequentialPartition(int *nums, int low, int high)
{
    int p = nums[high];
    int pos_bef_piv = low - 1;
    for (int i = low; i < high; i++)
    {
        if (nums[i] <= p)
        {
            pos_bef_piv++;
            int aux = nums[pos_bef_piv];
            nums[pos_bef_piv] = nums[i];
            nums[i] = aux;
        }
    }

    int aux = nums[pos_bef_piv + 1];
    nums[pos_bef_piv + 1] = nums[high];
    nums[high] = aux;
    return pos_bef_piv + 1;
}

int randomPartition(int *nums, int low, int high)
{
    int random = low + rand() % (high - low);
    int aux = nums[random];
    nums[random] = nums[high];
    nums[high] = aux;
    return sequentialPartition(nums, low, high);
}

void sequentialQuicksort(int(*partitionFunc)(int*, int, int), int *nums, int low, int high)
{
    if (low < high)
    {
        int pivot = partitionFunc(nums, low, high);
        sequentialQuicksort(partitionFunc, nums, low, pivot - 1);
        sequentialQuicksort(partitionFunc, nums, pivot + 1, high);
    }
}

void quicksortWithTaskConstruct(int(*partitionFunc)(int*, int, int), int *nums, int low, int high, 
	int divFactor, int noThreads)
{

    // nu mai cream task-uri, ci executam serial quicksort
    // Este mai eficient sa creem task-uri in continuare.
	// Daca am decomenta aici, performanta ar scadea 
	// (dar am evita adancimea prea mare, si am reduce nr task-uri)
    // Comentarea / decomentarea liniilor din if face diferenta intre cele 2 versiuni
	// ale Quick sort despre care am spus in documentatie
    if (divFactor >= noThreads)
    {
         //sequentialQuicksort(partitionFunc, nums, low, high);
         //return;
    }
 
    //printf("th: %d\n", omp_get_thread_num());

    // Partea paralelizata
    if (low < high)
    {
        int pivot = partitionFunc(nums, low, high);

#pragma omp task
	    {
                quicksortWithTaskConstruct(partitionFunc, nums, low, pivot - 1, divFactor * 2, noThreads);
	    }

#pragma omp task
	    {
                quicksortWithTaskConstruct(partitionFunc, nums, pivot + 1, high, divFactor * 2, noThreads);
	    }
    }
}

void printArray(int *nums)
{
    for (int i = 0; i < 50; i++)
    {
        printf("%d\n", nums[i]);
    }
    printf("\n");
}

void generateRandomArray()
{
    nums = (int*)malloc(PROBLEM_SIZE * sizeof(int));
    for (int i = 0; i < PROBLEM_SIZE; i++)
    {
        nums[i] = rand();
    }
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

// Rezultatele din tabele si grafice le-am facut pe baza unor
//    numere din fisiere mari (care erau aceleasi de la rulare la rulare)
// In codul pe care l-am pus in assignment, se genereaza un vector aleatoriu, deci rezultatele
//    s-ar putea sa difere (si intre ele de la rulare la rulare, si fata de ce e in tabele)
void readArray(int num)
{
    int lengths[] = {1000000, 2000000, 3000000, 5000000, 7000000, 10000000, 
			30000000};
    nums = (int*)malloc(lengths[num] * sizeof(int));
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

#pragma omp parallel
    {
        //int id = omp_get_thread_num();
        //printf("thread %d\n", id);
#pragma omp single
        quicksortWithTaskConstruct(sequentialPartition, nums, 0, PROBLEM_SIZE - 1, 
		1, workerThreads);
    }

    end = omp_get_wtime();

    //printArray(nums);

    for (int i = 0; i < PROBLEM_SIZE - 1; i++)
    {
	int x = nums[i];
        int y = nums[i + 1];
        if (x > y) { printf("Array-ul nu e ordonat!\n"); return 1;}
    }
    printf("Array-ul este ordonat\n");
    printf("Timp scurs quicksort : %lf sec\n", end - start);
    free(nums);
}







