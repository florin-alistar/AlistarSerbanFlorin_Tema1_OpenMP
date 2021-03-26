#include <stdlib.h>
#include <stdio.h>
#include <omp.h>
#include <time.h>

int arrayMax(int *arr, int n)
{
    int x = arr[0];
    for (int i = 1; i < n; i++)
    {
        if (arr[i] > x)
        {
            x = arr[i];
        }
    }
    return x;
}

// Pe vectori de dimensiuni mici (cum ar fi 10000) se obtine speedup bun prin paralelizare
void radixSort(int size, int* data, int numThreads)
{
    int maxArr = arrayMax(data, size);

    for (int exp = 1; maxArr / exp > 0; exp *= 10)
    {
        int *output = (int*)malloc(size * sizeof(int));
        int count[10] = { 0 };

        int localCount[10] = { 0 };

#pragma omp parallel firstprivate(localCount) num_threads(numThreads)
        {
             //printf("%d\n", omp_get_thread_num());
            // Paralelizare for -> fiecare thread scrie intr-un vector propriu
#pragma omp for schedule(static) nowait
            for (int numberIndex = 0; numberIndex < size; numberIndex++)
            {
                localCount[(data[numberIndex] / exp) % 10]++;
            }

// Un singur thread poate executa asta la un moment dat (dar toate o vor executa)
#pragma omp critical
            for (int i = 0; i < 10; i++)
            {
                count[i] += localCount[i];
            }

#pragma omp barrier

// Un singur thread executa acest for SI NUMAI UNUL (histograma)
#pragma omp single
            for (int bucketIndex = 1; bucketIndex < 10; bucketIndex++)
            {
                count[bucketIndex] += count[bucketIndex - 1];
            }

#pragma omp barrier

// Partea aceasta nu stiu cum s-ar paraleliza...
//     pare ca ar trebui sa fie o singura modificare in output si count la un moment dat
//	 si asta ar insemna secvential
#pragma omp single
            for (int numberIndex = size - 1; numberIndex >= 0; numberIndex--)
            {
                output[count[(data[numberIndex] / exp) % 10] - 1] = data[numberIndex];
                count[(data[numberIndex] / exp) % 10]--;
            }

#pragma omp barrier
#pragma omp parallel for
            for (int i = 0; i < size; i++)
            {
                data[i] = output[i];
            }
        }
#pragma omp barrier
#pragma omp single
        free(output);
    }
}

// daca se modifica SIZE, trebuie comentat for-ul ultim din main
#define SIZE 10000
int vec[SIZE];
int copy[SIZE];

int main(int argc, char **argv)
{
    int NO_THREADS = atoi(argv[1]);
    srand(time(NULL));
    for (int i = 0; i < SIZE; i++)
    {
        vec[i] = rand();
        copy[i] = vec[i];
        //printf("%d\n", vec[i]);
    }

    printf("\n");
    double start = omp_get_wtime();
    radixSort(SIZE, vec, NO_THREADS);
    /*for (int i = 0; i < SIZE; i++)
    {
        printf("%d\n", vec[i]);
    }*/
    printf("\n");

    double end = omp_get_wtime();
    printf("timp: %lf\n", end - start);
    printf("\n");

    for (int i = 1; i < SIZE; i++)
    {
        if (vec[i] < vec[i - 1])
        {
            printf("Vectorul nu este ordonat!\n");
            return 1;
        }
    }

    printf("Vectorul e ordonat...\n");

    // for-ul asta ar trebui comentat daca SIZE e prea mare (1.000.000 sau pe acolo...)
    for (int i = 0; i < SIZE; i++)
    {
        int x = vec[i];
        int found = 0;
        for (int j = 0; j < SIZE; j++)
        {
            if (x == copy[j])
            {
                found = 1;
                copy[j] = -1;
                break;
            }
        }
        if (!found)
        {
            printf("Vectorul rezultat nu e acelasi cu cel original!\n");
            return 1;
        }
    }

    printf("Vectorul rezultat contine toate elem orig\n");

    return 0;
}
