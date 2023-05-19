#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define WIDTH 800
#define HEIGHT 800
#define MAX_ITER 1000

int main(int argc, char **argv) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    double start_time, end_time;

    if (rank == 0) {
        start_time = MPI_Wtime();
    }

    // Calculate the range of the real and imaginary axis
    double real_min = -2.0;
    double real_max = 2.0;
    double imag_min = -2.0;
    double imag_max = 2.0;

    // Calculate the range of pixels assigned to this rank
    int pixels_per_rank = WIDTH * HEIGHT / size;
    int start_pixel = rank * pixels_per_rank;
    int end_pixel = start_pixel + pixels_per_rank;

    // Allocate memory for the image
    int *image = (int *)malloc(pixels_per_rank * sizeof(int));

    // Calculate the Mandelbrot set for the assigned pixels
    for (int i = start_pixel; i < end_pixel; i++) {
        int x = i % WIDTH;
        int y = i / WIDTH;

        double real = real_min + (real_max - real_min) * x / WIDTH;
        double imag = imag_min + (imag_max - imag_min) * y / HEIGHT;

        double z_real = real;
        double z_imag = imag;

        int iter;
        for (iter = 0; iter < MAX_ITER; iter++) {
            double z_real_squared = z_real * z_real;
            double z_imag_squared = z_imag * z_imag;

            if (z_real_squared + z_imag_squared > 4.0) {
                break;
            }

            double z_real_new = z_real_squared - z_imag_squared + real;
            double z_imag_new = 2.0 * z_real * z_imag + imag;

            z_real = z_real_new;
            z_imag = z_imag_new;
        }

        image[i - start_pixel] = iter;
    }

    // Gather the images from all ranks and combine them into a single image
    int *global_image = NULL;
    if (rank == 0) {
        global_image = (int *)malloc(WIDTH * HEIGHT * sizeof(int));
    }

    MPI_Gather(image, pixels_per_rank, MPI_INT, global_image, pixels_per_rank, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        end_time = MPI_Wtime();
        printf("Execution time: %f seconds\n", end_time - start_time);

        // Write the image to a file
        FILE *fp;
        fp = fopen("/shared/mandelbrot_static.pgm", "wb");
        fprintf(fp, "P5\n%d %d\n%d\n", WIDTH, HEIGHT, MAX_ITER - 1);
        fwrite(global_image, sizeof(int), WIDTH * HEIGHT, fp);
        fclose(fp);

        free(global_image);
    }

    free(image);

    MPI_Finalize();
    return 0;
}
