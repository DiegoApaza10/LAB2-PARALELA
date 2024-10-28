#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>

#define NUM_ITER 100000 // Número de iteraciones del ping-pong

int main(int argc, char *argv[]) {
    int rank, size, i;
    double start_wtime, end_wtime;
    clock_t start_clock, end_clock;
    int message = 0;
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size != 2) {
        if (rank == 0) {
            printf("Este programa debe ejecutarse con exactamente dos procesos.\n");
        }
        MPI_Finalize();
        return 1;
    }

    if (rank == 0) {
        printf("Proceso %d: Iniciando ping-pong con %d iteraciones.\n", rank, NUM_ITER);
        
        // Tiempo con MPI_Wtime
        start_wtime = MPI_Wtime();
        
        // Tiempo con clock()
        start_clock = clock();

        for (i = 0; i < NUM_ITER; i++) {
            // Enviar mensaje a proceso 1
            MPI_Send(&message, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
            // Recibir respuesta del proceso 1
            MPI_Recv(&message, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &status);
        }

        // Calcular tiempos finales
        end_wtime = MPI_Wtime();
        end_clock = clock();

        // Mostrar resultados
        printf("Proceso %d: Tiempo transcurrido con MPI_Wtime: %f segundos.\n", rank, end_wtime - start_wtime);
        printf("Proceso %d: Tiempo de CPU con clock: %f segundos.\n", rank, (double)(end_clock - start_clock) / CLOCKS_PER_SEC);
    } else if (rank == 1) {
        for (i = 0; i < NUM_ITER; i++) {
            // Recibir mensaje del proceso 0
            MPI_Recv(&message, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
            // Enviar respuesta al proceso 0
            MPI_Send(&message, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        }
    }

    MPI_Finalize();
    return 0;
}

