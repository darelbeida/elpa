//    This file is part of ELPA.
//
//    The ELPA library was originally created by the ELPA consortium,
//    consisting of the following organizations:
//
//    - Max Planck Computing and Data Facility (MPCDF), formerly known as
//      Rechenzentrum Garching der Max-Planck-Gesellschaft (RZG),
//    - Bergische Universität Wuppertal, Lehrstuhl für angewandte
//      Informatik,
//    - Technische Universität München, Lehrstuhl für Informatik mit
//      Schwerpunkt Wissenschaftliches Rechnen ,
//    - Fritz-Haber-Institut, Berlin, Abt. Theorie,
//    - Max-Plack-Institut für Mathematik in den Naturwissenschaften,
//      Leipzig, Abt. Komplexe Strukutren in Biologie und Kognition,
//      and
//    - IBM Deutschland GmbH
//
//    This particular source code file contains additions, changes and
//    enhancements authored by Intel Corporation which is not part of
//    the ELPA consortium.
//
//    More information can be found here:
//    http://elpa.mpcdf.mpg.de/
//
//    ELPA is free software: you can redistribute it and/or modify
//    it under the terms of the version 3 of the license of the
//    GNU Lesser General Public License as published by the Free
//    Software Foundation.
//
//    ELPA is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU Lesser General Public License for more details.
//
//    You should have received a copy of the GNU Lesser General Public License
//    along with ELPA.  If not, see <http://www.gnu.org/licenses/>
//
//    ELPA reflects a substantial effort on the part of the original
//    ELPA consortium, and we ask you to respect the spirit of the
//    license that we chose: i.e., please contribute any changes you
//    may have back to the original ELPA library distribution, and keep
//    any derivatives of ELPA under the same license that we chose for
//    the original distribution, the GNU Lesser General Public License.
//
// Author: Andreas Marek (andreas.marek@mpcdf.mpg.de)
// --------------------------------------------------------------------------------------------------
#include "config-f90.h"

#include <complex.h>
#include <x86intrin.h>

#define __forceinline __attribute__((always_inline))

#ifdef HAVE_AVX512

#define __ELPA_USE_FMA__
#define _mm512_FMADDSUB_pd(a,b,c) _mm512_fmaddsub_pd(a,b,c)
#define _mm512_FMSUBADD_pd(a,b,c) _mm512_fmsubadd_pd(a,b,c)
#endif


//Forward declaration

static __forceinline void hh_trafo_complex_kernel_16_AVX512_2hv_double(double complex* q, double complex* hh, int nb, int ldq, int ldh, double complex s);
static __forceinline void hh_trafo_complex_kernel_8_AVX512_2hv_double(double complex* q, double complex* hh, int nb, int ldq, int ldh, double complex s);
//static __forceinline void hh_trafo_complex_kernel_6_AVX_2hv_double(double complex* q, double complex* hh, int nb, int ldq, int ldh, double complex s);
//static __forceinline void hh_trafo_complex_kernel_4_AVX_2hv_double(double complex* q, double complex* hh, int nb, int ldq, int ldh, double complex s);
//static __forceinline void hh_trafo_complex_kernel_2_AVX_2hv_double(double complex* q, double complex* hh, int nb, int ldq, int ldh, double complex s);

/*
!f>#if defined(HAVE_AVX512)
!f> interface
!f>   subroutine double_hh_trafo_complex_avx512_2hv_double(q, hh, pnb, pnq, pldq, pldh) &
!f>                             bind(C, name="double_hh_trafo_complex_avx512_2hv_double")
!f>     use, intrinsic :: iso_c_binding
!f>     integer(kind=c_int)     :: pnb, pnq, pldq, pldh
!f>     complex(kind=c_double_complex)     :: q(*)
!f>     complex(kind=c_double_complex)     :: hh(pnb,2)
!f>   end subroutine
!f> end interface
!f>#endif
*/

void double_hh_trafo_complex_avx512_2hv_double(double complex* q, double complex* hh, int* pnb, int* pnq, int* pldq, int* pldh)
{
	int i;
	int nb = *pnb;
	int nq = *pldq;
	int ldq = *pldq;
	int ldh = *pldh;

	double complex s = conj(hh[(ldh)+1])*1.0;
	for (i = 2; i < nb; i++)
	{
		s += hh[i-1] * conj(hh[(i+ldh)]);
	}

	for (i = 0; i < nq-8; i+=16)
	{
		hh_trafo_complex_kernel_16_AVX512_2hv_double(&q[i], hh, nb, ldq, ldh, s);
//		hh_trafo_complex_kernel_8_AVX512_2hv_double(&q[i], hh, nb, ldq, ldh, s);
//		hh_trafo_complex_kernel_8_AVX512_2hv_double(&q[i+8], hh, nb, ldq, ldh, s);
	}
	if (nq-i == 0) {
		return;
	} else {
		hh_trafo_complex_kernel_8_AVX512_2hv_double(&q[i], hh, nb, ldq, ldh, s);
	}
}

