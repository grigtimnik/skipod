#include <mpi.h>
#include <mpi-ext.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>

#define MATRIX_SIZE 10
#define PROCESS_TO_KILL 2

int *ranks_gc;
int nf = 0;

int multiply(int **a, int *b, int *c, int n, int rank, int num_of_proccesses) {
    int first, num_rows;
    num_rows = n / num_of_proccesses;
    first = rank * num_rows;
    
    
    if (rank == num_of_proccesses - 1 && n % num_of_proccesses != 0) {
        num_rows += n % num_of_proccesses;
    }
    
    // fprintf(stderr, "num_rows = %d \n", num_rows);
    
    for (int i = first; i < (first + num_rows); ++i) {
        for (int j = 0; j < n; ++j) {
            c[i - first] += a[i-first][j] * b[j];
        }
    }
    return num_rows;
}

void print_vector(int *a, int n) {
    for (int i = 0; i < n; ++i) {
        printf("%d ", a[i]);
    }
    printf("\n");
}

int* create_zero_vector(int n) {
    int *c = malloc(sizeof(int) * n);
    for (int i = 0; i < n; ++i) {
        c[i] = 0;
    }
    return c;
}

int* vector_initializing(int* n) {
    int *b;
    b = (int*)malloc((*n)*sizeof(int*));

    for (int i = 0; i < (*n); ++i) {
        b[i] = 1;
    }
    return b;
}

int** matrix_initializing(int* n, int num_of_proccesses, int rank) {
    int num_rows = (*n) / num_of_proccesses;
    int **A;
    int start_row = rank * num_rows;
    if ((rank == num_of_proccesses - 1) && ((*n) % num_of_proccesses != 0)) {
        num_rows += (*n) % num_of_proccesses;
    }
    
    A = (int**) malloc(num_rows*sizeof(int*));
    for (int i = 0; i < num_rows; ++i) {
        A[i] = (int*) malloc((*n)*sizeof(int));
    }

    
    for (int i = 0; i < num_rows; i++) {
        for (int j = 0; j < (*n); j++) {
            A[i][j] = start_row + i;
        }
    }

    return A;
}

void free_vector(int *b) {
    free(b);
}

void free_matrix(int **A, int n) {
    for (int i = 0; i < n; i++) {
        free(A[i]);
    }
    free(A);
}


static void verbose_errhandler(MPI_Comm *pcomm, int *perr, ...)
{
	
    free(ranks_gc);

    MPI_Comm comm = *pcomm;
    int err = *perr;
    char errstr[MPI_MAX_ERROR_STRING];
    int i, rank, size, len, eclass;
    MPI_Group group_c, group_f;
    int *ranks_gf;

    MPI_Error_class(err, &eclass);
    if (MPIX_ERR_PROC_FAILED != eclass) {
        MPI_Abort(comm, err);
    }

    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &size);

    MPIX_Comm_failure_ack(comm);
    MPIX_Comm_failure_get_acked(comm, &group_f);
    MPI_Group_size(group_f, &nf);
    MPI_Error_string(err, errstr, &len);
    

    printf("Rank %d / %d: Notified of error %s. %d found dead: { ", rank, size, errstr, nf);

    ranks_gf = (int *)malloc(nf * sizeof(int));
    ranks_gc = (int *)malloc(nf * sizeof(int));
    MPI_Comm_group(comm, &group_c);
    for (i = 0; i < nf; i++) {
        ranks_gf[i] = i;
    }
    MPI_Group_translate_ranks(group_f, nf, ranks_gf,
                              group_c, ranks_gc);

    for(i = 0; i < nf; i++)
        printf("%d ", ranks_gc[i]);
    printf("}\n");

    free(ranks_gf);
    
}



int main(int argc, char** argv) {
    int **a;
    int *b, *c, *c_tmp;
    int n = MATRIX_SIZE;
    double start_time, end_time, time_loc, time;
    int *sendcounts;
    int *displacements;
    int num_rows;

    b = vector_initializing(&n);

    int rank, num_of_proccesses;

    MPI_Init(&argc, &argv);

    MPI_Comm_size(MPI_COMM_WORLD, &num_of_proccesses);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    
    MPI_Errhandler errh;
    MPI_Comm_create_errhandler(verbose_errhandler, &errh);
    MPI_Comm_set_errhandler(MPI_COMM_WORLD,errh);
    
    // fprintf(stderr, "my rank is %d \n", rank);

    a = matrix_initializing(&n, num_of_proccesses, rank);
    
    if (rank == 0) {
        c = create_zero_vector(n);
    }
    
    if (rank == 0) {
        sendcounts = (int *)malloc(num_of_proccesses * sizeof(int));
        displacements = (int *)malloc(num_of_proccesses * sizeof(int));
		
		int displacement = 0;
        for (int i = 0; i < num_of_proccesses; i++) {
            sendcounts[i] = n / num_of_proccesses;
            if (i == num_of_proccesses - 1 && n % num_of_proccesses != 0) {
                sendcounts[i] += n % num_of_proccesses;
            }
            displacements[i] = displacement;
        	displacement += sendcounts[i];
            // fprintf(stderr, "sendcounts  = %d \n", sendcounts[i]); 
        }
    }
	
    start_time = MPI_Wtime();

    
    if (rank == num_of_proccesses - 1 && n % num_of_proccesses != 0) {
    	c_tmp = create_zero_vector(n / num_of_proccesses + n % num_of_proccesses);	
    }
    else {
    	c_tmp = create_zero_vector(n / num_of_proccesses);
    }
    num_rows = multiply(a, b, c_tmp, n, rank, num_of_proccesses);
	
	/* убиваем один процесс */
	if (rank == PROCESS_TO_KILL){
		printf("\nRank %d / %d: process killed!\n\n", rank, num_of_proccesses);
    	raise(SIGKILL);
	}
    	

	if (rank == 0) {
        for (int i = 0; i < num_of_proccesses; i++) {
            if (i == 0) {
                for (int j = 0; j < num_rows; j++) {
                    c[j] = c_tmp[j];
                }
            } else {
                MPI_Recv(c + displacements[i], sendcounts[i], MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
        }
    } else {
    	

        MPI_Send(c_tmp, num_rows, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }
    
    
    if (nf > 0 && rank == 0) {
    	// fprintf(stderr, "number of failed processes = %d\n", nf);
    	// failed_rank получаем из обработчика ошибок
    	int failed_rank = ranks_gc[0];
    	int failed_displacement = displacements[failed_rank];
    	int failed_rows = sendcounts[failed_rank];
    	a = matrix_initializing(&n, num_of_proccesses, failed_rank);
    	if (failed_rank == num_of_proccesses - 1 && n % num_of_proccesses != 0) {
			c_tmp = create_zero_vector(n / num_of_proccesses + n % num_of_proccesses);	
		}
		else {
			c_tmp = create_zero_vector(n / num_of_proccesses);
		}
    	num_rows = multiply(a, b, c_tmp, n, failed_rank, num_of_proccesses);
    	memcpy(c + failed_displacement, c_tmp, failed_rows * sizeof(int));
    }
	

    end_time = MPI_Wtime();
    

	if (rank == 0) {
    	print_vector(c, n);
    	free_vector(c);
    }
    
    free_matrix(a, n / num_of_proccesses);
    free_vector(b);
    free(c_tmp);
    
    printf("Process %d took %f seconds\n", rank, end_time - start_time);

    MPI_Finalize();

    

    return 0;
}
