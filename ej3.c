#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>

struct arguments{
    int *nInter;
    int *arr1;
    int *arr2;
    int low_int1;
    int upp_int1;
    int low_int2;
    int upp_int2;
    pthread_mutex_t* lock_inter;
};



double get_wall_time(){
	struct timeval t;
	if (gettimeofday(&t, NULL)){
		printf("lol\n");
		return 0;
	}

	return (double)t.tv_sec + (double)t.tv_usec * .000001;
}

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

    for (int i = info->low_int1; i < info->upp_int1; i++){
        int value = info->arr1[i];
        for (int j = info->low_int2; j < info->upp_int2 && cont; j++)
        {
            if (value == info->arr2[j])
            {
                ocurrences++;
                cont = false;
            }
        }
        cont = true;
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
    
    int check[num_threads];
    for (int i = 0; i < num_threads; i++)
        check[i] = 0;

    double wall0 = get_wall_time();
    int index = 0;
    for (int i = 0; i < num_threads && index < n2; i++){
        
        getIntervals(num_threads, n1, i, &info[i].low_int1, &info[i].upp_int1);

        if (!i)
            index = info[i].low_int2 = 0;
        else
            index = info[i].low_int2 = info[i-1].upp_int2;
        
            
        while (arr2[index] <= arr1[info[i].upp_int1-1] && index < n2)
            index++;
        
        info[i].upp_int2 = index;


        if (info[i].upp_int2 - info[i].low_int2 > 0){
            info[i].arr1 = arr1;
            info[i].arr2 = arr2;
            info[i].nInter = nInter;
            info[i].lock_inter = &lockI;
            pthread_create(&threads[i], 0, Interseccion, (void *) &info[i]);
            check[i]++;
        }

    }
    
    for (int i = 0; i < num_threads; i++){
        if (check[i])
            pthread_join(threads[i], 0);
    }

    int nUnion = (n1 + n2) - *nInter;
    double jaccard = (double)*nInter/nUnion;

    double wall1 = get_wall_time();
    printf("\nParalelo : el coeficiente de Jaccard es %f, tiempo consumido: %f", jaccard, wall1-wall0);

    wall0 = get_wall_time();
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
    wall1 = get_wall_time();
    printf("\nSecuencial : el coeficiente de Jaccard es %f, tiempo consumido : %f\n\n", (double)ocurrences/u, wall1-wall0);

    pthread_mutex_destroy(&lockI);

    free(arr1);
    free(arr2);
    free(nInter);
}