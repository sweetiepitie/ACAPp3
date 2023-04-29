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
    pthread_mutex_t *lock;
};

void getIntervals(size_t num_threads, int arr_size, int id, int *low_int, int *upp_int)
{
    size_t remainder = arr_size % num_threads,
           per_thread = arr_size / num_threads,
           lsize = per_thread + (id < remainder);
    
    int mod = (id > remainder) ? remainder : id;
    *low_int = per_thread*id + mod;
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
    double max = info->arr[info->low_int];
    for (unsigned int i = info->low_int + 1; i < info->upp_int; i++)
        max = (max > info->arr[i]) ? max : info->arr[i];
    
    pthread_mutex_lock(info->lock);
    *info->res = (*info->res > max) ? *info->res : max;
    pthread_mutex_unlock(info->lock);
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
           *result = malloc(sizeof(double));
    struct arguments info[NUM_THREADS];
    

    printf("\nEl array creado es : \n");
    for (int i = 0; i < num; i++){
        arr[i] = double_range(MIN, MAX);
        printf("%f   ", arr[i]);
    }


    pthread_t threads[NUM_THREADS];
    pthread_mutex_t lock;
    pthread_mutex_init(&lock, 0);
    *result = MIN;
    for (int i = 0; i < NUM_THREADS; i++){
        info[i].arr = arr;
        info[i].res = result;
        info[i].id = i;
        info[i].lock = &lock;
        getIntervals(NUM_THREADS, num, i, &info[i].low_int, &info[i].upp_int);
        pthread_create(&threads[i], 0, getMax, (void *) &info[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++)
        pthread_join(threads[i], 0);

    printf("\n\nResultado empleando parallelismo : numero maximo es %f\n", *result);

    double max = arr[0];
    for (int i = 1; i < num; i++)
        max = (arr[i] > max) ? arr[i] : max;

    printf("Resultado version secuencial : numero maximo es %f\n\n", max);
    
    pthread_mutex_destroy(&lock);
    free(arr);
    free(result);
}