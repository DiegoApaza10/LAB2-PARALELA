#include <mpi.h>
#include <stdio.h>

int main(int argc, char** argv) {
    int comm_sz, my_rank, partner;
    int local_value, received_value;
    MPI_Init(&argc, &argv);

    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    local_value = my_rank + 1;  // Cada proceso tiene un valor inicial

    printf("Proceso %d: Mi valor inicial es %d\n", my_rank, local_value);

    // Encontramos la siguiente potencia de 2 mayor o igual al número de procesos
    int max_steps = 1;
    while (max_steps < comm_sz) max_steps *= 2;

    // Algoritmo butterfly con mensajes de depuración
    for (int step = 1; step < max_steps; step *= 2) {
        partner = my_rank ^ step;

        if (partner < comm_sz) {
            printf("Proceso %d (Paso %d): Me comunico con proceso %d\n", 
                    my_rank, step, partner);

            MPI_Sendrecv(&local_value, 1, MPI_INT, partner, 0,
                         &received_value, 1, MPI_INT, partner, 0,
                         MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            printf("Proceso %d (Paso %d): Recibí %d de proceso %d\n", 
                    my_rank, step, received_value, partner);

            local_value += received_value;
            printf("Proceso %d (Paso %d): Mi valor ahora es %d\n", 
                    my_rank, step, local_value);
        }
    }

    // Solo el proceso 0 imprime la suma global
    if (my_rank == 0) {
        printf("Proceso %d: La suma global es %d\n", my_rank, local_value);
    }

    MPI_Finalize();
    return 0;
}

