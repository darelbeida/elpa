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
//
// --------------------------------------------------------------------------------------------------
//
// This file contains the compute intensive kernels for the Householder transformations.
// It should be compiled with the highest possible optimization level.
//
// On Intel Nehalem or Intel Westmere or AMD Magny Cours use -O3 -msse3
// On Intel Sandy Bridge use -O3 -mavx
//
// Copyright of the original code rests with the authors inside the ELPA
// consortium. The copyright of any additional modifications shall rest
// with their original authors, but shall adhere to the licensing terms
// distributed along with the original code in the file "COPYING".
//
// Author: Alexander Heinecke (alexander.heinecke@mytum.de)
// Adapted for building a shared-library by Andreas Marek, MPCDF (andreas.marek@mpcdf.mpg.de)
// --------------------------------------------------------------------------------------------------

#include "config-f90.h"

#include <x86intrin.h>

#define __forceinline __attribute__((always_inline)) static

#ifdef HAVE_SSE_INTRINSICS
#undef __AVX__
#endif

//Forward declaration
__forceinline void hh_trafo_kernel_2_SSE_4hv(double* q, double* hh, int nb, int ldq, int ldh, double s_1_2, double s_1_3, double s_2_3, double s_1_4, double s_2_4, double s_3_4);
__forceinline void hh_trafo_kernel_4_SSE_4hv(double* q, double* hh, int nb, int ldq, int ldh, double s_1_2, double s_1_3, double s_2_3, double s_1_4, double s_2_4, double s_3_4);
__forceinline void hh_trafo_kernel_6_SSE_4hv(double* q, double* hh, int nb, int ldq, int ldh, double s_1_2, double s_1_3, double s_2_3, double s_1_4, double s_2_4, double s_3_4);

/*
!f>#ifdef HAVE_SSE_INTRINSICS
!f> interface
!f>   subroutine quad_hh_trafo_real_sse_4hv(q, hh, pnb, pnq, pldq, pldh) bind(C, name="quad_hh_trafo_real_sse_4hv")
!f>     use, intrinsic :: iso_c_binding
!f>     integer(kind=c_int)     :: pnb, pnq, pldq, pldh
!f>     real(kind=c_double)     :: q(*)
!f>     real(kind=c_double)     :: hh(pnb,6)
!f>   end subroutine
!f> end interface
!f>#endif
*/

void quad_hh_trafo_real_sse_4hv(double* q, double* hh, int* pnb, int* pnq, int* pldq, int* pldh);

void quad_hh_trafo_real_sse_4hv(double* q, double* hh, int* pnb, int* pnq, int* pldq, int* pldh)
{
	int i;
	int nb = *pnb;
	int nq = *pldq;
	int ldq = *pldq;
	int ldh = *pldh;

	// calculating scalar products to compute
	// 4 householder vectors simultaneously
	double s_1_2 = hh[(ldh)+1];
	double s_1_3 = hh[(ldh*2)+2];
	double s_2_3 = hh[(ldh*2)+1];
	double s_1_4 = hh[(ldh*3)+3];
	double s_2_4 = hh[(ldh*3)+2];
	double s_3_4 = hh[(ldh*3)+1];

	// calculate scalar product of first and fourth householder vector
	// loop counter = 2
	s_1_2 += hh[2-1] * hh[(2+ldh)];
	s_2_3 += hh[(ldh)+2-1] * hh[2+(ldh*2)];
	s_3_4 += hh[(ldh*2)+2-1] * hh[2+(ldh*3)];

	// loop counter = 3
	s_1_2 += hh[3-1] * hh[(3+ldh)];
	s_2_3 += hh[(ldh)+3-1] * hh[3+(ldh*2)];
	s_3_4 += hh[(ldh*2)+3-1] * hh[3+(ldh*3)];

	s_1_3 += hh[3-2] * hh[3+(ldh*2)];
	s_2_4 += hh[(ldh*1)+3-2] * hh[3+(ldh*3)];

	#pragma ivdep
	for (i = 4; i < nb; i++)
	{
		s_1_2 += hh[i-1] * hh[(i+ldh)];
		s_2_3 += hh[(ldh)+i-1] * hh[i+(ldh*2)];
		s_3_4 += hh[(ldh*2)+i-1] * hh[i+(ldh*3)];

		s_1_3 += hh[i-2] * hh[i+(ldh*2)];
		s_2_4 += hh[(ldh*1)+i-2] * hh[i+(ldh*3)];

		s_1_4 += hh[i-3] * hh[i+(ldh*3)];
	}

//	printf("s_1_2: %f\n", s_1_2);
//	printf("s_1_3: %f\n", s_1_3);
//	printf("s_2_3: %f\n", s_2_3);
//	printf("s_1_4: %f\n", s_1_4);
//	printf("s_2_4: %f\n", s_2_4);
//	printf("s_3_4: %f\n", s_3_4);

	// Production level kernel calls with padding
	for (i = 0; i < nq-4; i+=6)
	{
		hh_trafo_kernel_6_SSE_4hv(&q[i], hh, nb, ldq, ldh, s_1_2, s_1_3, s_2_3, s_1_4, s_2_4, s_3_4);
	}
	if (nq == i)
	{
		return;
	}
	else
	{
		if (nq-i > 2)
		{
			hh_trafo_kernel_4_SSE_4hv(&q[i], hh, nb, ldq, ldh, s_1_2, s_1_3, s_2_3, s_1_4, s_2_4, s_3_4);
		}
		else
		{
			hh_trafo_kernel_2_SSE_4hv(&q[i], hh, nb, ldq, ldh, s_1_2, s_1_3, s_2_3, s_1_4, s_2_4, s_3_4);
		}
	}
}