static __forceinline void hh_trafo_complex_kernel_16_AVX512_2hv_double(double complex* q, double complex* hh, int nb, int ldq, int ldh, double complex s)
{
	double* q_dbl = (double*)q;
	double* hh_dbl = (double*)hh;
	double* s_dbl = (double*)(&s);
        double s_helper[8];

        s_helper[0] = s_dbl[0];
        s_helper[1] = s_dbl[1];
        s_helper[2] = s_dbl[0];
        s_helper[3] = s_dbl[1];
        s_helper[4] = s_dbl[0];
        s_helper[5] = s_dbl[1];
        s_helper[6] = s_dbl[0];
        s_helper[7] = s_dbl[1];

	__m512d x1, x2, x3, x4;
	__m512d y1, y2, y3, y4;
	__m512d q1, q2, q3, q4;
	__m512d h1_real, h1_imag, h2_real, h2_imag;
	__m512d tmp1, tmp2, tmp3, tmp4;
	int i=0;

	//__m512d sign = (__m512d)_mm512_set_epi64(0x8000000000000000, 0x8000000000000000, 0x8000000000000000, 0x8000000000000000, 0x8000000000000000, 0x8000000000000000, 0x8000000000000000, 0x8000000000000000);
       __m512d sign = (__m512d)_mm512_set1_epi64(0x8000000000000000);

	x1 = _mm512_load_pd(&q_dbl[(2*ldq)+0]);  // q1, q2, q3, q4
	x2 = _mm512_load_pd(&q_dbl[(2*ldq)+8]);  // q5, q6, q7, q8
	x3 = _mm512_load_pd(&q_dbl[(2*ldq)+16]); // q9, q10, q11, q12
	x4 = _mm512_load_pd(&q_dbl[(2*ldq)+24]); // q13, q14, q15, q16

	h2_real = _mm512_set1_pd(hh_dbl[(ldh+1)*2]);
	h2_imag = _mm512_set1_pd(hh_dbl[((ldh+1)*2)+1]);

	y1 = _mm512_load_pd(&q_dbl[0]);
	y2 = _mm512_load_pd(&q_dbl[8]);
	y3 = _mm512_load_pd(&q_dbl[16]);
	y4 = _mm512_load_pd(&q_dbl[24]);

	tmp1 = _mm512_mul_pd(h2_imag, x1);

	y1 = _mm512_add_pd(y1, _mm512_FMSUBADD_pd(h2_real, x1, _mm512_shuffle_pd(tmp1, tmp1, 0x55)));

	tmp2 = _mm512_mul_pd(h2_imag, x2);

	y2 = _mm512_add_pd(y2, _mm512_FMSUBADD_pd(h2_real, x2, _mm512_shuffle_pd(tmp2, tmp2, 0x55)));

	tmp3 = _mm512_mul_pd(h2_imag, x3);

	y3 = _mm512_add_pd(y3, _mm512_FMSUBADD_pd(h2_real, x3, _mm512_shuffle_pd(tmp3, tmp3, 0x55)));

	tmp4 = _mm512_mul_pd(h2_imag, x4);

	y4 = _mm512_add_pd(y4, _mm512_FMSUBADD_pd(h2_real, x4, _mm512_shuffle_pd(tmp4, tmp4, 0x55)));

	for (i = 2; i < nb; i++)
	{
		q1 = _mm512_load_pd(&q_dbl[(2*i*ldq)+0]);
		q2 = _mm512_load_pd(&q_dbl[(2*i*ldq)+8]);
		q3 = _mm512_load_pd(&q_dbl[(2*i*ldq)+16]);
		q4 = _mm512_load_pd(&q_dbl[(2*i*ldq)+24]);

		h1_real = _mm512_set1_pd(hh_dbl[(i-1)*2]);
		h1_imag = _mm512_set1_pd(hh_dbl[((i-1)*2)+1]);

		tmp1 = _mm512_mul_pd(h1_imag, q1);

		x1 = _mm512_add_pd(x1, _mm512_FMSUBADD_pd(h1_real, q1, _mm512_shuffle_pd(tmp1, tmp1, 0x55)));

		tmp2 = _mm512_mul_pd(h1_imag, q2);

		x2 = _mm512_add_pd(x2, _mm512_FMSUBADD_pd(h1_real, q2, _mm512_shuffle_pd(tmp2, tmp2, 0x55)));

		tmp3 = _mm512_mul_pd(h1_imag, q3);

		x3 = _mm512_add_pd(x3, _mm512_FMSUBADD_pd(h1_real, q3, _mm512_shuffle_pd(tmp3, tmp3, 0x55)));

		tmp4 = _mm512_mul_pd(h1_imag, q4);

		x4 = _mm512_add_pd(x4, _mm512_FMSUBADD_pd(h1_real, q4, _mm512_shuffle_pd(tmp4, tmp4, 0x55)));

		h2_real = _mm512_set1_pd(hh_dbl[(ldh+i)*2]);
		h2_imag = _mm512_set1_pd(hh_dbl[((ldh+i)*2)+1]);

		tmp1 = _mm512_mul_pd(h2_imag, q1);

		y1 = _mm512_add_pd(y1, _mm512_FMSUBADD_pd(h2_real, q1, _mm512_shuffle_pd(tmp1, tmp1, 0x55)));

		tmp2 = _mm512_mul_pd(h2_imag, q2);

		y2 = _mm512_add_pd(y2, _mm512_FMSUBADD_pd(h2_real, q2, _mm512_shuffle_pd(tmp2, tmp2, 0x55)));

		tmp3 = _mm512_mul_pd(h2_imag, q3);

		y3 = _mm512_add_pd(y3, _mm512_FMSUBADD_pd(h2_real, q3, _mm512_shuffle_pd(tmp3, tmp3, 0x55)));

		tmp4 = _mm512_mul_pd(h2_imag, q4);

		y4 = _mm512_add_pd(y4, _mm512_FMSUBADD_pd(h2_real, q4, _mm512_shuffle_pd(tmp4, tmp4, 0x55)));
	}

	h1_real = _mm512_set1_pd(hh_dbl[(nb-1)*2]);
	h1_imag = _mm512_set1_pd(hh_dbl[((nb-1)*2)+1]);

	q1 = _mm512_load_pd(&q_dbl[(2*nb*ldq)+0]);
	q2 = _mm512_load_pd(&q_dbl[(2*nb*ldq)+8]);
	q3 = _mm512_load_pd(&q_dbl[(2*nb*ldq)+16]);
	q4 = _mm512_load_pd(&q_dbl[(2*nb*ldq)+24]);

	tmp1 = _mm512_mul_pd(h1_imag, q1);

	x1 = _mm512_add_pd(x1, _mm512_FMSUBADD_pd(h1_real, q1, _mm512_shuffle_pd(tmp1, tmp1, 0x55)));

	tmp2 = _mm512_mul_pd(h1_imag, q2);

	x2 = _mm512_add_pd(x2, _mm512_FMSUBADD_pd(h1_real, q2, _mm512_shuffle_pd(tmp2, tmp2, 0x55)));

	tmp3 = _mm512_mul_pd(h1_imag, q3);

	x3 = _mm512_add_pd(x3, _mm512_FMSUBADD_pd(h1_real, q3, _mm512_shuffle_pd(tmp3, tmp3, 0x55)));

	tmp4 = _mm512_mul_pd(h1_imag, q4);

	x4 = _mm512_add_pd(x4, _mm512_FMSUBADD_pd(h1_real, q4, _mm512_shuffle_pd(tmp4, tmp4, 0x55)));

	h1_real = _mm512_set1_pd(hh_dbl[0]);
	h1_imag = _mm512_set1_pd(hh_dbl[1]);

//	h1_real = _mm256_xor_pd(h1_real, sign);
//	h1_imag = _mm256_xor_pd(h1_imag, sign);
        h1_real = (__m512d) _mm512_xor_epi64((__m512i) h1_real, (__m512i) sign);
        h1_imag = (__m512d) _mm512_xor_epi64((__m512i) h1_imag, (__m512i) sign);

	tmp1 = _mm512_mul_pd(h1_imag, x1);

	x1 = _mm512_FMADDSUB_pd(h1_real, x1, _mm512_shuffle_pd(tmp1, tmp1, 0x55));

	tmp2 = _mm512_mul_pd(h1_imag, x2);

	x2 = _mm512_FMADDSUB_pd(h1_real, x2, _mm512_shuffle_pd(tmp2, tmp2, 0x55));

	tmp3 = _mm512_mul_pd(h1_imag, x3);

	x3 = _mm512_FMADDSUB_pd(h1_real, x3, _mm512_shuffle_pd(tmp3, tmp3, 0x55));

	tmp4 = _mm512_mul_pd(h1_imag, x4);

	x4 = _mm512_FMADDSUB_pd(h1_real, x4, _mm512_shuffle_pd(tmp4, tmp4, 0x55));

	h1_real = _mm512_set1_pd(hh_dbl[ldh*2]);
	h1_imag = _mm512_set1_pd(hh_dbl[(ldh*2)+1]);
	h2_real = _mm512_set1_pd(hh_dbl[ldh*2]);
	h2_imag = _mm512_set1_pd(hh_dbl[(ldh*2)+1]);

//	h1_real = _mm256_xor_pd(h1_real, sign);
//	h1_imag = _mm256_xor_pd(h1_imag, sign);
        h1_real = (__m512d) _mm512_xor_epi64((__m512i) h1_real, (__m512i) sign);
        h1_imag = (__m512d) _mm512_xor_epi64((__m512i) h1_imag, (__m512i) sign);

//	h2_real = _mm256_xor_pd(h2_real, sign);
//	h2_imag = _mm256_xor_pd(h2_imag, sign);
        h2_real = (__m512d) _mm512_xor_epi64((__m512i) h2_real, (__m512i) sign);
        h2_imag = (__m512d) _mm512_xor_epi64((__m512i) h2_imag, (__m512i) sign);

	//check this
//	__m128d tmp_s_128 = _mm_loadu_pd(s_dbl);
//	tmp2 = _mm256_broadcast_pd(&tmp_s_128);

//	__m512d tmp_s = _mm512_maskz_loadu_pd (0x01 + 0x02, s_dbl);
//        tmp2 = _mm512_broadcast_f64x2(_mm512_castpd512_pd128(tmp_s));
//        tmp2 = _mm512_set4_pd(s_dbl[0],s_dbl[1], s_dbl[0],s_dbl[1]);

	tmp2 = _mm512_load_pd(&s_helper);
	tmp1 = _mm512_mul_pd(h2_imag, tmp2);

	tmp2 = _mm512_FMADDSUB_pd(h2_real, tmp2, _mm512_shuffle_pd(tmp1, tmp1, 0x55));
//check this
//	_mm_storeu_pd(s_dbl, _mm256_castpd256_pd128(tmp2));
        _mm512_mask_storeu_pd(s_dbl, 0x01 + 0x02, tmp2);

	h2_real = _mm512_set1_pd(s_dbl[0]);
	h2_imag = _mm512_set1_pd(s_dbl[1]);

	tmp1 = _mm512_mul_pd(h1_imag, y1);

	y1 = _mm512_FMADDSUB_pd(h1_real, y1, _mm512_shuffle_pd(tmp1, tmp1, 0x55));

	tmp2 = _mm512_mul_pd(h1_imag, y2);

	y2 = _mm512_FMADDSUB_pd(h1_real, y2, _mm512_shuffle_pd(tmp2, tmp2, 0x55));

	tmp3 = _mm512_mul_pd(h1_imag, y3);

	y3 = _mm512_FMADDSUB_pd(h1_real, y3, _mm512_shuffle_pd(tmp3, tmp3, 0x55));

	tmp4 = _mm512_mul_pd(h1_imag, y4);

	y4 = _mm512_FMADDSUB_pd(h1_real, y4, _mm512_shuffle_pd(tmp4, tmp4, 0x55));

	tmp1 = _mm512_mul_pd(h2_imag, x1);

	y1 = _mm512_add_pd(y1, _mm512_FMADDSUB_pd(h2_real, x1, _mm512_shuffle_pd(tmp1, tmp1, 0x55)));

	tmp2 = _mm512_mul_pd(h2_imag, x2);

	y2 = _mm512_add_pd(y2, _mm512_FMADDSUB_pd(h2_real, x2, _mm512_shuffle_pd(tmp2, tmp2, 0x55)));

	tmp3 = _mm512_mul_pd(h2_imag, x3);

	y3 = _mm512_add_pd(y3, _mm512_FMADDSUB_pd(h2_real, x3, _mm512_shuffle_pd(tmp3, tmp3, 0x55)));

	tmp4 = _mm512_mul_pd(h2_imag, x4);

	y4 = _mm512_add_pd(y4, _mm512_FMADDSUB_pd(h2_real, x4, _mm512_shuffle_pd(tmp4, tmp4, 0x55)));

	q1 = _mm512_load_pd(&q_dbl[0]);
	q2 = _mm512_load_pd(&q_dbl[8]);
	q3 = _mm512_load_pd(&q_dbl[16]);
	q4 = _mm512_load_pd(&q_dbl[24]);

	q1 = _mm512_add_pd(q1, y1);
	q2 = _mm512_add_pd(q2, y2);
	q3 = _mm512_add_pd(q3, y3);
	q4 = _mm512_add_pd(q4, y4);

	_mm512_store_pd(&q_dbl[0], q1);
	_mm512_store_pd(&q_dbl[8], q2);
	_mm512_store_pd(&q_dbl[16], q3);
	_mm512_store_pd(&q_dbl[24], q4);

	h2_real = _mm512_set1_pd(hh_dbl[(ldh+1)*2]);
	h2_imag = _mm512_set1_pd(hh_dbl[((ldh+1)*2)+1]);

	q1 = _mm512_load_pd(&q_dbl[(ldq*2)+0]);
	q2 = _mm512_load_pd(&q_dbl[(ldq*2)+8]);
	q3 = _mm512_load_pd(&q_dbl[(ldq*2)+16]);
	q4 = _mm512_load_pd(&q_dbl[(ldq*2)+24]);

	q1 = _mm512_add_pd(q1, x1);
	q2 = _mm512_add_pd(q2, x2);
	q3 = _mm512_add_pd(q3, x3);
	q4 = _mm512_add_pd(q4, x4);

	tmp1 = _mm512_mul_pd(h2_imag, y1);

	q1 = _mm512_add_pd(q1, _mm512_FMADDSUB_pd(h2_real, y1, _mm512_shuffle_pd(tmp1, tmp1, 0x55)));

	tmp2 = _mm512_mul_pd(h2_imag, y2);

//#ifdef __FMA4_
//	q2 = _mm256_add_pd(q2, _mm256_FMADDSUB_pd(h2_real, y2, _mm256_shuffle_pd(tmp2, tmp2, 0x55)));
//#else
//	q2 = _mm256_add_pd(q2, _mm256_addsub_pd( _mm256_mul_pd(h2_real, y2), _mm256_shuffle_pd(tmp2, tmp2, 0x55)));
//#endif

	q2 = _mm512_add_pd(q2, _mm512_FMADDSUB_pd(h2_real, y2, _mm512_shuffle_pd(tmp2, tmp2, 0x55)));

	tmp3 = _mm512_mul_pd(h2_imag, y3);

	q3 = _mm512_add_pd(q3, _mm512_FMADDSUB_pd(h2_real, y3, _mm512_shuffle_pd(tmp3, tmp3, 0x55)));

	tmp4 = _mm512_mul_pd(h2_imag, y4);

	q4 = _mm512_add_pd(q4, _mm512_FMADDSUB_pd(h2_real, y4, _mm512_shuffle_pd(tmp4, tmp4, 0x55)));

	_mm512_store_pd(&q_dbl[(ldq*2)+0], q1);
	_mm512_store_pd(&q_dbl[(ldq*2)+8], q2);
	_mm512_store_pd(&q_dbl[(ldq*2)+16], q3);
	_mm512_store_pd(&q_dbl[(ldq*2)+24], q4);

	for (i = 2; i < nb; i++)
	{
		q1 = _mm512_load_pd(&q_dbl[(2*i*ldq)+0]);
		q2 = _mm512_load_pd(&q_dbl[(2*i*ldq)+8]);
		q3 = _mm512_load_pd(&q_dbl[(2*i*ldq)+16]);
		q4 = _mm512_load_pd(&q_dbl[(2*i*ldq)+24]);

		h1_real = _mm512_set1_pd(hh_dbl[(i-1)*2]);
		h1_imag = _mm512_set1_pd(hh_dbl[((i-1)*2)+1]);

		tmp1 = _mm512_mul_pd(h1_imag, x1);

		q1 = _mm512_add_pd(q1, _mm512_FMADDSUB_pd(h1_real, x1, _mm512_shuffle_pd(tmp1, tmp1, 0x55)));

		tmp2 = _mm512_mul_pd(h1_imag, x2);

		q2 = _mm512_add_pd(q2, _mm512_FMADDSUB_pd(h1_real, x2, _mm512_shuffle_pd(tmp2, tmp2, 0x55)));

		tmp3 = _mm512_mul_pd(h1_imag, x3);

		q3 = _mm512_add_pd(q3, _mm512_FMADDSUB_pd(h1_real, x3, _mm512_shuffle_pd(tmp3, tmp3, 0x55)));

		tmp4 = _mm512_mul_pd(h1_imag, x4);

		q4 = _mm512_add_pd(q4, _mm512_FMADDSUB_pd(h1_real, x4, _mm512_shuffle_pd(tmp4, tmp4, 0x55)));

		h2_real = _mm512_set1_pd(hh_dbl[(ldh+i)*2]);
		h2_imag = _mm512_set1_pd(hh_dbl[((ldh+i)*2)+1]);

		tmp1 = _mm512_mul_pd(h2_imag, y1);

		q1 = _mm512_add_pd(q1, _mm512_FMADDSUB_pd(h2_real, y1, _mm512_shuffle_pd(tmp1, tmp1, 0x55)));

		tmp2 = _mm512_mul_pd(h2_imag, y2);

		q2 = _mm512_add_pd(q2, _mm512_FMADDSUB_pd(h2_real, y2, _mm512_shuffle_pd(tmp2, tmp2, 0x55)));

		tmp3 = _mm512_mul_pd(h2_imag, y3);

		q3 = _mm512_add_pd(q3, _mm512_FMADDSUB_pd(h2_real, y3, _mm512_shuffle_pd(tmp3, tmp3, 0x55)));

		tmp4 = _mm512_mul_pd(h2_imag, y4);

		q4 = _mm512_add_pd(q4, _mm512_FMADDSUB_pd(h2_real, y4, _mm512_shuffle_pd(tmp4, tmp4, 0x55)));

		_mm512_store_pd(&q_dbl[(2*i*ldq)+0], q1);
		_mm512_store_pd(&q_dbl[(2*i*ldq)+8], q2);
		_mm512_store_pd(&q_dbl[(2*i*ldq)+16], q3);
		_mm512_store_pd(&q_dbl[(2*i*ldq)+24], q4);
	}

	h1_real = _mm512_set1_pd(hh_dbl[(nb-1)*2]);
	h1_imag = _mm512_set1_pd(hh_dbl[((nb-1)*2)+1]);

	q1 = _mm512_load_pd(&q_dbl[(2*nb*ldq)+0]);
	q2 = _mm512_load_pd(&q_dbl[(2*nb*ldq)+8]);
	q3 = _mm512_load_pd(&q_dbl[(2*nb*ldq)+16]);
	q4 = _mm512_load_pd(&q_dbl[(2*nb*ldq)+24]);

	tmp1 = _mm512_mul_pd(h1_imag, x1);

	q1 = _mm512_add_pd(q1, _mm512_FMADDSUB_pd(h1_real, x1, _mm512_shuffle_pd(tmp1, tmp1, 0x55)));

	tmp2 = _mm512_mul_pd(h1_imag, x2);

	q2 = _mm512_add_pd(q2, _mm512_FMADDSUB_pd(h1_real, x2, _mm512_shuffle_pd(tmp2, tmp2, 0x55)));

	tmp3 = _mm512_mul_pd(h1_imag, x3);

	q3 = _mm512_add_pd(q3, _mm512_FMADDSUB_pd(h1_real, x3, _mm512_shuffle_pd(tmp3, tmp3, 0x55)));

	tmp4 = _mm512_mul_pd(h1_imag, x4);

	q4 = _mm512_add_pd(q4, _mm512_FMADDSUB_pd(h1_real, x4, _mm512_shuffle_pd(tmp4, tmp4, 0x55)));

	_mm512_store_pd(&q_dbl[(2*nb*ldq)+0], q1);
	_mm512_store_pd(&q_dbl[(2*nb*ldq)+8], q2);
	_mm512_store_pd(&q_dbl[(2*nb*ldq)+16], q3);
	_mm512_store_pd(&q_dbl[(2*nb*ldq)+24], q4);
}

