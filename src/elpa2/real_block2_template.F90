        do j = ncols, 2, -2
          w(:,1) = bcast_buffer(1:nbw,j+off)
          w(:,2) = bcast_buffer(1:nbw,j+off-1)

#ifdef WITH_OPENMP_TRADITIONAL
          call double_hh_trafo_&
          &MATH_DATATYPE&
#ifdef _ASSEMBLY_
          &_&
          &PRECISION&
#endif
          &VEC_SET&
#ifdef _ASSEMBLY_
#else
          &2hv_&
          &PRECISION &
#endif
          & (c_loc(a(1,j+off+a_off-1,istripe,my_thread)), w, nbw, nl, stripe_width, nbw)
#else
          call double_hh_trafo_&
          &MATH_DATATYPE&
#ifdef _ASSEMBLY_
          &_&
          &PRECISION&
#endif
          &VEC_SET&
#ifdef _ASSEMBLY_
#else
          &2hv_&
          &PRECISION &
#endif
          & (c_loc(a(1,j+off+a_off-1,istripe)), w, nbw, nl, stripe_width, nbw)
#endif
        enddo
