#include "config-f90.h"
#include <stdio.h>
#include <stdlib.h>
#ifdef WITH_MPI
#include <mpi.h>
#endif
#include <math.h>

//#include <elpa/elpa.h>
//#include <elpa/elpa_generated.h>
//#include <elpa/elpa_constants.h>
//#include <elpa/elpa_generated_legacy.h>
//#include <elpa/elpa_generic.h>
//#include <elpa/elpa_legacy.h>
//
void pdlacpy_(char*, int*, int*, double*, int*, int*, int*, double*, int*, int*, int*);
void dlacpy_(char*, int*, int*, double*, int*, double*, int*);
void dgemm_(char*, char*, int*, int*, int*, double*, double*, int*, double*, int*, double*, double*, int*); 
void pdtran_(int*, int*, double*, double*, int*, int*, int*, double*, double*, int*, int*, int*);
//void pdelset_(double*, int*, int*, int*, double*);
//void pdsymm_(char*, char*, int*, int*, double*, double*, int*, int*, int*, double*, int*, int*, int*, double*, double*, int*, int*, int*);
//void pdpotrf_(char*, int*, double*, int*, int*, int*, int*);
//void pdsyngst_(int*, char*, int*, double*, int*, int*, int*, double*, int*, int*, int*, double*, double*, int*, int*);
//void descinit_(int*, int*, int*, int*, int*, int*, int*, int*, int*, int*);
int numroc_(int*, int*, int*, int*, int*);
//void set_up_blacsgrid_f1(int, int*, int*, int*, int*, int*, int*, int*);
//void pdtrtrs_(char*, char*, char*, int*, int*, double*, int*, int*, int*, double*, int*, int*, int*, int*);
//void pdsyevr_(char*, char*, char*, int*, double*, int*, int*, int*, int*, int*, int*, int*, int*, int*, double*, double*, int*, int*, int*, double*, int*, int*, int*, int*);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////// My function for reduction //////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void d_test_c_bindings(double* A, int np_rows, int np_cols, int my_prow, int my_pcol, int* a_desc,
                         double *Res, MPI_Comm row_comm, MPI_Comm col_comm)
{
   int na, nblk, i, j, Size_send_A, Size_receive_A, Size_send_U, Size_receive_U, Buf_rows, Buf_cols, where_to_send_A, from_where_to_receive_A, where_to_send_U, from_where_to_receive_U, last_proc_row, last_proc_col, cols_in_buffer_A, rows_in_buffer_A, intNumber;
   double *Buf_to_send_A, *Buf_to_receive_A, *Buf_to_send_U, *Buf_to_receive_U, *double_ptr, *Buf_A, *Buf_pos, *U_local_start, *Res_ptr, *M, *M_T, *A_local_start, *U_local_start_curr, *U_stored, *CopyTo, *CopyFrom, *U_to_calc;
   int ratio, num_of_iters, cols_in_buffer, rows_in_block, rows_in_buffer, curr_col_loc, cols_in_block, curr_col_glob, curr_row_loc, Size_receive_A_now, Nb, owner, cols_in_buffer_A_now;
   int  row_of_origin_U, rows_in_block_U, num_of_blocks_in_U_buffer, k, startPos, cols_in_buffer_U, rows_in_buffer_U, col_of_origin_A, curr_row_loc_res, curr_row_loc_A, curr_col_glob_res; 
   int curr_col_loc_res, curr_col_loc_buf, proc_row_curr, curr_col_loc_U, A_local_index, LDA_A, LDA_A_new, index_row_A_for_LDA, ii, rows_in_block_U_curr, width, row_origin_U, rows_in_block_A, cols_in_buffer_A_my_initial, rows_in_buffer_A_my_initial, proc_col_min;
   int *SizesU;
   int Size_U_skewed, Size_U_stored, Curr_pos_in_U_stored, rows_in_buffer_A_now;
   double done = 1.0;
   double dzero = 0.0;
   int one = 1; 
   int zero = 0; 
   int na_rows, na_cols;
        
   MPI_Status status;
   MPI_Request request_A_Recv; 
   MPI_Request request_A_Send;
   MPI_Request request_U_Recv; 
   MPI_Request request_U_Send;
      
   na = a_desc[2];
   nblk = a_desc[4];
   na_rows = numroc_(&na, &nblk, &my_prow, &zero, &np_rows);
   na_cols = numroc_(&na, &nblk, &my_pcol, &zero, &np_cols); 

   for (int i = 0; i < na_rows * na_cols; i++)
     Res[i] = A[i] + 2;
}

//***********************************************************************************************************
/*
!f> interface
!f>   subroutine test_c_bindings(A, local_rows, local_cols, np_rows, np_cols, my_prow, my_pcol, a_desc, &
!f>                                Res, row_comm, col_comm) &
!f>                             bind(C, name="d_test_c_bindings_c")
!f>     use, intrinsic :: iso_c_binding
!f>     real(c_double)                        :: A(local_rows, local_cols), Res(local_rows, local_cols)
!f>     !type(c_ptr), value                   :: A, Res
!f>     integer(kind=c_int)                   :: a_desc(9)
!f>     integer(kind=c_int),value             :: local_rows, local_cols
!f>     integer(kind=c_int),value     :: np_rows, np_cols, my_prow, my_pcol, row_comm, col_comm
!f>   end subroutine
!f> end interface
*/
void d_test_c_bindings_c(double* A, int local_rows, int local_cols, int np_rows, int np_cols, int my_prow, int my_pcol, int* a_desc,
                         double *Res, int row_comm, int col_comm)
{
  //printf("%d, %d, %d, %d, %lf, %lf, %lf, %lf, com: %d, %d\n", np_rows, np_cols, my_prow, my_pcol, A[0], A[1], U[0], U[1], row_comm, col_comm);

  MPI_Comm c_row_comm = MPI_Comm_f2c(row_comm);
  MPI_Comm c_col_comm = MPI_Comm_f2c(col_comm);
  
  //int c_my_prow, c_my_pcol;
  //MPI_Comm_rank(c_row_comm, &c_my_prow);
  //MPI_Comm_rank(c_col_comm, &c_my_pcol);
  //printf("FORT<->C row: %d<->%d, col: %d<->%d\n", my_prow, c_my_prow, my_pcol, c_my_pcol);

  // BEWARE
  // in the cannons algorithm, column and row communicators are exchanged
  // What we usually call row_comm in elpa, is thus passed to col_comm parameter of the function and vice versa
  // (order is swapped in the following call)
  // It is a bit unfortunate, maybe it should be changed in the Cannon algorithm to comply with ELPA standard notation?
  d_test_c_bindings(A, np_rows, np_cols, my_prow, my_pcol, a_desc, Res, c_col_comm, c_row_comm);
}

