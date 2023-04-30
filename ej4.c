#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>

#define MASTER 0
#define TAG 0

struct arguments{
    int l_interval;
    int u_interval;
    int intervals;
    double *part_pi;
    pthread_mutex_t *lock;
};

double get_wall_time(){
	struct timeval t;
	if (gettimeofday(&t, NULL)){
		printf("lol\n");
		return 0;
	}

	return (double)t.tv_sec + (double)t.tv_usec * .000001;
}

void *piRectangles(void* arguments)
{
	struct arguments *info = arguments;
    double width = 1.0/info->intervals;
	double sum = 0.0, x;
	for(int i = info->l_interval; i<info->u_interval; i++){
		x = (i + 0.5)*width;
		sum += 4.0/(1.0 + x*x);
	}

	pthread_mutex_lock(info->lock);
    *info->part_pi += sum*width;
    pthread_mutex_unlock(info->lock);
}

void getIntervals(size_t parallel_ent, int arr_size, int id, int *low_int, int *upp_int)
{
    size_t remainder = arr_size % parallel_ent,
           per_ent = arr_size / parallel_ent,
           lsize = per_ent + (id < remainder);
    
    int mod = (id > remainder) ? remainder : id;
    *low_int = per_ent*id + mod;
    *upp_int = *low_int + lsize;
}

int main(int argc, char **argv)
{
    int size, rank;
    int intervals, num_threads;
    if (argc != 3){
		printf("\n\nUse: mpiexec -n num_processes prog intervals num_threads\n");
        exit(-1);
    }

    intervals = strtol(argv[1], NULL, 10);
    num_threads = strtol(argv[2], NULL, 10);

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int l_limit, u_limit;
    
    if (!rank){
        // Master
        double global_pi = 0;
        double wall0, wall1;

        MPI_Barrier(MPI_COMM_WORLD);
        wall0 = get_wall_time();

        getIntervals(size, intervals, rank, &l_limit, &u_limit);
        
        double *part_pi = malloc(sizeof(double));
        int interval_size = u_limit - l_limit;
        pthread_t threads[num_threads];
        struct arguments info[num_threads];
        pthread_mutex_t lock;
        pthread_mutex_init(&lock, NULL);
        *part_pi = 0;
        
        for (int i = 0; i < num_threads; i++){
            info[i].intervals = intervals;
            info[i].lock = &lock;
            info[i].part_pi = part_pi;
            getIntervals(num_threads, interval_size, i, &info[i].l_interval, &info[i].u_interval);
            pthread_create(&threads[i], NULL, (void *) piRectangles, (void *) &info[i]);
        }

        for (int i = 0; i < num_threads; i++)
            pthread_join(threads[i], 0);

        MPI_Reduce(part_pi, &global_pi, 1, MPI_DOUBLE, MPI_SUM, MASTER, MPI_COMM_WORLD);
        
        wall1 = get_wall_time();

        printf("\n\nPI calculated by rectangles : %f\n", global_pi);
        printf("Wall time consumed : %f\n\n", wall1 - wall0);

    }else{
        // Their bithces
        MPI_Barrier(MPI_COMM_WORLD);

        getIntervals(size, intervals, rank, &l_limit, &u_limit);

        double *part_pi = malloc(sizeof(double));
        int interval_size = u_limit - l_limit;
        pthread_t threads[num_threads];
        struct arguments info[num_threads];
        pthread_mutex_t lock;
        pthread_mutex_init(&lock, NULL);
        *part_pi = 0;
        
        for (int i = 0; i < num_threads; i++){
            info[i].intervals = intervals;
            info[i].lock = &lock;
            info[i].part_pi = part_pi;
            getIntervals(num_threads, interval_size, i, &info[i].l_interval, &info[i].u_interval);
            info[i].l_interval += l_limit;
            info[i].u_interval += l_limit;
            pthread_create(&threads[i], NULL, (void *) piRectangles, (void *) &info[i]);
        }

        for (int i = 0; i < num_threads; i++)
            pthread_join(threads[i], 0);
        
        MPI_Reduce(part_pi, NULL, 1, MPI_DOUBLE, MPI_SUM, MASTER, MPI_COMM_WORLD);
    }

    MPI_Finalize();

    return 0;
}