#include <stdio.h>

#include <stdlib.h>

#include <math.h>

#include <mpi.h>

 

#define WIDTH 800

#define HEIGHT 600

#define MAX_ITER 1000

 

int main(int argc, char** argv) {

  MPI_Init(&argc, &argv);

 

  int rank, size;

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  MPI_Comm_size(MPI_COMM_WORLD, &size);

 

  int rows_per_process = HEIGHT / size;

 

  double computation_start_time = MPI_Wtime();

 

  // Compute the Mandelbrot set for each row

  int* image = (int*) malloc(WIDTH * rows_per_process * sizeof(int));

 

  for (int y = 0; y < rows_per_process; y++) {

    for (int x = 0; x < WIDTH; x++) {

      double c_re = (x - WIDTH/2.0)*4.0/WIDTH;

      double c_im = (y + rank*rows_per_process - HEIGHT/2.0)*4.0/HEIGHT;

 

      double z_re = c_re, z_im = c_im;

      int i;

      for (i = 0; i < MAX_ITER; i++) {

        if (z_re*z_re + z_im*z_im > 4) break;

 

        double new_re = z_re*z_re - z_im*z_im;

        double new_im = 2*z_re*z_im;

 

        z_re = c_re + new_re;

        z_im = c_im + new_im;

      }

 

      image[y*WIDTH + x] = i;

    }

  }

 

  double computation_end_time = MPI_Wtime();

 

  // Gather the results from all processes

  double communication_start_time = MPI_Wtime();

 
int* recv_buffer = (int*) malloc(WIDTH * HEIGHT * sizeof(int));
  if (rank == 0) {

   

 

    // Copy local data to the recv_buffer for the root process

    for (int y = 0; y < rows_per_process; y++) {

      for (int x = 0; x < WIDTH; x++) {

        recv_buffer[y*WIDTH + x] = image[y*WIDTH + x];

      }

    }

 

    // Receive data from other processes

    for (int i = 1; i < size; i++) {

      int offset = i * rows_per_process;

      MPI_Recv(&recv_buffer[offset*WIDTH], rows_per_process*WIDTH, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    }

 

    free(recv_buffer);

  } else {

    // Send local data to the root process

    MPI_Send(image, rows_per_process*WIDTH, MPI_INT, 0, 0, MPI_COMM_WORLD);

  }

 

  double communication_end_time = MPI_Wtime();

 

  // Display the computation time, communication time, and computation to communication ratio

  if (rank == 0) {

    double computation_time = computation_end_time - computation_start_time;

    double communication_time = communication_end_time - communication_start_time;

    double ratio = computation_time / communication_time;

 

    printf("Computation time: %f seconds\n", computation_time);

    printf("Communication time: %f seconds\n", communication_time);

    printf("Computation to communication ratio: %f\n", ratio);

 

    // Display the Mandelbrot set image using ASCII art

    for (int y = 0; y < HEIGHT; y++) {

      for (int x = 0; x < WIDTH; x++) {

        int i = recv_buffer[y*WIDTH + x];

char c = (i == MAX_ITER) ? ' ' : (char)('0' + (i % 10));

putchar(c);

}

putchar('\n');

}

free(recv_buffer);

}

 

free(image);

 

MPI_Finalize();

 

return 0;
}
