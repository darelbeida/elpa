      do j = ncols, 4, -4
        w(:,1) = bcast_buffer(1:nbw,j+off)
        w(:,2) = bcast_buffer(1:nbw,j+off-1)
        w(:,3) = bcast_buffer(1:nbw,j+off-2)
        w(:,4) = bcast_buffer(1:nbw,j+off-3)
#ifdef WITH_OPENMP_TRADITIONAL
        call quad_hh_trafo_&
        &MATH_DATATYPE&
        &VEC_SET&
        &4hv_&
        &PRECISION&
        & (c_loc(a(1,j+off+a_off-3,istripe,my_thread)), w, nbw, nl, stripe_width, nbw)
#else
        call quad_hh_trafo_&
        &MATH_DATATYPE&
        &VEC_SET&
        &4hv_&
        &PRECISION&
        & (c_loc(a(1,j+off+a_off-3,istripe)), w, nbw, nl, stripe_width, nbw)
#endif
      enddo
      do jj = j, 2, -2
        w(:,1) = bcast_buffer(1:nbw,jj+off)
        w(:,2) = bcast_buffer(1:nbw,jj+off-1)
#ifdef WITH_OPENMP_TRADITIONAL
        call double_hh_trafo_&
        &MATH_DATATYPE&
        &VEC_SET&
        &2hv_&
        &PRECISION&
        & (c_loc(a(1,jj+off+a_off-1,istripe,my_thread)), w, nbw, nl, stripe_width, nbw)
#else
        call double_hh_trafo_&
        &MATH_DATATYPE&
        &VEC_SET&
        &2hv_&
        &PRECISION&
        & (c_loc(a(1,jj+off+a_off-1,istripe)), w, nbw, nl, stripe_width, nbw)
#endif
      enddo
#ifdef WITH_OPENMP_TRADITIONAL
      if (jj==1) call single_hh_trafo_&
      &MATH_DATATYPE&
      &_cpu_openmp_&
      &PRECISION&
      & (a(1:stripe_width,1+off+a_off:1+off+a_off+nbw-1, istripe,my_thread), &
               bcast_buffer(1:nbw,off+1), nbw, nl, stripe_width)
#else
      if (jj==1) call single_hh_trafo_&
      &MATH_DATATYPE&
      &_cpu_&
      &PRECISION&
      & (a(1:stripe_width,1+off+a_off:1+off+a_off+nbw-1,istripe), bcast_buffer(1:nbw,off+1), nbw, nl, stripe_width)
#endif

