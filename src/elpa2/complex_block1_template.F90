        ttt = mpi_wtime()
        do j = ncols, 1, -1
#ifdef WITH_OPENMP_TRADITIONAL
          call single_hh_trafo_&
          &MATH_DATATYPE&
#ifdef _ASSEMBLY_
          &_&
          &PRECISION&
#endif
          &VEC_SET&
#ifdef _ASSEMBLY_
#else
          &1hv_&
          &PRECISION&
#endif
          & (c_loc(a(1,j+off+a_off,istripe,my_thread)), bcast_buffer(1,j+off),nbw,nl,stripe_width)
#else
          call single_hh_trafo_&
          &MATH_DATATYPE&
#ifdef _ASSEMBLY_
          &_&
          &PRECISION&
#endif
          &VEC_SET&
#ifdef _ASSEMBLY_
#else
          &1hv_&
          &PRECISION&
#endif
          & (c_loc(a(1,j+off+a_off,istripe)), bcast_buffer(1,j+off),nbw,nl,stripe_width)
#endif
        enddo