//#if 0
//void quad_hh_trafo_fast_(double* q, double* hh, int* pnb, int* pnq, int* pldq, int* pldh)
//{
//	int i;
//	int nb = *pnb;
//	int nq = *pldq;
//	int ldq = *pldq;
//	int ldh = *pldh;
//
//	// calculating scalar products to compute
//	// 4 householder vectors simultaneously
//	double s_1_2 = hh[(ldh)+1];
//	double s_1_3 = hh[(ldh*2)+2];
//	double s_2_3 = hh[(ldh*2)+1];
//	double s_1_4 = hh[(ldh*3)+3];
//	double s_2_4 = hh[(ldh*3)+2];
//	double s_3_4 = hh[(ldh*3)+1];
//
//	// calculate scalar product of first and fourth householder vector
//	// loop counter = 2
//	s_1_2 += hh[2-1] * hh[(2+ldh)];
//	s_2_3 += hh[(ldh)+2-1] * hh[2+(ldh*2)];
//	s_3_4 += hh[(ldh*2)+2-1] * hh[2+(ldh*3)];
//
//	// loop counter = 3
//	s_1_2 += hh[3-1] * hh[(3+ldh)];
//	s_2_3 += hh[(ldh)+3-1] * hh[3+(ldh*2)];
//	s_3_4 += hh[(ldh*2)+3-1] * hh[3+(ldh*3)];
//
//	s_1_3 += hh[3-2] * hh[3+(ldh*2)];
//	s_2_4 += hh[(ldh*1)+3-2] * hh[3+(ldh*3)];
//
//	#pragma ivdep
//	for (i = 4; i < nb; i++)
//	{
//		s_1_2 += hh[i-1] * hh[(i+ldh)];
//		s_2_3 += hh[(ldh)+i-1] * hh[i+(ldh*2)];
//		s_3_4 += hh[(ldh*2)+i-1] * hh[i+(ldh*3)];
//
//		s_1_3 += hh[i-2] * hh[i+(ldh*2)];
//		s_2_4 += hh[(ldh*1)+i-2] * hh[i+(ldh*3)];
//
//		s_1_4 += hh[i-3] * hh[i+(ldh*3)];
//	}
//
//	// Production level kernel calls with padding
//#ifdef __AVX__
//	for (i = 0; i < nq; i+=12)
//	{
//		hh_trafo_kernel_12_AVX_4hv(&q[i], hh, nb, ldq, ldh, s_1_2, s_1_3, s_2_3, s_1_4, s_2_4, s_3_4);
//	}
//#else
//	for (i = 0; i < nq; i+=6)
//	{
//		hh_trafo_kernel_6_SSE_4hv(&q[i], hh, nb, ldq, ldh, s_1_2, s_1_3, s_2_3, s_1_4, s_2_4, s_3_4);
//	}
//#endif
//}
//#endif


/**
 * Unrolled kernel that computes
 * 6 rows of Q simultaneously, a
 * matrix vector product with two householder
 * vectors + a rank 1 update is performed
 */
