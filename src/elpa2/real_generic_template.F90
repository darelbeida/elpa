        do j = ncols, 2, -2
          w(:,1) = bcast_buffer(1:nbw,j+off)
          w(:,2) = bcast_buffer(1:nbw,j+off-1)
#ifdef WITH_OPENMP_TRADITIONAL

#ifdef _BGP_
#if REALCASE == 1
          call double_hh_trafo&
          &VEC_SET&
          &PRECISION&
          & (a(1,j+off+a_off-1,istripe,my_thread), w, nbw, nl, stripe_width, nbw)
#endif
#else /* _BGP_ */

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

#endif /* _BGP_ */

#else /* WITH_OPENMP_TRADITIONAL */

#ifdef _BGP_
#if REALCASE == 1
          call double_hh_trafo&
          &VEC_SET&
          &PRECISION&
          & (a(1,j+off+a_off-1,istripe), w, nbw, nl, stripe_width, nbw)
#endif
#else /* _BGP_ */

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

#endif /* _BGP_ */
#endif /* WITH_OPENMP_TRADITIONAL */
        enddo

#if REALCASE == 1
! this is due to an inconsistent naming of the routines
! should be fixed soon
#ifdef WITH_OPENMP_TRADITIONAL
        if (j==1) call single_hh_trafo_&
        &MATH_DATATYPE&
        &_cpu_openmp_&
        &PRECISION&
        & (a(1:stripe_width, 1+off+a_off:1+off+a_off+nbw-1,istripe,my_thread), &
                 bcast_buffer(1:nbw,off+1), nbw, nl,stripe_width)
#else
        if (j==1) call single_hh_trafo_&
        &MATH_DATATYPE&
        &_cpu_&
        &PRECISION&
        & (a(1:stripe_width,1+off+a_off:1+off+a_off+nbw-1,istripe), bcast_buffer(1:nbw,off+1), nbw, nl,&
                 stripe_width)
#endif
#endif
#if COMPLEXCASE == 1
#ifdef WITH_OPENMP_TRADITIONAL
#ifdef USE_ASSUMED_SIZE

        if (j == 1) call single_hh_trafo_&
                 &MATH_DATATYPE&
                 &VEC_SET&
                 &PRECISION&
                 & (a(1,j+off+a_off,istripe,my_thread), bcast_buffer(1,j+off),nbw,nl,stripe_width)
#else
        if (j == 1) call single_hh_trafo_&
                 &MATH_DATATYPE&
                 &VEC_SET&
                 &PRECISION&
                 & (a(1:stripe_width,j+off+a_off:j+off+a_off+nbw-1,istripe,my_thread), &
                 bcast_buffer(1:nbw,j+off), nbw, nl, stripe_width)
#endif

#else /* WITH_OPENMP_TRADITIONAL */

#ifdef USE_ASSUMED_SIZE
         if (j == 1) call single_hh_trafo_&
                 &MATH_DATATYPE&
                 &VEC_SET&
                 &PRECISION&
                 & (a(1,j+off+a_off,istripe), bcast_buffer(1,j+off),nbw,nl,stripe_width)
#else
         if (j == 1) call single_hh_trafo_&
                 &MATH_DATATYPE&
                 &VEC_SET&
                 &PRECISION&
                 & (a(1:stripe_width,j+off+a_off:j+off+a_off+nbw-1,istripe), bcast_buffer(1:nbw,j+off), &
                 nbw, nl, stripe_width)
#endif
#endif /* WITH_OPENMP_TRADITIONAL */
#endif
