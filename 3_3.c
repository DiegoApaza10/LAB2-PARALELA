#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    int my_rank, comm_sz;
    int local_value, global_sum;
    int partner;

    // Inicializa MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    // Cada proceso tiene un valor local, para simplificar usamos rank + 1
    local_value = my_rank + 1;
    printf("Proceso %d: valor local = %d\n", my_rank, local_value);

    // PASO 1: Identificar la mayor potencia de dos menor o igual a comm_sz
    int power_of_two = 1;
    while (power_of_two <= comm_sz / 2) {
        power_of_two *= 2;
    }

    // Si este proceso está fuera del rango de la potencia de dos
    if (my_rank >= power_of_two) {
        int target = my_rank - power_of_two;
        printf("Proceso %d: enviando %d a Proceso %d\n", my_rank, local_value, target);
        MPI_Send(&local_value, 1, MPI_INT, target, 0, MPI_COMM_WORLD);
        MPI_Finalize();  // Este proceso termina después de enviar
        return 0;
    }

    // Si este proceso está dentro del rango, podría recibir datos de otros procesos
    if (my_rank < comm_sz - power_of_two) {
        int received_value;
        MPI_Recv(&received_value, 1, MPI_INT, my_rank + power_of_two, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        local_value += received_value;
        printf("Proceso %d: recibió %d de Proceso %d, nuevo valor local = %d\n", 
               my_rank, received_value, my_rank + power_of_two, local_value);
    }

    // PASO 2: Realizar la suma en estructura de árbol con los procesos restantes
    int step = 1;
    while (step < power_of_two) {
        if (my_rank % (2 * step) == 0) {
            partner = my_rank + step;
            int received_value;
            MPI_Recv(&received_value, 1, MPI_INT, partner, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            local_value += received_value;
            printf("Proceso %d: recibió %d de Proceso %d, nuevo valor local = %d\n", 
                   my_rank, received_value, partner, local_value);
        } else if (my_rank % step == 0) {
            partner = my_rank - step;
            printf("Proceso %d: enviando %d a Proceso %d\n", my_rank, local_value, partner);
            MPI_Send(&local_value, 1, MPI_INT, partner, 0, MPI_COMM_WORLD);
            break;  // Este proceso termina después de enviar
        }
        step *= 2;
    }

    // El proceso 0 muestra el resultado final
    if (my_rank == 0) {
        printf("Global sum = %d\n", local_value);
    }

    // Finaliza MPI
    MPI_Finalize();
    return 0;
}

