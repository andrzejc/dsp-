/*!
 * @internal Parks-McClellan algorithm for FIR filter design (C version)
 * @copyright Copyright &copy; 1995,1998  Jake Janovetz (janovetz@uiuc.edu)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.

 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#ifndef REMEZ_H_INCLUDED
#define REMEZ_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif


//! @internal Type of filter designed by remez() algorithm.
enum remez_filter_type_e
{
	REMEZ_FILTER_BANDPASS, 			//!< @internal Band-pass (LP, HP, BP, BS) filter.
	REMEZ_FILTER_DIFFERENTIATOR,	//!< @internal Differentiator.
	REMEZ_FILTER_HILBERT			//!< @internal Hilbert transformer.
};

typedef enum remez_filter_type_e remez_filter_type;

//! @internal Error code indicating that the algorithm was unable to allocate internal arrays.
#define REMEZ_ERRNOMEM 		(-1)
//! @internal Warning code indicating that the design was interrupted due to algorithm reaching max iteration count, the results may be invalid.
#define REMEZ_WARNMAXITER	(1)
//! @internal No-error status code.
#define REMEZ_NOERR			(0)
#define REMEZ_FAILED(stat) 	((stat) < 0)

/*!
 * @internal FIR filter design using Parks-McClellan algorithm (Remez exchange).
 *
 * Calculates the optimal (in the Chebyshev/minimax sense)
 * FIR filter impulse response given a set of band edges,
 * the desired reponse on those bands, and the weight given to
 * the error in those bands.
 * @param[in] numtaps Number of filter coefficients.
 * @param[in] numband Number of bands in filter specification.
 * @param[in] bands User-specified band edges [2 * numband].
 * @param[in] des User-specified responses at band edges [2*numband].
 * @param[in] weight User-specified error weights [numband].
 * @param[in] type Type of filter.
 * @param[in] grid_density Initial grid density.
 * @param[in] max_iterations Maximum iteration count.
 * @param[out] h Impulse response of final filter [numtaps].
 * @return status code (REMEZ_NOERR, one of REMEZ_ERR* constants or combination of REMEZ_WARN* masks).
 * @retval REMEZ_NOERR if completed without errors nor warnings.
 * @retval REMEZ_WARNMAXITER if maximum iteration count was reached.
 * @retval REMEZ_ERRNOMEM if unable to allocate memory for internal arrays.
 */
int remez(double h[], int numtaps,
           int numband, double bands[], const double des[], const double weight[],
           remez_filter_type type, int grid_density, int max_iterations);

#ifdef __cplusplus
}
#endif

#endif /* REMEZ_H_INCLUDED */