__forceinline void hh_trafo_kernel_6_SSE_4hv(double* q, double* hh, int nb, int ldq, int ldh, double s_1_2, double s_1_3, double s_2_3, double s_1_4, double s_2_4, double s_3_4)
{
	/////////////////////////////////////////////////////
	// Matrix Vector Multiplication, Q [6 x nb+3] * hh
	// hh contains four householder vectors
	/////////////////////////////////////////////////////
	int i;

	__m128d a1_1 = _mm_load_pd(&q[ldq*3]);
	__m128d a2_1 = _mm_load_pd(&q[ldq*2]);
	__m128d a3_1 = _mm_load_pd(&q[ldq]);
	__m128d a4_1 = _mm_load_pd(&q[0]);

	__m128d h_2_1 = _mm_loaddup_pd(&hh[ldh+1]);
	__m128d h_3_2 = _mm_loaddup_pd(&hh[(ldh*2)+1]);
	__m128d h_3_1 = _mm_loaddup_pd(&hh[(ldh*2)+2]);
	__m128d h_4_3 = _mm_loaddup_pd(&hh[(ldh*3)+1]);
	__m128d h_4_2 = _mm_loaddup_pd(&hh[(ldh*3)+2]);
	__m128d h_4_1 = _mm_loaddup_pd(&hh[(ldh*3)+3]);

	register __m128d w1 = _mm_add_pd(a4_1, _mm_mul_pd(a3_1, h_4_3));
	w1 = _mm_add_pd(w1, _mm_mul_pd(a2_1, h_4_2));
	w1 = _mm_add_pd(w1, _mm_mul_pd(a1_1, h_4_1));
	register __m128d z1 = _mm_add_pd(a3_1, _mm_mul_pd(a2_1, h_3_2));
	z1 = _mm_add_pd(z1, _mm_mul_pd(a1_1, h_3_1));
	register __m128d y1 = _mm_add_pd(a2_1, _mm_mul_pd(a1_1, h_2_1));
	register __m128d x1 = a1_1;

	__m128d a1_2 = _mm_load_pd(&q[(ldq*3)+2]);
	__m128d a2_2 = _mm_load_pd(&q[(ldq*2)+2]);
	__m128d a3_2 = _mm_load_pd(&q[ldq+2]);
	__m128d a4_2 = _mm_load_pd(&q[0+2]);

	register __m128d w2 = _mm_add_pd(a4_2, _mm_mul_pd(a3_2, h_4_3));
	w2 = _mm_add_pd(w2, _mm_mul_pd(a2_2, h_4_2));
	w2 = _mm_add_pd(w2, _mm_mul_pd(a1_2, h_4_1));
	register __m128d z2 = _mm_add_pd(a3_2, _mm_mul_pd(a2_2, h_3_2));
	z2 = _mm_add_pd(z2, _mm_mul_pd(a1_2, h_3_1));
	register __m128d y2 = _mm_add_pd(a2_2, _mm_mul_pd(a1_2, h_2_1));
	register __m128d x2 = a1_2;

	__m128d a1_3 = _mm_load_pd(&q[(ldq*3)+4]);
	__m128d a2_3 = _mm_load_pd(&q[(ldq*2)+4]);
	__m128d a3_3 = _mm_load_pd(&q[ldq+4]);
	__m128d a4_3 = _mm_load_pd(&q[0+4]);

	register __m128d w3 = _mm_add_pd(a4_3, _mm_mul_pd(a3_3, h_4_3));
	w3 = _mm_add_pd(w3, _mm_mul_pd(a2_3, h_4_2));
	w3 = _mm_add_pd(w3, _mm_mul_pd(a1_3, h_4_1));
	register __m128d z3 = _mm_add_pd(a3_3, _mm_mul_pd(a2_3, h_3_2));
	z3 = _mm_add_pd(z3, _mm_mul_pd(a1_3, h_3_1));
	register __m128d y3 = _mm_add_pd(a2_3, _mm_mul_pd(a1_3, h_2_1));
	register __m128d x3 = a1_3;

	__m128d q1;
	__m128d q2;
	__m128d q3;

	__m128d h1;
	__m128d h2;
	__m128d h3;
	__m128d h4;

	for(i = 4; i < nb; i++)
	{
		h1 = _mm_loaddup_pd(&hh[i-3]);
		q1 = _mm_load_pd(&q[i*ldq]);
		q2 = _mm_load_pd(&q[(i*ldq)+2]);
		q3 = _mm_load_pd(&q[(i*ldq)+4]);

		x1 = _mm_add_pd(x1, _mm_mul_pd(q1,h1));
		x2 = _mm_add_pd(x2, _mm_mul_pd(q2,h1));
		x3 = _mm_add_pd(x3, _mm_mul_pd(q3,h1));

		h2 = _mm_loaddup_pd(&hh[ldh+i-2]);

		y1 = _mm_add_pd(y1, _mm_mul_pd(q1,h2));
		y2 = _mm_add_pd(y2, _mm_mul_pd(q2,h2));
		y3 = _mm_add_pd(y3, _mm_mul_pd(q3,h2));

		h3 = _mm_loaddup_pd(&hh[(ldh*2)+i-1]);

		z1 = _mm_add_pd(z1, _mm_mul_pd(q1,h3));
		z2 = _mm_add_pd(z2, _mm_mul_pd(q2,h3));
		z3 = _mm_add_pd(z3, _mm_mul_pd(q3,h3));

		h4 = _mm_loaddup_pd(&hh[(ldh*3)+i]);

		w1 = _mm_add_pd(w1, _mm_mul_pd(q1,h4));
		w2 = _mm_add_pd(w2, _mm_mul_pd(q2,h4));
		w3 = _mm_add_pd(w3, _mm_mul_pd(q3,h4));
	}

	h1 = _mm_loaddup_pd(&hh[nb-3]);

	q1 = _mm_load_pd(&q[nb*ldq]);
	q2 = _mm_load_pd(&q[(nb*ldq)+2]);
	q3 = _mm_load_pd(&q[(nb*ldq)+4]);

	x1 = _mm_add_pd(x1, _mm_mul_pd(q1,h1));
	x2 = _mm_add_pd(x2, _mm_mul_pd(q2,h1));
	x3 = _mm_add_pd(x3, _mm_mul_pd(q3,h1));

	h2 = _mm_loaddup_pd(&hh[ldh+nb-2]);

	y1 = _mm_add_pd(y1, _mm_mul_pd(q1,h2));
	y2 = _mm_add_pd(y2, _mm_mul_pd(q2,h2));
	y3 = _mm_add_pd(y3, _mm_mul_pd(q3,h2));

	h3 = _mm_loaddup_pd(&hh[(ldh*2)+nb-1]);

	z1 = _mm_add_pd(z1, _mm_mul_pd(q1,h3));
	z2 = _mm_add_pd(z2, _mm_mul_pd(q2,h3));
	z3 = _mm_add_pd(z3, _mm_mul_pd(q3,h3));

	h1 = _mm_loaddup_pd(&hh[nb-2]);

	q1 = _mm_load_pd(&q[(nb+1)*ldq]);
	q2 = _mm_load_pd(&q[((nb+1)*ldq)+2]);
	q3 = _mm_load_pd(&q[((nb+1)*ldq)+4]);

	x1 = _mm_add_pd(x1, _mm_mul_pd(q1,h1));
	x2 = _mm_add_pd(x2, _mm_mul_pd(q2,h1));
	x3 = _mm_add_pd(x3, _mm_mul_pd(q3,h1));

	h2 = _mm_loaddup_pd(&hh[(ldh*1)+nb-1]);

	y1 = _mm_add_pd(y1, _mm_mul_pd(q1,h2));
	y2 = _mm_add_pd(y2, _mm_mul_pd(q2,h2));
	y3 = _mm_add_pd(y3, _mm_mul_pd(q3,h2));

	h1 = _mm_loaddup_pd(&hh[nb-1]);

	q1 = _mm_load_pd(&q[(nb+2)*ldq]);
	q2 = _mm_load_pd(&q[((nb+2)*ldq)+2]);
	q3 = _mm_load_pd(&q[((nb+2)*ldq)+4]);

	x1 = _mm_add_pd(x1, _mm_mul_pd(q1,h1));
	x2 = _mm_add_pd(x2, _mm_mul_pd(q2,h1));
	x3 = _mm_add_pd(x3, _mm_mul_pd(q3,h1));

	/////////////////////////////////////////////////////
	// Rank-1 update of Q [6 x nb+3]
	/////////////////////////////////////////////////////

	__m128d tau1 = _mm_loaddup_pd(&hh[0]);

	h1 = tau1;
	x1 = _mm_mul_pd(x1, h1);
	x2 = _mm_mul_pd(x2, h1);
	x3 = _mm_mul_pd(x3, h1);

	__m128d tau2 = _mm_loaddup_pd(&hh[ldh]);
	__m128d vs_1_2 = _mm_loaddup_pd(&s_1_2);

	h1 = tau2;
	h2 = _mm_mul_pd(h1, vs_1_2);

	y1 = _mm_sub_pd(_mm_mul_pd(y1,h1), _mm_mul_pd(x1,h2));
	y2 = _mm_sub_pd(_mm_mul_pd(y2,h1), _mm_mul_pd(x2,h2));
	y3 = _mm_sub_pd(_mm_mul_pd(y3,h1), _mm_mul_pd(x3,h2));

	__m128d tau3 = _mm_loaddup_pd(&hh[ldh*2]);
	__m128d vs_1_3 = _mm_loaddup_pd(&s_1_3);
	__m128d vs_2_3 = _mm_loaddup_pd(&s_2_3);

	h1 = tau3;
	h2 = _mm_mul_pd(h1, vs_1_3);
	h3 = _mm_mul_pd(h1, vs_2_3);

	z1 = _mm_sub_pd(_mm_mul_pd(z1,h1), _mm_add_pd(_mm_mul_pd(y1,h3), _mm_mul_pd(x1,h2)));
	z2 = _mm_sub_pd(_mm_mul_pd(z2,h1), _mm_add_pd(_mm_mul_pd(y2,h3), _mm_mul_pd(x2,h2)));
	z3 = _mm_sub_pd(_mm_mul_pd(z3,h1), _mm_add_pd(_mm_mul_pd(y3,h3), _mm_mul_pd(x3,h2)));

	__m128d tau4 = _mm_loaddup_pd(&hh[ldh*3]);
	__m128d vs_1_4 = _mm_loaddup_pd(&s_1_4);
	__m128d vs_2_4 = _mm_loaddup_pd(&s_2_4);
	__m128d vs_3_4 = _mm_loaddup_pd(&s_3_4);

	h1 = tau4;
	h2 = _mm_mul_pd(h1, vs_1_4);
	h3 = _mm_mul_pd(h1, vs_2_4);
	h4 = _mm_mul_pd(h1, vs_3_4);

	w1 = _mm_sub_pd(_mm_mul_pd(w1,h1), _mm_add_pd(_mm_mul_pd(z1,h4), _mm_add_pd(_mm_mul_pd(y1,h3), _mm_mul_pd(x1,h2))));
	w2 = _mm_sub_pd(_mm_mul_pd(w2,h1), _mm_add_pd(_mm_mul_pd(z2,h4), _mm_add_pd(_mm_mul_pd(y2,h3), _mm_mul_pd(x2,h2))));
	w3 = _mm_sub_pd(_mm_mul_pd(w3,h1), _mm_add_pd(_mm_mul_pd(z3,h4), _mm_add_pd(_mm_mul_pd(y3,h3), _mm_mul_pd(x3,h2))));

	q1 = _mm_load_pd(&q[0]);
	q2 = _mm_load_pd(&q[2]);
	q3 = _mm_load_pd(&q[4]);
	q1 = _mm_sub_pd(q1, w1);
	q2 = _mm_sub_pd(q2, w2);
	q3 = _mm_sub_pd(q3, w3);
	_mm_store_pd(&q[0],q1);
	_mm_store_pd(&q[2],q2);
	_mm_store_pd(&q[4],q3);

	h4 = _mm_loaddup_pd(&hh[(ldh*3)+1]);
	q1 = _mm_load_pd(&q[ldq]);
	q2 = _mm_load_pd(&q[ldq+2]);
	q3 = _mm_load_pd(&q[ldq+4]);

	q1 = _mm_sub_pd(q1, _mm_add_pd(z1, _mm_mul_pd(w1, h4)));
	q2 = _mm_sub_pd(q2, _mm_add_pd(z2, _mm_mul_pd(w2, h4)));
	q3 = _mm_sub_pd(q3, _mm_add_pd(z3, _mm_mul_pd(w3, h4)));

	_mm_store_pd(&q[ldq],q1);
	_mm_store_pd(&q[ldq+2],q2);
	_mm_store_pd(&q[ldq+4],q3);

	h4 = _mm_loaddup_pd(&hh[(ldh*3)+2]);
	q1 = _mm_load_pd(&q[ldq*2]);
	q2 = _mm_load_pd(&q[(ldq*2)+2]);
	q3 = _mm_load_pd(&q[(ldq*2)+4]);
	q1 = _mm_sub_pd(q1, y1);
	q2 = _mm_sub_pd(q2, y2);
	q3 = _mm_sub_pd(q3, y3);

	q1 = _mm_sub_pd(q1, _mm_mul_pd(w1, h4));
	q2 = _mm_sub_pd(q2, _mm_mul_pd(w2, h4));
	q3 = _mm_sub_pd(q3, _mm_mul_pd(w3, h4));
	h3 = _mm_loaddup_pd(&hh[(ldh*2)+1]);

	q1 = _mm_sub_pd(q1, _mm_mul_pd(z1, h3));
	q2 = _mm_sub_pd(q2, _mm_mul_pd(z2, h3));
	q3 = _mm_sub_pd(q3, _mm_mul_pd(z3, h3));

	_mm_store_pd(&q[ldq*2],q1);
	_mm_store_pd(&q[(ldq*2)+2],q2);
	_mm_store_pd(&q[(ldq*2)+4],q3);

	h4 = _mm_loaddup_pd(&hh[(ldh*3)+3]);
	q1 = _mm_load_pd(&q[ldq*3]);
	q2 = _mm_load_pd(&q[(ldq*3)+2]);
	q3 = _mm_load_pd(&q[(ldq*3)+4]);
	q1 = _mm_sub_pd(q1, x1);
	q2 = _mm_sub_pd(q2, x2);
	q3 = _mm_sub_pd(q3, x3);

	q1 = _mm_sub_pd(q1, _mm_mul_pd(w1, h4));
	q2 = _mm_sub_pd(q2, _mm_mul_pd(w2, h4));
	q3 = _mm_sub_pd(q3, _mm_mul_pd(w3, h4));

	h2 = _mm_loaddup_pd(&hh[ldh+1]);

	q1 = _mm_sub_pd(q1, _mm_mul_pd(y1, h2));
	q2 = _mm_sub_pd(q2, _mm_mul_pd(y2, h2));
	q3 = _mm_sub_pd(q3, _mm_mul_pd(y3, h2));

	h3 = _mm_loaddup_pd(&hh[(ldh*2)+2]);

	q1 = _mm_sub_pd(q1, _mm_mul_pd(z1, h3));
	q2 = _mm_sub_pd(q2, _mm_mul_pd(z2, h3));
	q3 = _mm_sub_pd(q3, _mm_mul_pd(z3, h3));
	_mm_store_pd(&q[ldq*3], q1);
	_mm_store_pd(&q[(ldq*3)+2], q2);
	_mm_store_pd(&q[(ldq*3)+4], q3);

	for (i = 4; i < nb; i++)
	{
		h1 = _mm_loaddup_pd(&hh[i-3]);

		q1 = _mm_load_pd(&q[i*ldq]);
		q2 = _mm_load_pd(&q[(i*ldq)+2]);
		q3 = _mm_load_pd(&q[(i*ldq)+4]);

		q1 = _mm_sub_pd(q1, _mm_mul_pd(x1,h1));
		q2 = _mm_sub_pd(q2, _mm_mul_pd(x2,h1));
		q3 = _mm_sub_pd(q3, _mm_mul_pd(x3,h1));

		h2 = _mm_loaddup_pd(&hh[ldh+i-2]);

		q1 = _mm_sub_pd(q1, _mm_mul_pd(y1,h2));
		q2 = _mm_sub_pd(q2, _mm_mul_pd(y2,h2));
		q3 = _mm_sub_pd(q3, _mm_mul_pd(y3,h2));

		h3 = _mm_loaddup_pd(&hh[(ldh*2)+i-1]);

		q1 = _mm_sub_pd(q1, _mm_mul_pd(z1,h3));
		q2 = _mm_sub_pd(q2, _mm_mul_pd(z2,h3));
		q3 = _mm_sub_pd(q3, _mm_mul_pd(z3,h3));

		h4 = _mm_loaddup_pd(&hh[(ldh*3)+i]);

		q1 = _mm_sub_pd(q1, _mm_mul_pd(w1,h4));
		q2 = _mm_sub_pd(q2, _mm_mul_pd(w2,h4));
		q3 = _mm_sub_pd(q3, _mm_mul_pd(w3,h4));

		_mm_store_pd(&q[i*ldq],q1);
		_mm_store_pd(&q[(i*ldq)+2],q2);
		_mm_store_pd(&q[(i*ldq)+4],q3);
	}

	h1 = _mm_loaddup_pd(&hh[nb-3]);
	q1 = _mm_load_pd(&q[nb*ldq]);
	q2 = _mm_load_pd(&q[(nb*ldq)+2]);
	q3 = _mm_load_pd(&q[(nb*ldq)+4]);

	q1 = _mm_sub_pd(q1, _mm_mul_pd(x1, h1));
	q2 = _mm_sub_pd(q2, _mm_mul_pd(x2, h1));
	q3 = _mm_sub_pd(q3, _mm_mul_pd(x3, h1));

	h2 = _mm_loaddup_pd(&hh[ldh+nb-2]);

	q1 = _mm_sub_pd(q1, _mm_mul_pd(y1, h2));
	q2 = _mm_sub_pd(q2, _mm_mul_pd(y2, h2));
	q3 = _mm_sub_pd(q3, _mm_mul_pd(y3, h2));

	h3 = _mm_loaddup_pd(&hh[(ldh*2)+nb-1]);

	q1 = _mm_sub_pd(q1, _mm_mul_pd(z1, h3));
	q2 = _mm_sub_pd(q2, _mm_mul_pd(z2, h3));
	q3 = _mm_sub_pd(q3, _mm_mul_pd(z3, h3));

	_mm_store_pd(&q[nb*ldq],q1);
	_mm_store_pd(&q[(nb*ldq)+2],q2);
	_mm_store_pd(&q[(nb*ldq)+4],q3);

	h1 = _mm_loaddup_pd(&hh[nb-2]);
	q1 = _mm_load_pd(&q[(nb+1)*ldq]);
	q2 = _mm_load_pd(&q[((nb+1)*ldq)+2]);
	q3 = _mm_load_pd(&q[((nb+1)*ldq)+4]);

	q1 = _mm_sub_pd(q1, _mm_mul_pd(x1, h1));
	q2 = _mm_sub_pd(q2, _mm_mul_pd(x2, h1));
	q3 = _mm_sub_pd(q3, _mm_mul_pd(x3, h1));

	h2 = _mm_loaddup_pd(&hh[ldh+nb-1]);

	q1 = _mm_sub_pd(q1, _mm_mul_pd(y1, h2));
	q2 = _mm_sub_pd(q2, _mm_mul_pd(y2, h2));
	q3 = _mm_sub_pd(q3, _mm_mul_pd(y3, h2));

	_mm_store_pd(&q[(nb+1)*ldq],q1);
	_mm_store_pd(&q[((nb+1)*ldq)+2],q2);
	_mm_store_pd(&q[((nb+1)*ldq)+4],q3);

	h1 = _mm_loaddup_pd(&hh[nb-1]);
	q1 = _mm_load_pd(&q[(nb+2)*ldq]);
	q2 = _mm_load_pd(&q[((nb+2)*ldq)+2]);
	q3 = _mm_load_pd(&q[((nb+2)*ldq)+4]);

	q1 = _mm_sub_pd(q1, _mm_mul_pd(x1, h1));
	q2 = _mm_sub_pd(q2, _mm_mul_pd(x2, h1));
	q3 = _mm_sub_pd(q3, _mm_mul_pd(x3, h1));

	_mm_store_pd(&q[(nb+2)*ldq],q1);
	_mm_store_pd(&q[((nb+2)*ldq)+2],q2);
	_mm_store_pd(&q[((nb+2)*ldq)+4],q3);
}

