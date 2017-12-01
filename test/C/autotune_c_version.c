/*     This file is part of ELPA. */
/*  */
/*     The ELPA library was originally created by the ELPA consortium, */
/*     consisting of the following organizations: */
/*  */
/*     - Max Planck Computing and Data Facility (MPCDF), formerly known as */
/*       Rechenzentrum Garching der Max-Planck-Gesellschaft (RZG), */
/*     - Bergische Universität Wuppertal, Lehrstuhl für angewandte */
/*       Informatik, */
/*     - Technische Universität München, Lehrstuhl für Informatik mit */
/*       Schwerpunkt Wissenschaftliches Rechnen , */
/*     - Fritz-Haber-Institut, Berlin, Abt. Theorie, */
/*     - Max-Plack-Institut für Mathematik in den Naturwissenschaften, */
/*       Leipzig, Abt. Komplexe Strukutren in Biologie und Kognition, */
/*       and */
/*     - IBM Deutschland GmbH */
/*  */
/*  */
/*     More information can be found here: */
/*     http://elpa.mpcdf.mpg.de/ */
/*  */
/*     ELPA is free software: you can redistribute it and/or modify */
/*     it under the terms of the version 3 of the license of the */
/*     GNU Lesser General Public License as published by the Free */
/*     Software Foundation. */
/*  */
/*     ELPA is distributed in the hope that it will be useful, */
/*     but WITHOUT ANY WARRANTY; without even the implied warranty of */
/*     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/*     GNU Lesser General Public License for more details. */
/*  */
/*     You should have received a copy of the GNU Lesser General Public License */
/*     along with ELPA.  If not, see <http://www.gnu.org/licenses/> */
/*  */
/*     ELPA reflects a substantial effort on the part of the original */
/*     ELPA consortium, and we ask you to respect the spirit of the */
/*     license that we chose: i.e., please contribute any changes you */
/*     may have back to the original ELPA library distribution, and keep */
/*     any derivatives of ELPA under the same license that we chose for */
/*     the original distribution, the GNU Lesser General Public License. */
/*  */
/*  */

#include "config-f90.h"

#include <stdio.h>
#include <stdlib.h>
#ifdef WITH_MPI
#include <mpi.h>
#endif
#include <math.h>

#include <elpa/elpa.h>
#include <assert.h>

#include <string.h>

#include <test/shared/generated.h>

#define DOUBLE_PRECISION_REAL 1

#define assert_elpa_ok(x) assert(x == ELPA_OK)

