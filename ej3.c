#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>

struct arguments{
    int *nInter;
    int *arr1;
    int *arr2;
    int low_int;
    int upp_int;
    int arr2_size;
    pthread_mutex_t* lock_inter;
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


void *Interseccion(void * args)
{
    int ocurrences = 0;
    struct arguments* info = args;
    bool cont = true;

    for (int i = info->low_int; i < info->upp_int; i++){
        int value = info->arr1[i];
        cont = true;
        for (int j = 0; j < info->arr2_size && cont; j++)
        {
            //cont = value > info->arr2[i];
            if (value == info->arr2[j])
            {
                ocurrences++;
                cont = false;
            }
        }
    }

    pthread_mutex_lock(info->lock_inter);
    *info->nInter += ocurrences;
    pthread_mutex_unlock(info->lock_inter);
}

int main (int argc, char **argv)
{
    int n1, n2, num_threads;
    printf("\nIntroduzca el tamaño del primer array: ");
    do {
        scanf("%d", &n1);
    }while (n1 <= 0);

    printf("\nIntroduzca el tamaño del segundo array: ");
    do {
        scanf("%d", &n2);
    }while (n2 <= 0);

    printf("\nIntroduzca el numero de hebras a utilizar: ");
    do {
        scanf("%d", &num_threads);
    }while (num_threads <= 0);


    int *arr1 = malloc(sizeof(int)*n1),
        *arr2 = malloc(sizeof(int)*n2),
        *nInter = malloc(sizeof(int));
    
    for (int i = 0; i < n1; i++)
        arr1[i] = i;

    for (int i = 0; i < n2; i++)
        arr2[i] = 2*i;
    
    pthread_t threads[num_threads];
    struct arguments info[num_threads];
    pthread_mutex_t lockI;
    pthread_mutex_init(&lockI, 0);
    *nInter = 0;

    for (int i = 0; i < num_threads; i++){
        info[i].arr1 = arr1;
        info[i].arr2 = arr2;
        info[i].arr2_size = n2;

        getIntervals(num_threads, n1, i, &info[i].low_int, &info[i].upp_int);
        info[i].nInter = nInter;
        info[i].lock_inter = &lockI;
        pthread_create(&threads[i], 0, Interseccion, (void *) &info[i]);
    }

    for (int i = 0; i < num_threads; i++)
        pthread_join(threads[i], 0);

    int nUnion = (n1 + n2) - *nInter;
    double jaccard = (double)*nInter/nUnion;
    printf("\nParalelo : el coeficiente de Jaccard es %f", jaccard);

    int ocurrences = 0;
    for (int i = 0; i < n1; i++){
        int value = arr1[i];
        for (int j = 0; j < n2; j++)
        {
            if (value == arr2[j])
                ocurrences++;
        }
    }
    int u = (n1 + n2) - ocurrences;
    printf("\nParalelo : el coeficiente de Jaccard es %f\n\n", (double)ocurrences/u);


    pthread_mutex_destroy(&lockI);

    free(arr1);
    free(arr2);
    free(nInter);
}