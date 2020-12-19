        ttt = mpi_wtime()
        do j = ncols, 2, -2
          w(:,1) = bcast_buffer(1:nbw,j+off)
          w(:,2) = bcast_buffer(1:nbw,j+off-1)
#ifdef WITH_OPENMP_TRADITIONAL
          call double_hh_trafo_&
          &MATH_DATATYPE&
          &VEC_SET&
          &2hv_&
          &PRECISION&
          & (c_loc(a(1,j+off+a_off-1,istripe,my_thread)), w, nbw, nl, stripe_width, nbw)
#else
          call double_hh_trafo_&
          &MATH_DATATYPE&
          &VEC_SET&
          &2hv_&
          &PRECISION&
          & (c_loc(a(1,j+off+a_off-1,istripe)), w, nbw, nl, stripe_width, nbw)
#endif
        enddo
#ifdef WITH_OPENMP_TRADITIONAL
        if (j==1) call single_hh_trafo_&
        &MATH_DATATYPE&
        &VEC_SET&
        &1hv_&
        &PRECISION&
        & (c_loc(a(1,1+off+a_off,istripe,my_thread)), bcast_buffer(1,off+1), nbw, nl, stripe_width)
#else
        if (j==1) call single_hh_trafo_&
        &MATH_DATATYPE&
        &VEC_SET&
        &1hv_&
        &PRECISION&
        & (c_loc(a(1,1+off+a_off,istripe)), bcast_buffer(1,off+1), nbw, nl, stripe_width)
#endif
