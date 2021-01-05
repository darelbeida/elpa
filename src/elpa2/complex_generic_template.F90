          ttt = mpi_wtime()
          do j = ncols, 1, -1
#ifdef WITH_OPENMP_TRADITIONAL
#ifdef USE_ASSUMED_SIZE

            call single_hh_trafo_&
                 &MATH_DATATYPE&
                 &VEC_SET&
                 &PRECISION&
                 & (a(1,j+off+a_off,istripe,my_thread), bcast_buffer(1,j+off),nbw,nl,stripe_width)
#else
            call single_hh_trafo_&
                 &MATH_DATATYPE&
                 &VEC_SET&
                 &PRECISION&
                 & (a(1:stripe_width,j+off+a_off:j+off+a_off+nbw-1,istripe,my_thread), &
                 bcast_buffer(1:nbw,j+off), nbw, nl, stripe_width)
#endif

#else /* WITH_OPENMP_TRADITIONAL */

#ifdef USE_ASSUMED_SIZE
            call single_hh_trafo_&
                 &MATH_DATATYPE&
                 &VEC_SET&
                 &PRECISION&
                 & (a(1,j+off+a_off,istripe), bcast_buffer(1,j+off),nbw,nl,stripe_width)
#else
            call single_hh_trafo_&
                 &MATH_DATATYPE&
                 &VEC_SET&
                 &PRECISION&
                 & (a(1:stripe_width,j+off+a_off:j+off+a_off+nbw-1,istripe), bcast_buffer(1:nbw,j+off), &
                 nbw, nl, stripe_width)
#endif
#endif /* WITH_OPENMP_TRADITIONAL */

          enddo