int main(int argc, char** argv) {
   int myid;
   int nprocs;
#ifndef WITH_MPI
   int MPI_COMM_WORLD;
#endif
   int na, nev, nblk;

   int status;

   int np_cols, np_rows, np_colsStart;

   int my_blacs_ctxt, my_prow, my_pcol;

   int mpierr;

   int my_mpi_comm_world;
   int mpi_comm_rows, mpi_comm_cols;

   int info, *sc_desc;

   int na_rows, na_cols;
   double startVal;
#ifdef DOUBLE_PRECISION_REAL
   double *a, *z, *as, *ev;
#else
   float *a, *z, *as, *ev;
#endif

   int success;

   elpa_t handle;
   elpa_autotune_t autotune_handle;
   int value, error, unfinished, i;

#ifdef WITH_MPI
   MPI_Init(&argc, &argv);
   MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
   MPI_Comm_rank(MPI_COMM_WORLD, &myid);
#else
   nprocs = 1;
   myid=0;
   MPI_COMM_WORLD=1;
#endif
   na = 100;
   nev = 50;
   nblk = 16;

   if (myid == 0) {
     printf("This is the c version of an ELPA test-programm\n");
     printf("\n");
     printf("It will call the 2stage ELPA real solver for an\n");
     printf("matrix of size %d. It will compute %d eigenvalues\n",na,nev);
     printf("and uses a blocksize of %d\n",nblk);
     printf("\n");
     printf("This is an example program with much less functionality\n");
     printf("as it's Fortran counterpart. It's only purpose is to show how \n");
     printf("to evoke ELPA2 from a c programm\n");
     printf("\n");
#ifdef DOUBLE_PRECISION_REAL
    printf(" Double precision version of ELPA2 is used. \n");
#else
    printf(" Single precision version of ELPA2 is used. \n");
#endif
   }

   status = 0;

   startVal = sqrt((double) nprocs);
   np_colsStart = (int) round(startVal);
   for (np_cols=np_colsStart;np_cols>1;np_cols--){
     if (nprocs %np_cols ==0){
     break;
     }
   }

   np_rows = nprocs/np_cols;

   if (myid == 0) {
     printf("\n");
     printf("Number of processor rows %d, cols %d, total %d \n",np_rows,np_cols,nprocs);
   }

   /* set up blacs */
   /* convert communicators before */
#ifdef WITH_MPI
   my_mpi_comm_world = MPI_Comm_c2f(MPI_COMM_WORLD);
#else
  my_mpi_comm_world = 1;
#endif
   set_up_blacsgrid_f(my_mpi_comm_world, np_rows, np_cols, 'C', &my_blacs_ctxt, &my_prow, &my_pcol);

   if (myid == 0) {
     printf("\n");
     printf("Past BLACS_Gridinfo...\n");
     printf("\n");
   }

   sc_desc = malloc(9*sizeof(int));

   set_up_blacs_descriptor_f(na, nblk, my_prow, my_pcol, np_rows, np_cols, &na_rows, &na_cols, sc_desc, my_blacs_ctxt, &info);

   if (myid == 0) {
     printf("\n");
     printf("Past scalapack descriptor setup...\n");
     printf("\n");
   }

   /* allocate the matrices needed for elpa */
   if (myid == 0) {
     printf("\n");
     printf("Allocating matrices with na_rows=%d and na_cols=%d\n",na_rows, na_cols);
     printf("\n");
   }
#ifdef DOUBLE_PRECISION_REAL
   a  = malloc(na_rows*na_cols*sizeof(double));
   z  = malloc(na_rows*na_cols*sizeof(double));
   as = malloc(na_rows*na_cols*sizeof(double));
   ev = malloc(na*sizeof(double));
#else
   a  = malloc(na_rows*na_cols*sizeof(float));
   z  = malloc(na_rows*na_cols*sizeof(float));
   as = malloc(na_rows*na_cols*sizeof(float));
   ev = malloc(na*sizeof(float));
#endif
#ifdef DOUBLE_PRECISION_REAL
   prepare_matrix_random_real_double_f(na, myid, na_rows, na_cols, sc_desc, a, z, as);
#else
   prepare_matrix_random_real_single_f(na, myid, na_rows, na_cols, sc_desc, a, z, as);
#endif

   if (elpa_init(CURRENT_API_VERSION) != ELPA_OK) {
     fprintf(stderr, "Error: ELPA API version not supported");
     exit(1);
   }

   handle = elpa_allocate(&error);
   assert_elpa_ok(error);

   /* Set parameters */
   elpa_set(handle, "na", na, &error);
   assert_elpa_ok(error);

   elpa_set(handle, "nev", nev, &error);
   assert_elpa_ok(error);

   elpa_set(handle, "local_nrows", na_rows, &error);
   assert_elpa_ok(error);

   elpa_set(handle, "local_ncols", na_cols, &error);
   assert_elpa_ok(error);

   elpa_set(handle, "nblk", nblk, &error);
   assert_elpa_ok(error);

#ifdef WITH_MPI
   elpa_set(handle, "mpi_comm_parent", MPI_Comm_c2f(MPI_COMM_WORLD), &error);
   assert_elpa_ok(error);

   elpa_set(handle, "process_row", my_prow, &error);
   assert_elpa_ok(error);

   elpa_set(handle, "process_col", my_pcol, &error);
   assert_elpa_ok(error);
#endif

   /* Setup */
   assert_elpa_ok(elpa_setup(handle));

   /* Set tunables */

   elpa_set(handle, "gpu", 0, &error);
   assert_elpa_ok(error);
#ifdef WITH_MPI
   mpierr = MPI_Barrier(MPI_COMM_WORLD);
#endif
 
   autotune_handle = elpa_autotune_setup(handle, ELPA_AUTOTUNE_FAST, ELPA_AUTOTUNE_DOMAIN_REAL);
   /* mimic 10 scf steps */

   for (i=0; i < 20; i++) {

      unfinished = elpa_autotune_step(handle, autotune_handle);

      if (unfinished == 0) {
        if (myid == 0) {
       	  printf("ELPA autotuning finished in the %d th scf step \n",i);
        }
	break;
      }
      /* Solve EV problem */
      elpa_eigenvectors(handle, a, ev, z, &error);
      assert_elpa_ok(error);

      /* check the results */
#ifdef DOUBLE_PRECISION_REAL
      status = check_correctness_evp_numeric_residuals_real_double_f(na, nev, na_rows, na_cols, as, z, ev, sc_desc, myid);
      memcpy(a, as, na_rows*na_cols*sizeof(double));
#else
      status = check_correctness_evp_numeric_residuals_real_single_f(na, nev, na_rows, na_cols, as, z, ev, sc_desc, myid);
      memcpy(a, as, na_rows*na_cols*sizeof(float));
#endif

      if (status !=0){
        printf("The computed EVs are not correct !\n");
	break;
      }
      printf("hier %d \n",myid);
   }

   if (unfinished == 1) {
     if (myid == 0) {
        printf("ELPA autotuning did not finished during %d scf cycles\n",i);

     }	     

   }
   elpa_autotune_set_best(handle, autotune_handle);

   elpa_autotune_deallocate(autotune_handle);
   elpa_deallocate(handle);
   elpa_uninit();

   if (myid == 0) {
     printf("\n");
     printf("2stage ELPA real solver complete\n");
     printf("\n");
   }

   if (status ==0){
     if (myid ==0) {
       printf("All ok!\n");
     }
   }

   free(sc_desc);
   free(a);
   free(z);
   free(as);
   free(ev);

#ifdef WITH_MPI
   MPI_Finalize();
#endif
   return 0;
}