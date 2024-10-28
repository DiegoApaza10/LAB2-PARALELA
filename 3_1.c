#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

void find_bin(float value, float min_meas, int bin_count, float bin_width, int *bin) {
    *bin = (int)((value - min_meas) / bin_width);
    if (*bin >= bin_count) *bin = bin_count - 1;
}

void compute_local_histogram(float *local_data, int local_data_count, 
                             float min_meas, int bin_count, float bin_width, int *local_bin_counts) {
    for (int i = 0; i < bin_count; i++) local_bin_counts[i] = 0;

    for (int i = 0; i < local_data_count; i++) {
        int bin;
        find_bin(local_data[i], min_meas, bin_count, bin_width, &bin);
        local_bin_counts[bin]++;
    }
}

int main(int argc, char *argv[]) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    float min_meas = 0.0, max_meas = 5.0;
    int bin_count = 5;
    float bin_width = (max_meas - min_meas) / bin_count;

    int data_count;
    float *data = NULL;

    if (rank == 0) {
        printf("Proceso 0: Generando datos iniciales.\n");
        data_count = 20;
        data = (float *)malloc(data_count * sizeof(float));
        float sample_data[20] = {1.3, 2.9, 0.4, 0.3, 1.3, 4.4, 1.7, 0.4, 
                                 3.2, 0.3, 4.9, 2.4, 3.1, 4.4, 3.9, 0.4, 
                                 4.2, 4.5, 4.9, 0.9};
        for (int i = 0; i < data_count; i++) {
            data[i] = sample_data[i];
        }
        printf("Proceso 0: Datos generados.\n");
    }

    printf("Proceso %d: Esperando para recibir el número total de datos.\n", rank);
    MPI_Bcast(&data_count, 1, MPI_INT, 0, MPI_COMM_WORLD);
    printf("Proceso %d: Número total de datos recibido: %d.\n", rank, data_count);

    int local_data_count = data_count / size;
    float *local_data = (float *)malloc(local_data_count * sizeof(float));
    MPI_Scatter(data, local_data_count, MPI_FLOAT, local_data, 
                local_data_count, MPI_FLOAT, 0, MPI_COMM_WORLD);
    printf("Proceso %d: Recibidos %d datos locales.\n", rank, local_data_count);

    int *local_bin_counts = (int *)malloc(bin_count * sizeof(int));
    printf("Proceso %d: Calculando el histograma local.\n", rank);
    compute_local_histogram(local_data, local_data_count, min_meas, 
                            bin_count, bin_width, local_bin_counts);

    printf("Proceso %d: Histograma local calculado:\n", rank);
    for (int i = 0; i < bin_count; i++) {
        printf("Proceso %d: Bin %d: %d\n", rank, i, local_bin_counts[i]);
    }

    int *global_bin_counts = NULL;
    if (rank == 0) {
        global_bin_counts = (int *)malloc(bin_count * sizeof(int));
    }

    printf("Proceso %d: Comenzando la reducción para el histograma global.\n", rank);
    MPI_Reduce(local_bin_counts, global_bin_counts, bin_count, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("Proceso 0: Histograma global final:\n");
        for (int i = 0; i < bin_count; i++) {
            printf("Bin %d: %d\n", i, global_bin_counts[i]);
        }
        free(global_bin_counts);
        free(data);
    }

    printf("Proceso %d: Liberando memoria y finalizando.\n", rank);
    free(local_data);
    free(local_bin_counts);

    MPI_Finalize();
    printf("Proceso %d: MPI finalizado.\n", rank);
    return 0;
}

