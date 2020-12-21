      ! X86 INTRINSIC CODE, USING 6 HOUSEHOLDER VECTORS
      do j = ncols, 6, -6
        w(:,1) = bcast_buffer(1:nbw,j+off)
        w(:,2) = bcast_buffer(1:nbw,j+off-1)
        w(:,3) = bcast_buffer(1:nbw,j+off-2)
        w(:,4) = bcast_buffer(1:nbw,j+off-3)
        w(:,5) = bcast_buffer(1:nbw,j+off-4)
        w(:,6) = bcast_buffer(1:nbw,j+off-5)
#ifdef WITH_OPENMP_TRADITIONAL
        call hexa_hh_trafo_&
        &MATH_DATATYPE&
        &VEC_SET&
        &6hv_&
        &PRECISION&
        & (c_loc(a(1,j+off+a_off-5,istripe,my_thread)), w, nbw, nl, stripe_width, nbw)
#else
        call hexa_hh_trafo_&
        &MATH_DATATYPE&
        &VEC_SET&
        &6hv_&
        &PRECISION&
        & (c_loc(a(1,j+off+a_off-5,istripe)), w, nbw, nl, stripe_width, nbw)
#endif
      enddo
      do jj = j, 4, -4
        w(:,1) = bcast_buffer(1:nbw,jj+off)
        w(:,2) = bcast_buffer(1:nbw,jj+off-1)
        w(:,3) = bcast_buffer(1:nbw,jj+off-2)
        w(:,4) = bcast_buffer(1:nbw,jj+off-3)
#ifdef WITH_OPENMP_TRADITIONAL
        call quad_hh_trafo_&
        &MATH_DATATYPE&
        &VEC_SET&
        &4hv_&
        &PRECISION&
        & (c_loc(a(1,jj+off+a_off-3,istripe,my_thread)), w, nbw, nl, stripe_width, nbw)
#else
        call quad_hh_trafo_&
        &MATH_DATATYPE&
        &VEC_SET&
        &4hv_&
        &PRECISION&
        & (c_loc(a(1,jj+off+a_off-3,istripe)), w, &
                                                  nbw, nl, stripe_width, nbw)
#endif
      enddo
      do jjj = jj, 2, -2
        w(:,1) = bcast_buffer(1:nbw,jjj+off)
        w(:,2) = bcast_buffer(1:nbw,jjj+off-1)
#ifdef WITH_OPENMP_TRADITIONAL
        call double_hh_trafo_&
        &MATH_DATATYPE&
        &VEC_SET&
        &2hv_&
        &PRECISION&
        & (c_loc(a(1,jjj+off+a_off-1,istripe,my_thread)), w, nbw, nl, stripe_width, nbw)
#else
        call double_hh_trafo_&
        &MATH_DATATYPE&
        &VEC_SET&
        &2hv_&
        &PRECISION&
        & (c_loc(a(1,jjj+off+a_off-1,istripe)), w, nbw, nl, stripe_width, nbw)
#endif
      enddo
#ifdef WITH_OPENMP_TRADITIONAL
      if (jjj==1) call single_hh_trafo_&
      &MATH_DATATYPE&
      &_cpu_openmp_&
      &PRECISION&
      & (a(1:stripe_width,1+off+a_off:1+off+a_off+nbw-1, istripe,my_thread), &
                                bcast_buffer(1:nbw,off+1), nbw, nl, stripe_width)
#else
      if (jjj==1) call single_hh_trafo_&
      &MATH_DATATYPE&
      &_cpu_&
      &PRECISION&
      & (a(1:stripe_width,1+off+a_off:1+off+a_off+nbw-1,istripe), bcast_buffer(1:nbw,off+1), nbw, nl, stripe_width)
#endif

