/*
 * TrilinosLinearAlgebra.h
 *
 *  Created on: Jun 13, 2012
 *      Author: gratienj
 */
#ifndef ALIEN_KERNEL_TRILINOS_ALGEBRA_TRILINOSLINEARALGEBRA_H
#define ALIEN_KERNEL_TRILINOS_ALGEBRA_TRILINOSLINEARALGEBRA_H

#include <alien/utils/Precomp.h>

#include <ALIEN/Kernels/Trilinos/TrilinosBackEnd.h>
#include <alien/core/backend/LinearAlgebra.h>

/*---------------------------------------------------------------------------*/

namespace Alien {

typedef LinearAlgebra<BackEnd::tag::tpetraserial> TrilinosLinearAlgebra;
#ifdef KOKKOS_USE_OPENMP
typedef LinearAlgebra<BackEnd::tag::tpetraomp> TpetraOmpLinearAlgebra;
#endif
#ifdef KOKKOS_USE_THREADS
typedef LinearAlgebra<BackEnd::tag::tpetrapth> TpetraPthLinearAlgebra;
#endif
#ifdef KOKKOS_USE_CUDA
typedef LinearAlgebra<BackEnd::tag::tpetracuda> TpetraCudaLinearAlgebra;
#endif

} // namespace Alien

/*---------------------------------------------------------------------------*/

#endif /* ALIEN_KERNEL_TRILINOS_ALGEBRA_TRILINOSLINEARALGEBRA_H */
