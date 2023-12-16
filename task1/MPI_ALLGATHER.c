#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <mpi.h>

#define MATRIX_SIZE 5
#define BUF_SIZE 25

// объединяем два буфера в первый
void concat_buffers(int send_buf[BUF_SIZE], int new_buf[BUF_SIZE]) {
    for (int i = 0; i < BUF_SIZE; ++i) {
        if (new_buf[i] != 0) {
            send_buf[i] = new_buf[i];
        }
    }
}


int main(int argc, char* argv[]) {
    int size;
    int dims[2] = { 5, 5 };
    int periods[2] = { false, false };
    int my_rank;
    int my_coords[2];


    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Request request;
    MPI_Status status;

    MPI_Dims_create(size, 2, dims);
    MPI_Comm new_communicator;
    MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periods, false, &new_communicator);

    MPI_Comm_rank(new_communicator, &my_rank);
    MPI_Cart_coords(new_communicator, my_rank, 2, my_coords);

    int send_buf[BUF_SIZE] = {0};
    int new_buf[BUF_SIZE];
    int recv_buf[BUF_SIZE];

    send_buf[my_rank] = my_rank; // В i-ом процессе i-я часть буфера

	
// Первый ряд
    if (my_coords[0] == 0) {
        MPI_Isend(send_buf, BUF_SIZE, MPI_INT, my_rank + 5, 0, MPI_COMM_WORLD, &request);
        MPI_Recv(recv_buf, BUF_SIZE, MPI_INT, my_rank + 5, 0, MPI_COMM_WORLD, &status);
    }

// Последний ряд
    if (my_coords[0] == 4) {
        MPI_Isend(send_buf, BUF_SIZE, MPI_INT, my_rank - 5, 0, MPI_COMM_WORLD, &request);
        MPI_Recv(recv_buf, BUF_SIZE, MPI_INT, my_rank - 5, 0, MPI_COMM_WORLD, &status);
    }
    
// Второй ряд
    if (my_coords[0] == 1) {
    	MPI_Recv(new_buf, BUF_SIZE, MPI_INT, my_rank - 5, 0, MPI_COMM_WORLD, &status);
    	new_buf[my_rank] = my_rank;
    	
        MPI_Isend(new_buf, BUF_SIZE, MPI_INT, my_rank + 5, 0, MPI_COMM_WORLD, &request);
        MPI_Recv(recv_buf, BUF_SIZE, MPI_INT, my_rank + 5, 0, MPI_COMM_WORLD, &status);
        MPI_Isend(recv_buf, BUF_SIZE, MPI_INT, my_rank - 5, 0, MPI_COMM_WORLD, &request);
    }
 
// Четвёртый ряд
    if (my_coords[0] == 3) {
    	MPI_Recv(new_buf, BUF_SIZE, MPI_INT, my_rank + 5, 0, MPI_COMM_WORLD, &status);
    	new_buf[my_rank] = my_rank;
    	
        MPI_Isend(new_buf, BUF_SIZE, MPI_INT, my_rank - 5, 0, MPI_COMM_WORLD, &request);
        MPI_Recv(recv_buf, BUF_SIZE, MPI_INT, my_rank - 5, 0, MPI_COMM_WORLD, &status);
        MPI_Isend(recv_buf, BUF_SIZE, MPI_INT, my_rank + 5, 0, MPI_COMM_WORLD, &request);
    }
    