/**
 * Unrolled kernel that computes
 * 4 rows of Q simultaneously, a
 * matrix vector product with two householder
 * vectors + a rank 1 update is performed
 */
__forceinline void hh_trafo_kernel_4_SSE_4hv(double* q, double* hh, int nb, int ldq, int ldh, double s_1_2, double s_1_3, double s_2_3, double s_1_4, double s_2_4, double s_3_4)
{
	/////////////////////////////////////////////////////
	// Matrix Vector Multiplication, Q [4 x nb+3] * hh
	// hh contains four householder vectors
	/////////////////////////////////////////////////////
	int i;

	__m128d a1_1 = _mm_load_pd(&q[ldq*3]);
	__m128d a2_1 = _mm_load_pd(&q[ldq*2]);
	__m128d a3_1 = _mm_load_pd(&q[ldq]);
	__m128d a4_1 = _mm_load_pd(&q[0]);

	__m128d h_2_1 = _mm_loaddup_pd(&hh[ldh+1]);
	__m128d h_3_2 = _mm_loaddup_pd(&hh[(ldh*2)+1]);
	__m128d h_3_1 = _mm_loaddup_pd(&hh[(ldh*2)+2]);
	__m128d h_4_3 = _mm_loaddup_pd(&hh[(ldh*3)+1]);
	__m128d h_4_2 = _mm_loaddup_pd(&hh[(ldh*3)+2]);
	__m128d h_4_1 = _mm_loaddup_pd(&hh[(ldh*3)+3]);

	__m128d w1 = _mm_add_pd(a4_1, _mm_mul_pd(a3_1, h_4_3));
	w1 = _mm_add_pd(w1, _mm_mul_pd(a2_1, h_4_2));
	w1 = _mm_add_pd(w1, _mm_mul_pd(a1_1, h_4_1));
	__m128d z1 = _mm_add_pd(a3_1, _mm_mul_pd(a2_1, h_3_2));
	z1 = _mm_add_pd(z1, _mm_mul_pd(a1_1, h_3_1));
	__m128d y1 = _mm_add_pd(a2_1, _mm_mul_pd(a1_1, h_2_1));
	__m128d x1 = a1_1;

	__m128d a1_2 = _mm_load_pd(&q[(ldq*3)+2]);
	__m128d a2_2 = _mm_load_pd(&q[(ldq*2)+2]);
	__m128d a3_2 = _mm_load_pd(&q[ldq+2]);
	__m128d a4_2 = _mm_load_pd(&q[0+2]);

	__m128d w2 = _mm_add_pd(a4_2, _mm_mul_pd(a3_2, h_4_3));
	w2 = _mm_add_pd(w2, _mm_mul_pd(a2_2, h_4_2));
	w2 = _mm_add_pd(w2, _mm_mul_pd(a1_2, h_4_1));
	__m128d z2 = _mm_add_pd(a3_2, _mm_mul_pd(a2_2, h_3_2));
	z2 = _mm_add_pd(z2, _mm_mul_pd(a1_2, h_3_1));
	__m128d y2 = _mm_add_pd(a2_2, _mm_mul_pd(a1_2, h_2_1));
	__m128d x2 = a1_2;

	__m128d q1;
	__m128d q2;

	__m128d h1;
	__m128d h2;
	__m128d h3;
	__m128d h4;

	for(i = 4; i < nb; i++)
	{
		h1 = _mm_loaddup_pd(&hh[i-3]);
		h2 = _mm_loaddup_pd(&hh[ldh+i-2]);
		h3 = _mm_loaddup_pd(&hh[(ldh*2)+i-1]);
		h4 = _mm_loaddup_pd(&hh[(ldh*3)+i]);

		q1 = _mm_load_pd(&q[i*ldq]);

		x1 = _mm_add_pd(x1, _mm_mul_pd(q1,h1));
		y1 = _mm_add_pd(y1, _mm_mul_pd(q1,h2));
		z1 = _mm_add_pd(z1, _mm_mul_pd(q1,h3));
		w1 = _mm_add_pd(w1, _mm_mul_pd(q1,h4));

		q2 = _mm_load_pd(&q[(i*ldq)+2]);

		x2 = _mm_add_pd(x2, _mm_mul_pd(q2,h1));
		y2 = _mm_add_pd(y2, _mm_mul_pd(q2,h2));
		z2 = _mm_add_pd(z2, _mm_mul_pd(q2,h3));
		w2 = _mm_add_pd(w2, _mm_mul_pd(q2,h4));
	}

	h1 = _mm_loaddup_pd(&hh[nb-3]);
	h2 = _mm_loaddup_pd(&hh[ldh+nb-2]);
	h3 = _mm_loaddup_pd(&hh[(ldh*2)+nb-1]);

	q1 = _mm_load_pd(&q[nb*ldq]);
	q2 = _mm_load_pd(&q[(nb*ldq)+2]);

	x1 = _mm_add_pd(x1, _mm_mul_pd(q1,h1));
	x2 = _mm_add_pd(x2, _mm_mul_pd(q2,h1));
	y1 = _mm_add_pd(y1, _mm_mul_pd(q1,h2));
	y2 = _mm_add_pd(y2, _mm_mul_pd(q2,h2));
	z1 = _mm_add_pd(z1, _mm_mul_pd(q1,h3));
	z2 = _mm_add_pd(z2, _mm_mul_pd(q2,h3));

	h1 = _mm_loaddup_pd(&hh[nb-2]);
	h2 = _mm_loaddup_pd(&hh[(ldh*1)+nb-1]);

	q1 = _mm_load_pd(&q[(nb+1)*ldq]);
	q2 = _mm_load_pd(&q[((nb+1)*ldq)+2]);

	x1 = _mm_add_pd(x1, _mm_mul_pd(q1,h1));
	x2 = _mm_add_pd(x2, _mm_mul_pd(q2,h1));
	y1 = _mm_add_pd(y1, _mm_mul_pd(q1,h2));
	y2 = _mm_add_pd(y2, _mm_mul_pd(q2,h2));

	h1 = _mm_loaddup_pd(&hh[nb-1]);

	q1 = _mm_load_pd(&q[(nb+2)*ldq]);
	q2 = _mm_load_pd(&q[((nb+2)*ldq)+2]);

	x1 = _mm_add_pd(x1, _mm_mul_pd(q1,h1));
	x2 = _mm_add_pd(x2, _mm_mul_pd(q2,h1));

	/////////////////////////////////////////////////////
	// Rank-1 update of Q [4 x nb+3]
	/////////////////////////////////////////////////////

	__m128d tau1 = _mm_loaddup_pd(&hh[0]);
	__m128d tau2 = _mm_loaddup_pd(&hh[ldh]);
	__m128d tau3 = _mm_loaddup_pd(&hh[ldh*2]);
	__m128d tau4 = _mm_loaddup_pd(&hh[ldh*3]);

	__m128d vs_1_2 = _mm_loaddup_pd(&s_1_2);
	__m128d vs_1_3 = _mm_loaddup_pd(&s_1_3);
	__m128d vs_2_3 = _mm_loaddup_pd(&s_2_3);
	__m128d vs_1_4 = _mm_loaddup_pd(&s_1_4);
	__m128d vs_2_4 = _mm_loaddup_pd(&s_2_4);
	__m128d vs_3_4 = _mm_loaddup_pd(&s_3_4);

	h1 = tau1;
	x1 = _mm_mul_pd(x1, h1);
	x2 = _mm_mul_pd(x2, h1);

	h1 = tau2;
	h2 = _mm_mul_pd(h1, vs_1_2);

	y1 = _mm_sub_pd(_mm_mul_pd(y1,h1), _mm_mul_pd(x1,h2));
	y2 = _mm_sub_pd(_mm_mul_pd(y2,h1), _mm_mul_pd(x2,h2));

	h1 = tau3;
	h2 = _mm_mul_pd(h1, vs_1_3);
	h3 = _mm_mul_pd(h1, vs_2_3);

	z1 = _mm_sub_pd(_mm_mul_pd(z1,h1), _mm_add_pd(_mm_mul_pd(y1,h3), _mm_mul_pd(x1,h2)));
	z2 = _mm_sub_pd(_mm_mul_pd(z2,h1), _mm_add_pd(_mm_mul_pd(y2,h3), _mm_mul_pd(x2,h2)));

	h1 = tau4;
	h2 = _mm_mul_pd(h1, vs_1_4);
	h3 = _mm_mul_pd(h1, vs_2_4);
	h4 = _mm_mul_pd(h1, vs_3_4);

	w1 = _mm_sub_pd(_mm_mul_pd(w1,h1), _mm_add_pd(_mm_mul_pd(z1,h4), _mm_add_pd(_mm_mul_pd(y1,h3), _mm_mul_pd(x1,h2))));
	w2 = _mm_sub_pd(_mm_mul_pd(w2,h1), _mm_add_pd(_mm_mul_pd(z2,h4), _mm_add_pd(_mm_mul_pd(y2,h3), _mm_mul_pd(x2,h2))));

	q1 = _mm_load_pd(&q[0]);
	q2 = _mm_load_pd(&q[2]);
	q1 = _mm_sub_pd(q1, w1);
	q2 = _mm_sub_pd(q2, w2);
	_mm_store_pd(&q[0],q1);
	_mm_store_pd(&q[2],q2);

	h4 = _mm_loaddup_pd(&hh[(ldh*3)+1]);
	q1 = _mm_load_pd(&q[ldq]);
	q2 = _mm_load_pd(&q[ldq+2]);

	q1 = _mm_sub_pd(q1, _mm_add_pd(z1, _mm_mul_pd(w1, h4)));
	q2 = _mm_sub_pd(q2, _mm_add_pd(z2, _mm_mul_pd(w2, h4)));

	_mm_store_pd(&q[ldq],q1);
	_mm_store_pd(&q[ldq+2],q2);

	h3 = _mm_loaddup_pd(&hh[(ldh*2)+1]);
	h4 = _mm_loaddup_pd(&hh[(ldh*3)+2]);
	q1 = _mm_load_pd(&q[ldq*2]);
	q2 = _mm_load_pd(&q[(ldq*2)+2]);

	q1 = _mm_sub_pd(q1, _mm_add_pd(y1, _mm_add_pd(_mm_mul_pd(z1, h3), _mm_mul_pd(w1, h4))));
	q2 = _mm_sub_pd(q2, _mm_add_pd(y2, _mm_add_pd(_mm_mul_pd(z2, h3), _mm_mul_pd(w2, h4))));
	_mm_store_pd(&q[ldq*2],q1);
	_mm_store_pd(&q[(ldq*2)+2],q2);

	h2 = _mm_loaddup_pd(&hh[ldh+1]);
	h3 = _mm_loaddup_pd(&hh[(ldh*2)+2]);
	h4 = _mm_loaddup_pd(&hh[(ldh*3)+3]);
	q1 = _mm_load_pd(&q[ldq*3]);
	q2 = _mm_load_pd(&q[(ldq*3)+2]);

	q1 = _mm_sub_pd(q1, _mm_add_pd(x1, _mm_add_pd(_mm_mul_pd(y1, h2), _mm_add_pd(_mm_mul_pd(z1, h3), _mm_mul_pd(w1, h4)))));
	q2 = _mm_sub_pd(q2, _mm_add_pd(x2, _mm_add_pd(_mm_mul_pd(y2, h2), _mm_add_pd(_mm_mul_pd(z2, h3), _mm_mul_pd(w2, h4)))));

	_mm_store_pd(&q[ldq*3], q1);
	_mm_store_pd(&q[(ldq*3)+2], q2);

	for (i = 4; i < nb; i++)
	{
		h1 = _mm_loaddup_pd(&hh[i-3]);
		h2 = _mm_loaddup_pd(&hh[ldh+i-2]);
		h3 = _mm_loaddup_pd(&hh[(ldh*2)+i-1]);
		h4 = _mm_loaddup_pd(&hh[(ldh*3)+i]);

		q1 = _mm_load_pd(&q[i*ldq]);

		q1 = _mm_sub_pd(q1, _mm_add_pd(_mm_add_pd(_mm_mul_pd(w1, h4), _mm_mul_pd(z1, h3)), _mm_add_pd(_mm_mul_pd(x1,h1), _mm_mul_pd(y1, h2))));

		_mm_store_pd(&q[i*ldq],q1);

		q2 = _mm_load_pd(&q[(i*ldq)+2]);

		q2 = _mm_sub_pd(q2, _mm_add_pd(_mm_add_pd(_mm_mul_pd(w2, h4), _mm_mul_pd(z2, h3)), _mm_add_pd(_mm_mul_pd(x2,h1), _mm_mul_pd(y2, h2))));

		_mm_store_pd(&q[(i*ldq)+2],q2);
	}

	h1 = _mm_loaddup_pd(&hh[nb-3]);
	h2 = _mm_loaddup_pd(&hh[ldh+nb-2]);
	h3 = _mm_loaddup_pd(&hh[(ldh*2)+nb-1]);
	q1 = _mm_load_pd(&q[nb*ldq]);
	q2 = _mm_load_pd(&q[(nb*ldq)+2]);

	q1 = _mm_sub_pd(q1, _mm_add_pd(_mm_add_pd(_mm_mul_pd(z1, h3), _mm_mul_pd(y1, h2)) , _mm_mul_pd(x1, h1)));
	q2 = _mm_sub_pd(q2, _mm_add_pd(_mm_add_pd(_mm_mul_pd(z2, h3), _mm_mul_pd(y2, h2)) , _mm_mul_pd(x2, h1)));

	_mm_store_pd(&q[nb*ldq],q1);
	_mm_store_pd(&q[(nb*ldq)+2],q2);

	h1 = _mm_loaddup_pd(&hh[nb-2]);
	h2 = _mm_loaddup_pd(&hh[ldh+nb-1]);
	q1 = _mm_load_pd(&q[(nb+1)*ldq]);
	q2 = _mm_load_pd(&q[((nb+1)*ldq)+2]);

	q1 = _mm_sub_pd(q1, _mm_add_pd( _mm_mul_pd(y1, h2) , _mm_mul_pd(x1, h1)));
	q2 = _mm_sub_pd(q2, _mm_add_pd( _mm_mul_pd(y2, h2) , _mm_mul_pd(x2, h1)));

	_mm_store_pd(&q[(nb+1)*ldq],q1);
	_mm_store_pd(&q[((nb+1)*ldq)+2],q2);

	h1 = _mm_loaddup_pd(&hh[nb-1]);
	q1 = _mm_load_pd(&q[(nb+2)*ldq]);
	q2 = _mm_load_pd(&q[((nb+2)*ldq)+2]);

	q1 = _mm_sub_pd(q1, _mm_mul_pd(x1, h1));
	q2 = _mm_sub_pd(q2, _mm_mul_pd(x2, h1));

	_mm_store_pd(&q[(nb+2)*ldq],q1);
	_mm_store_pd(&q[((nb+2)*ldq)+2],q2);
}