static __forceinline void hh_trafo_complex_kernel_8_AVX512_2hv_double(double complex* q, double complex* hh, int nb, int ldq, int ldh, double complex s)
{
	double* q_dbl = (double*)q;
	double* hh_dbl = (double*)hh;
	double* s_dbl = (double*)(&s);
        double s_helper[8];

        s_helper[0] = s_dbl[0];
        s_helper[1] = s_dbl[1];
        s_helper[2] = s_dbl[0];
        s_helper[3] = s_dbl[1];
        s_helper[4] = s_dbl[0];
        s_helper[5] = s_dbl[1];
        s_helper[6] = s_dbl[0];
        s_helper[7] = s_dbl[1];

	__m512d x1, x2;
	__m512d y1, y2;
	__m512d q1, q2;
	__m512d h1_real, h1_imag, h2_real, h2_imag;
	__m512d tmp1, tmp2;
	int i=0;

	// __m512d sign = (__m512d)_mm512_set_epi64(0x8000000000000000, 0x8000000000000000, 0x8000000000000000, 0x8000000000000000, 0x8000000000000000, 0x8000000000000000, 0x8000000000000000, 0x8000000000000000);
       __m512d sign = (__m512d)_mm512_set1_epi64(0x8000000000000000);

        x1 = _mm512_load_pd(&q_dbl[(2*ldq)+0]);
	x2 = _mm512_load_pd(&q_dbl[(2*ldq)+8]);

	h2_real = _mm512_set1_pd(hh_dbl[(ldh+1)*2]);
	h2_imag = _mm512_set1_pd(hh_dbl[((ldh+1)*2)+1]);

	y1 = _mm512_load_pd(&q_dbl[0]);
	y2 = _mm512_load_pd(&q_dbl[8]);

	tmp1 = _mm512_mul_pd(h2_imag, x1);

	y1 = _mm512_add_pd(y1, _mm512_FMSUBADD_pd(h2_real, x1, _mm512_shuffle_pd(tmp1, tmp1, 0x55)));

	tmp2 = _mm512_mul_pd(h2_imag, x2);

	y2 = _mm512_add_pd(y2, _mm512_FMSUBADD_pd(h2_real, x2, _mm512_shuffle_pd(tmp2, tmp2, 0x55)));

	for (i = 2; i < nb; i++)
	{
		q1 = _mm512_load_pd(&q_dbl[(2*i*ldq)+0]);
		q2 = _mm512_load_pd(&q_dbl[(2*i*ldq)+8]);

		h1_real = _mm512_set1_pd(hh_dbl[(i-1)*2]);
		h1_imag = _mm512_set1_pd(hh_dbl[((i-1)*2)+1]);

		tmp1 = _mm512_mul_pd(h1_imag, q1);

		x1 = _mm512_add_pd(x1, _mm512_FMSUBADD_pd(h1_real, q1, _mm512_shuffle_pd(tmp1, tmp1, 0x55)));

		tmp2 = _mm512_mul_pd(h1_imag, q2);

		x2 = _mm512_add_pd(x2, _mm512_FMSUBADD_pd(h1_real, q2, _mm512_shuffle_pd(tmp2, tmp2, 0x55)));

		h2_real = _mm512_set1_pd(hh_dbl[(ldh+i)*2]);
		h2_imag = _mm512_set1_pd(hh_dbl[((ldh+i)*2)+1]);

		tmp1 = _mm512_mul_pd(h2_imag, q1);

		y1 = _mm512_add_pd(y1, _mm512_FMSUBADD_pd(h2_real, q1, _mm512_shuffle_pd(tmp1, tmp1, 0x55)));

		tmp2 = _mm512_mul_pd(h2_imag, q2);

		y2 = _mm512_add_pd(y2, _mm512_FMSUBADD_pd(h2_real, q2, _mm512_shuffle_pd(tmp2, tmp2, 0x55)));
	}

	h1_real = _mm512_set1_pd(hh_dbl[(nb-1)*2]);
	h1_imag = _mm512_set1_pd(hh_dbl[((nb-1)*2)+1]);

	q1 = _mm512_load_pd(&q_dbl[(2*nb*ldq)+0]);
	q2 = _mm512_load_pd(&q_dbl[(2*nb*ldq)+8]);

	tmp1 = _mm512_mul_pd(h1_imag, q1);

	x1 = _mm512_add_pd(x1, _mm512_FMSUBADD_pd(h1_real, q1, _mm512_shuffle_pd(tmp1, tmp1, 0x55)));

	tmp2 = _mm512_mul_pd(h1_imag, q2);

	x2 = _mm512_add_pd(x2, _mm512_FMSUBADD_pd(h1_real, q2, _mm512_shuffle_pd(tmp2, tmp2, 0x55)));

	h1_real = _mm512_set1_pd(hh_dbl[0]);
	h1_imag = _mm512_set1_pd(hh_dbl[1]);

//	h1_real = _mm512_xor_pd(h1_real, sign);
//	h1_imag = _mm512_xor_pd(h1_imag, sign);
        h1_real = (__m512d) _mm512_xor_epi64((__m512i) h1_real, (__m512i) sign);
        h1_imag = (__m512d) _mm512_xor_epi64((__m512i) h1_imag, (__m512i) sign);

	tmp1 = _mm512_mul_pd(h1_imag, x1);

	x1 = _mm512_FMADDSUB_pd(h1_real, x1, _mm512_shuffle_pd(tmp1, tmp1, 0x55));

	tmp2 = _mm512_mul_pd(h1_imag, x2);

	x2 = _mm512_FMADDSUB_pd(h1_real, x2, _mm512_shuffle_pd(tmp2, tmp2, 0x55));

	h1_real = _mm512_set1_pd(hh_dbl[ldh*2]);
	h1_imag = _mm512_set1_pd(hh_dbl[(ldh*2)+1]);
	h2_real = _mm512_set1_pd(hh_dbl[ldh*2]);
	h2_imag = _mm512_set1_pd(hh_dbl[(ldh*2)+1]);

//	h1_real = _mm512_xor_pd(h1_real, sign);
//	h1_imag = _mm512_xor_pd(h1_imag, sign);
        h1_real = (__m512d) _mm512_xor_epi64((__m512i) h1_real, (__m512i) sign);
        h1_imag = (__m512d) _mm512_xor_epi64((__m512i) h1_imag, (__m512i) sign);

//	h2_real = _mm512_xor_pd(h2_real, sign);
//	h2_imag = _mm512_xor_pd(h2_imag, sign);
        h2_real = (__m512d) _mm512_xor_epi64((__m512i) h2_real, (__m512i) sign);
        h2_imag = (__m512d) _mm512_xor_epi64((__m512i) h2_imag, (__m512i) sign);

//check this
//	__m128d tmp_s_128 = _mm_loadu_pd(s_dbl);
//	tmp2 = _mm512_broadcast_pd(&tmp_s_128);

//	__m512d tmp_s = _mm512_maskz_loadu_pd (0x01 + 0x02, s_dbl);
//       tmp2 = _mm512_broadcast_f64x2(_mm512_castpd512_pd128(tmp_s));
 //       tmp2 = _mm512_set4_pd(s_dbl[0],s_dbl[1], s_dbl[0],s_dbl[1]);

        tmp2 = _mm512_load_pd(&s_helper);

	tmp1 = _mm512_mul_pd(h2_imag, tmp2);

	tmp2 = _mm512_FMADDSUB_pd(h2_real, tmp2, _mm512_shuffle_pd(tmp1, tmp1, 0x55));
//check this
//	_mm_storeu_pd(s_dbl, _mm512_castpd512_pd128(tmp2));
        _mm512_mask_storeu_pd(s_dbl, 0x01 + 0x02, tmp2);

	h2_real = _mm512_set1_pd(s_dbl[0]);
	h2_imag = _mm512_set1_pd(s_dbl[1]);

	tmp1 = _mm512_mul_pd(h1_imag, y1);

	y1 = _mm512_FMADDSUB_pd(h1_real, y1, _mm512_shuffle_pd(tmp1, tmp1, 0x55));

	tmp2 = _mm512_mul_pd(h1_imag, y2);

	y2 = _mm512_FMADDSUB_pd(h1_real, y2, _mm512_shuffle_pd(tmp2, tmp2, 0x55));

	tmp1 = _mm512_mul_pd(h2_imag, x1);

	y1 = _mm512_add_pd(y1, _mm512_FMADDSUB_pd(h2_real, x1, _mm512_shuffle_pd(tmp1, tmp1, 0x55)));

	tmp2 = _mm512_mul_pd(h2_imag, x2);

	y2 = _mm512_add_pd(y2, _mm512_FMADDSUB_pd(h2_real, x2, _mm512_shuffle_pd(tmp2, tmp2, 0x55)));

	q1 = _mm512_load_pd(&q_dbl[0]);
	q2 = _mm512_load_pd(&q_dbl[8]);

	q1 = _mm512_add_pd(q1, y1);
	q2 = _mm512_add_pd(q2, y2);

	_mm512_store_pd(&q_dbl[0], q1);
	_mm512_store_pd(&q_dbl[8], q2);

	h2_real = _mm512_set1_pd(hh_dbl[(ldh+1)*2]);
	h2_imag = _mm512_set1_pd(hh_dbl[((ldh+1)*2)+1]);

	q1 = _mm512_load_pd(&q_dbl[(ldq*2)+0]);
	q2 = _mm512_load_pd(&q_dbl[(ldq*2)+8]);

	q1 = _mm512_add_pd(q1, x1);
	q2 = _mm512_add_pd(q2, x2);

	tmp1 = _mm512_mul_pd(h2_imag, y1);

	q1 = _mm512_add_pd(q1, _mm512_FMADDSUB_pd(h2_real, y1, _mm512_shuffle_pd(tmp1, tmp1, 0x55)));

	tmp2 = _mm512_mul_pd(h2_imag, y2);

	q2 = _mm512_add_pd(q2, _mm512_FMADDSUB_pd(h2_real, y2, _mm512_shuffle_pd(tmp2, tmp2, 0x55)));

	_mm512_store_pd(&q_dbl[(ldq*2)+0], q1);
	_mm512_store_pd(&q_dbl[(ldq*2)+8], q2);

	for (i = 2; i < nb; i++)
	{
		q1 = _mm512_load_pd(&q_dbl[(2*i*ldq)+0]);
		q2 = _mm512_load_pd(&q_dbl[(2*i*ldq)+8]);

		h1_real = _mm512_set1_pd(hh_dbl[(i-1)*2]);
		h1_imag = _mm512_set1_pd(hh_dbl[((i-1)*2)+1]);

		tmp1 = _mm512_mul_pd(h1_imag, x1);

		q1 = _mm512_add_pd(q1, _mm512_FMADDSUB_pd(h1_real, x1, _mm512_shuffle_pd(tmp1, tmp1, 0x55)));

		tmp2 = _mm512_mul_pd(h1_imag, x2);

		q2 = _mm512_add_pd(q2, _mm512_FMADDSUB_pd(h1_real, x2, _mm512_shuffle_pd(tmp2, tmp2, 0x55)));

		h2_real = _mm512_set1_pd(hh_dbl[(ldh+i)*2]);
		h2_imag = _mm512_set1_pd(hh_dbl[((ldh+i)*2)+1]);

		tmp1 = _mm512_mul_pd(h2_imag, y1);

		q1 = _mm512_add_pd(q1, _mm512_FMADDSUB_pd(h2_real, y1, _mm512_shuffle_pd(tmp1, tmp1, 0x55)));

		tmp2 = _mm512_mul_pd(h2_imag, y2);

		q2 = _mm512_add_pd(q2, _mm512_FMADDSUB_pd(h2_real, y2, _mm512_shuffle_pd(tmp2, tmp2, 0x55)));

		_mm512_store_pd(&q_dbl[(2*i*ldq)+0], q1);
		_mm512_store_pd(&q_dbl[(2*i*ldq)+8], q2);
	}
	h1_real = _mm512_set1_pd(hh_dbl[(nb-1)*2]);
	h1_imag = _mm512_set1_pd(hh_dbl[((nb-1)*2)+1]);

	q1 = _mm512_load_pd(&q_dbl[(2*nb*ldq)+0]);
	q2 = _mm512_load_pd(&q_dbl[(2*nb*ldq)+8]);

	tmp1 = _mm512_mul_pd(h1_imag, x1);

	q1 = _mm512_add_pd(q1, _mm512_FMADDSUB_pd(h1_real, x1, _mm512_shuffle_pd(tmp1, tmp1, 0x55)));

	tmp2 = _mm512_mul_pd(h1_imag, x2);

	q2 = _mm512_add_pd(q2, _mm512_FMADDSUB_pd(h1_real, x2, _mm512_shuffle_pd(tmp2, tmp2, 0x55)));

	_mm512_store_pd(&q_dbl[(2*nb*ldq)+0], q1);
	_mm512_store_pd(&q_dbl[(2*nb*ldq)+8], q2);
}