// Третий ряд
	if (my_coords[0] == 2) {
		if (my_coords[1] == 0) {
			MPI_Recv(new_buf, BUF_SIZE, MPI_INT, my_rank - 5, 0, MPI_COMM_WORLD, &status);
			concat_buffers(send_buf, new_buf);
			MPI_Recv(new_buf, BUF_SIZE, MPI_INT, my_rank + 5, 0, MPI_COMM_WORLD, &status);
			concat_buffers(send_buf, new_buf);
			
			MPI_Isend(send_buf, BUF_SIZE, MPI_INT, my_rank + 1, 0, MPI_COMM_WORLD, &request);
			MPI_Recv(recv_buf, BUF_SIZE, MPI_INT, my_rank + 1, 0, MPI_COMM_WORLD, &status);
			
			MPI_Isend(recv_buf, BUF_SIZE, MPI_INT, my_rank + 5, 0, MPI_COMM_WORLD, &request);
			MPI_Isend(recv_buf, BUF_SIZE, MPI_INT, my_rank - 5, 0, MPI_COMM_WORLD, &request);
		}
		
		if (my_coords[1] == 4) {
			MPI_Recv(new_buf, BUF_SIZE, MPI_INT, my_rank - 5, 0, MPI_COMM_WORLD, &status);
			concat_buffers(send_buf, new_buf);
			MPI_Recv(new_buf, BUF_SIZE, MPI_INT, my_rank + 5, 0, MPI_COMM_WORLD, &status);
			concat_buffers(send_buf, new_buf);
			
			MPI_Isend(send_buf, BUF_SIZE, MPI_INT, my_rank - 1, 0, MPI_COMM_WORLD, &request);
			MPI_Recv(recv_buf, BUF_SIZE, MPI_INT, my_rank - 1, 0, MPI_COMM_WORLD, &status);
			
			MPI_Isend(recv_buf, BUF_SIZE, MPI_INT, my_rank + 5, 0, MPI_COMM_WORLD, &request);
			MPI_Isend(recv_buf, BUF_SIZE, MPI_INT, my_rank - 5, 0, MPI_COMM_WORLD, &request);
		}
		
		if (my_coords[1] == 1) {
			MPI_Recv(new_buf, BUF_SIZE, MPI_INT, my_rank - 5, 0, MPI_COMM_WORLD, &status);
			concat_buffers(send_buf, new_buf);
			MPI_Recv(new_buf, BUF_SIZE, MPI_INT, my_rank + 5, 0, MPI_COMM_WORLD, &status);
			concat_buffers(send_buf, new_buf);
			MPI_Recv(new_buf, BUF_SIZE, MPI_INT, my_rank - 1, 0, MPI_COMM_WORLD, &status);
			concat_buffers(send_buf, new_buf);
			
			MPI_Isend(send_buf, BUF_SIZE, MPI_INT, my_rank + 1, 0, MPI_COMM_WORLD, &request);
			MPI_Recv(recv_buf, BUF_SIZE, MPI_INT, my_rank + 1, 0, MPI_COMM_WORLD, &status);
			
			MPI_Isend(recv_buf, BUF_SIZE, MPI_INT, my_rank - 1, 0, MPI_COMM_WORLD, &request);
			MPI_Isend(recv_buf, BUF_SIZE, MPI_INT, my_rank + 5, 0, MPI_COMM_WORLD, &request);
			MPI_Isend(recv_buf, BUF_SIZE, MPI_INT, my_rank - 5, 0, MPI_COMM_WORLD, &request);
		}
		
		if (my_coords[1] == 3) {
			MPI_Recv(new_buf, BUF_SIZE, MPI_INT, my_rank - 5, 0, MPI_COMM_WORLD, &status);
			concat_buffers(send_buf, new_buf);
			MPI_Recv(new_buf, BUF_SIZE, MPI_INT, my_rank + 5, 0, MPI_COMM_WORLD, &status);
			concat_buffers(send_buf, new_buf);
			MPI_Recv(new_buf, BUF_SIZE, MPI_INT, my_rank + 1, 0, MPI_COMM_WORLD, &status);
			concat_buffers(send_buf, new_buf);
			
			MPI_Isend(send_buf, BUF_SIZE, MPI_INT, my_rank - 1, 0, MPI_COMM_WORLD, &request);
			MPI_Recv(recv_buf, BUF_SIZE, MPI_INT, my_rank - 1, 0, MPI_COMM_WORLD, &status);
			
			MPI_Isend(recv_buf, BUF_SIZE, MPI_INT, my_rank + 1, 0, MPI_COMM_WORLD, &request);
			MPI_Isend(recv_buf, BUF_SIZE, MPI_INT, my_rank + 5, 0, MPI_COMM_WORLD, &request);
			MPI_Isend(recv_buf, BUF_SIZE, MPI_INT, my_rank - 5, 0, MPI_COMM_WORLD, &request);
		}
		
		if (my_coords[1] == 2) {
			MPI_Recv(new_buf, BUF_SIZE, MPI_INT, my_rank - 5, 0, MPI_COMM_WORLD, &status);
			concat_buffers(send_buf, new_buf);
			MPI_Recv(new_buf, BUF_SIZE, MPI_INT, my_rank + 5, 0, MPI_COMM_WORLD, &status);
			concat_buffers(send_buf, new_buf);
			MPI_Recv(new_buf, BUF_SIZE, MPI_INT, my_rank - 1, 0, MPI_COMM_WORLD, &status);
			concat_buffers(send_buf, new_buf);
			MPI_Recv(new_buf, BUF_SIZE, MPI_INT, my_rank + 1, 0, MPI_COMM_WORLD, &status);
			concat_buffers(send_buf, new_buf);
			
			for (int i = 0; i < BUF_SIZE; ++i) {
		        recv_buf[i] = send_buf[i];
		    }
			
			MPI_Isend(recv_buf, BUF_SIZE, MPI_INT, my_rank + 1, 0, MPI_COMM_WORLD, &request);
			MPI_Isend(recv_buf, BUF_SIZE, MPI_INT, my_rank - 1, 0, MPI_COMM_WORLD, &request);
			MPI_Isend(recv_buf, BUF_SIZE, MPI_INT, my_rank + 5, 0, MPI_COMM_WORLD, &request);
			MPI_Isend(recv_buf, BUF_SIZE, MPI_INT, my_rank - 5, 0, MPI_COMM_WORLD, &request);
		}
	
	
	}



    fprintf(stdout, "%2d process recv buf:", my_rank);
    for (int i = 0; i < BUF_SIZE; ++i) {
        fprintf(stdout, " %d", recv_buf[i]);
    }
    fprintf(stdout, "\n");
    
    MPI_Finalize();
    

    return EXIT_SUCCESS;
}