/**
 * Unrolled kernel that computes
 * 2 rows of Q simultaneously, a
 * matrix vector product with two householder
 * vectors + a rank 1 update is performed
 */
__forceinline void hh_trafo_kernel_2_SSE_4hv(double* q, double* hh, int nb, int ldq, int ldh, double s_1_2, double s_1_3, double s_2_3, double s_1_4, double s_2_4, double s_3_4)
{
	/////////////////////////////////////////////////////
	// Matrix Vector Multiplication, Q [2 x nb+3] * hh
	// hh contains four householder vectors
	/////////////////////////////////////////////////////
	int i;

	__m128d a1_1 = _mm_load_pd(&q[ldq*3]);
	__m128d a2_1 = _mm_load_pd(&q[ldq*2]);
	__m128d a3_1 = _mm_load_pd(&q[ldq]);
	__m128d a4_1 = _mm_load_pd(&q[0]);

	__m128d h_2_1 = _mm_loaddup_pd(&hh[ldh+1]);
	__m128d h_3_2 = _mm_loaddup_pd(&hh[(ldh*2)+1]);
	__m128d h_3_1 = _mm_loaddup_pd(&hh[(ldh*2)+2]);
	__m128d h_4_3 = _mm_loaddup_pd(&hh[(ldh*3)+1]);
	__m128d h_4_2 = _mm_loaddup_pd(&hh[(ldh*3)+2]);
	__m128d h_4_1 = _mm_loaddup_pd(&hh[(ldh*3)+3]);

	__m128d w1 = _mm_add_pd(a4_1, _mm_mul_pd(a3_1, h_4_3));
	w1 = _mm_add_pd(w1, _mm_mul_pd(a2_1, h_4_2));
	w1 = _mm_add_pd(w1, _mm_mul_pd(a1_1, h_4_1));
	__m128d z1 = _mm_add_pd(a3_1, _mm_mul_pd(a2_1, h_3_2));
	z1 = _mm_add_pd(z1, _mm_mul_pd(a1_1, h_3_1));
	__m128d y1 = _mm_add_pd(a2_1, _mm_mul_pd(a1_1, h_2_1));
	__m128d x1 = a1_1;

	__m128d q1;

	__m128d h1;
	__m128d h2;
	__m128d h3;
	__m128d h4;

	for(i = 4; i < nb; i++)
	{
		h1 = _mm_loaddup_pd(&hh[i-3]);
		h2 = _mm_loaddup_pd(&hh[ldh+i-2]);
		h3 = _mm_loaddup_pd(&hh[(ldh*2)+i-1]);
		h4 = _mm_loaddup_pd(&hh[(ldh*3)+i]);

		q1 = _mm_load_pd(&q[i*ldq]);

		x1 = _mm_add_pd(x1, _mm_mul_pd(q1,h1));
		y1 = _mm_add_pd(y1, _mm_mul_pd(q1,h2));
		z1 = _mm_add_pd(z1, _mm_mul_pd(q1,h3));
		w1 = _mm_add_pd(w1, _mm_mul_pd(q1,h4));
	}

	h1 = _mm_loaddup_pd(&hh[nb-3]);
	h2 = _mm_loaddup_pd(&hh[ldh+nb-2]);
	h3 = _mm_loaddup_pd(&hh[(ldh*2)+nb-1]);
	q1 = _mm_load_pd(&q[nb*ldq]);

	x1 = _mm_add_pd(x1, _mm_mul_pd(q1,h1));
	y1 = _mm_add_pd(y1, _mm_mul_pd(q1,h2));
	z1 = _mm_add_pd(z1, _mm_mul_pd(q1,h3));

	h1 = _mm_loaddup_pd(&hh[nb-2]);
	h2 = _mm_loaddup_pd(&hh[(ldh*1)+nb-1]);
	q1 = _mm_load_pd(&q[(nb+1)*ldq]);

	x1 = _mm_add_pd(x1, _mm_mul_pd(q1,h1));
	y1 = _mm_add_pd(y1, _mm_mul_pd(q1,h2));

	h1 = _mm_loaddup_pd(&hh[nb-1]);
	q1 = _mm_load_pd(&q[(nb+2)*ldq]);

	x1 = _mm_add_pd(x1, _mm_mul_pd(q1,h1));
	/////////////////////////////////////////////////////
	// Rank-1 update of Q [2 x nb+3]
	/////////////////////////////////////////////////////

	__m128d tau1 = _mm_loaddup_pd(&hh[0]);
	__m128d tau2 = _mm_loaddup_pd(&hh[ldh]);
	__m128d tau3 = _mm_loaddup_pd(&hh[ldh*2]);
	__m128d tau4 = _mm_loaddup_pd(&hh[ldh*3]);

	__m128d vs_1_2 = _mm_loaddup_pd(&s_1_2);
	__m128d vs_1_3 = _mm_loaddup_pd(&s_1_3);
	__m128d vs_2_3 = _mm_loaddup_pd(&s_2_3);
	__m128d vs_1_4 = _mm_loaddup_pd(&s_1_4);
	__m128d vs_2_4 = _mm_loaddup_pd(&s_2_4);
	__m128d vs_3_4 = _mm_loaddup_pd(&s_3_4);

	h1 = tau1;
	x1 = _mm_mul_pd(x1, h1);

	h1 = tau2;
	h2 = _mm_mul_pd(h1, vs_1_2);

	y1 = _mm_sub_pd(_mm_mul_pd(y1,h1), _mm_mul_pd(x1,h2));

	h1 = tau3;
	h2 = _mm_mul_pd(h1, vs_1_3);
	h3 = _mm_mul_pd(h1, vs_2_3);

	z1 = _mm_sub_pd(_mm_mul_pd(z1,h1), _mm_add_pd(_mm_mul_pd(y1,h3), _mm_mul_pd(x1,h2)));

	h1 = tau4;
	h2 = _mm_mul_pd(h1, vs_1_4);
	h3 = _mm_mul_pd(h1, vs_2_4);
	h4 = _mm_mul_pd(h1, vs_3_4);

	w1 = _mm_sub_pd(_mm_mul_pd(w1,h1), _mm_add_pd(_mm_mul_pd(z1,h4), _mm_add_pd(_mm_mul_pd(y1,h3), _mm_mul_pd(x1,h2))));

	q1 = _mm_load_pd(&q[0]);
	q1 = _mm_sub_pd(q1, w1);
	_mm_store_pd(&q[0],q1);

	h4 = _mm_loaddup_pd(&hh[(ldh*3)+1]);
	q1 = _mm_load_pd(&q[ldq]);

	q1 = _mm_sub_pd(q1, _mm_add_pd(z1, _mm_mul_pd(w1, h4)));

	_mm_store_pd(&q[ldq],q1);

	h3 = _mm_loaddup_pd(&hh[(ldh*2)+1]);
	h4 = _mm_loaddup_pd(&hh[(ldh*3)+2]);
	q1 = _mm_load_pd(&q[ldq*2]);

	q1 = _mm_sub_pd(q1, _mm_add_pd(y1, _mm_add_pd(_mm_mul_pd(z1, h3), _mm_mul_pd(w1, h4))));

	_mm_store_pd(&q[ldq*2],q1);

	h2 = _mm_loaddup_pd(&hh[ldh+1]);
	h3 = _mm_loaddup_pd(&hh[(ldh*2)+2]);
	h4 = _mm_loaddup_pd(&hh[(ldh*3)+3]);
	q1 = _mm_load_pd(&q[ldq*3]);

	q1 = _mm_sub_pd(q1, _mm_add_pd(x1, _mm_add_pd(_mm_mul_pd(y1, h2), _mm_add_pd(_mm_mul_pd(z1, h3), _mm_mul_pd(w1, h4)))));

	_mm_store_pd(&q[ldq*3], q1);

	for (i = 4; i < nb; i++)
	{
		h1 = _mm_loaddup_pd(&hh[i-3]);
		h2 = _mm_loaddup_pd(&hh[ldh+i-2]);
		h3 = _mm_loaddup_pd(&hh[(ldh*2)+i-1]);
		h4 = _mm_loaddup_pd(&hh[(ldh*3)+i]);

		q1 = _mm_load_pd(&q[i*ldq]);

		q1 = _mm_sub_pd(q1, _mm_add_pd(_mm_add_pd(_mm_mul_pd(w1, h4), _mm_mul_pd(z1, h3)), _mm_add_pd(_mm_mul_pd(x1,h1), _mm_mul_pd(y1, h2))));

		_mm_store_pd(&q[i*ldq],q1);
	}

	h1 = _mm_loaddup_pd(&hh[nb-3]);
	h2 = _mm_loaddup_pd(&hh[ldh+nb-2]);
	h3 = _mm_loaddup_pd(&hh[(ldh*2)+nb-1]);
	q1 = _mm_load_pd(&q[nb*ldq]);

	q1 = _mm_sub_pd(q1, _mm_add_pd(_mm_add_pd(_mm_mul_pd(z1, h3), _mm_mul_pd(y1, h2)) , _mm_mul_pd(x1, h1)));

	_mm_store_pd(&q[nb*ldq],q1);

	h1 = _mm_loaddup_pd(&hh[nb-2]);
	h2 = _mm_loaddup_pd(&hh[ldh+nb-1]);
	q1 = _mm_load_pd(&q[(nb+1)*ldq]);

	q1 = _mm_sub_pd(q1, _mm_add_pd( _mm_mul_pd(y1, h2) , _mm_mul_pd(x1, h1)));

	_mm_store_pd(&q[(nb+1)*ldq],q1);

	h1 = _mm_loaddup_pd(&hh[nb-1]);
	q1 = _mm_load_pd(&q[(nb+2)*ldq]);

	q1 = _mm_sub_pd(q1, _mm_mul_pd(x1, h1));

	_mm_store_pd(&q[(nb+2)*ldq],q1);
}