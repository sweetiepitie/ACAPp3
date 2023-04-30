# ACAPp3

Compilation:

	ej1 > gcc ej1.c -lpthread -o ej1
	
	ej2 > gcc ej2.c -lpthread -o ej2
	
	ej3 > gcc ej3.c -lpthread -o ej3
	
	ej4 > mpicc ej4.c -lpthread -o ej4
	
Execution:

	ej1 > ./ej1
	
	ej2 > ./ej2
	
	ej3 > ./ej3
	
	ej4 > mpiexec --oversubscribe -n num_process ej4 num_intervals threads_per_process
