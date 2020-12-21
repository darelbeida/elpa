        do j = ncols, 2, -2
          w(:,1) = bcast_buffer(1:nbw,j+off)
          w(:,2) = bcast_buffer(1:nbw,j+off-1)
#ifdef WITH_OPENMP_TRADITIONAL

#ifdef USE_ASSUMED_SIZE
          call double_hh_trafo_&
          &MATH_DATATYPE&
          &VEC_SET&
          &PRECISION&
          & (a(1,j+off+a_off-1,istripe,my_thread), w, nbw, nl, stripe_width, nbw)
#else
          call double_hh_trafo_&
          &MATH_DATATYPE&
          &VEC_SET&
          &PRECISION&
          & (a(1:stripe_width,j+off+a_off-1:j+off+a_off+nbw-1, istripe,my_thread), w(1:nbw,1:6), &
          nbw, nl, stripe_width, nbw)
#endif
#else /* WITH_OPENMP_TRADITIONAL */

#ifdef USE_ASSUMED_SIZE
          call double_hh_trafo_&
          &MATH_DATATYPE&
          &VEC_SET&
          &PRECISION&
          & (a(1,j+off+a_off-1,istripe),w, nbw, nl, stripe_width, nbw)
#else
          call double_hh_trafo_&
          &MATH_DATATYPE&
          &VEC_SET&
          &PRECISION&
          & (a(1:stripe_width,j+off+a_off-1:j+off+a_off+nbw-1,istripe),w(1:nbw,1:6), nbw, nl, stripe_width, nbw)
#endif
#endif /* WITH_OPENMP_TRADITIONAL */
        enddo

