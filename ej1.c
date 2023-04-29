#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>

#define RAND_MAX 2147483647
#define MIN -100
#define MAX 100
#define NUM_THREADS 4


struct arguments{
    double* arr;
    double* res;
    int id;
    int low_int;
    int upp_int;
};

void getIntervals(size_t num_threads, int arr_size, int id, int *low_int, int *upp_int)
{
    size_t remainder = arr_size % num_threads,
           per_thread = arr_size / num_threads,
           lsize = per_thread + (id < remainder);
    
    *low_int = per_thread*id + (id > remainder) ? remainder : id;
    *upp_int = *low_int + lsize;
}

double double_range(double min, double max)
{
    double random = ((double) rand()) / RAND_MAX,
           range = (max - min) * random,
           number = min + range;

    return number;

}

void *getMax(void *args)
{   
    struct arguments *info = args;
    double max = (*info).arr[(*info).low_int];
    for (unsigned int i = (*info).low_int + 1; i < (*info).upp_int; i++)
        max = (max > (*info).arr[i]) ? max : (*info).arr[i];
    
    (*info).res[(*info).id] = max;
}


int main(int argc, char ** argv)
{
    int num;
    printf ("\nIntroduzca un numero de elementos del vector: ");
    do{
        scanf("%d", &num);
    }while (num < 0);

    srand(time(NULL));

    // Heap is shared between threads of one process so i will make use of it
    double *arr = malloc(sizeof(double) * num),
           *result = malloc(sizeof(double) * NUM_THREADS);
    struct arguments a[NUM_THREADS];
    

    printf("\nEl array creado es : \n");
    for (int i = 0; i < num; i++){
        arr[i] = double_range(MIN, MAX);
        printf("%f   ", arr[i]);
    }


    pthread_t threads[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; i++){
        a[i].arr = arr;
        a[i].res = result;
        a[i].id = i;
        getIntervals(NUM_THREADS, num, i, &a[i].low_int, &a[i].upp_int);
        pthread_create(&threads[i], 0, getMax, (void *) &a[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++)
        pthread_join(threads[i], 0);

    double max = result[0];
    int limit = (num < NUM_THREADS) ? num : NUM_THREADS;

    for (int i = 1; i < limit; i++)
        max = (result[i] > max) ? result[i] : max;

    printf("\n\nEl numero maximo que aparece en el array es %f\n\n", max);

    free(arr);
    free(result);
}