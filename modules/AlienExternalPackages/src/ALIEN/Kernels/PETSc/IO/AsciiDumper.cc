#include "AsciiDumper.h"
/* Author : havep at Fri Sep 28 17:13:28 2012
 * Generated by createNew
 */

#include <ALIEN/Kernels/PETSc/DataStructure/PETScMatrix.h>
#include <ALIEN/Kernels/PETSc/DataStructure/PETScVector.h>
#include <ALIEN/Kernels/PETSc/DataStructure/PETScInternal.h>
#include <ALIEN/Kernels/PETSc/DataStructure/PETScInit.h>
#include <ALIEN/Kernels/PETSc/PETScBackEnd.h>

#include <alien/core/impl/MultiMatrixImpl.h>
#include <alien/core/impl/MultiVectorImpl.h>

#include <alien/data/IMatrix.h>
#include <alien/data/IVector.h>
#include <alien/kernels/simple_csr/data_structure/SimpleCSRMatrix.h>
#include <alien/kernels/simple_csr/data_structure/SimpleCSRVector.h>
#include <petscmat.h>
#include <petscsys.h>
#include <petscvec.h>
#include <petscversion.h>

#include <arccore/base/NotImplementedException.h>
#include <arccore/collections/Array2.h>
#include <arccore/message_passing_mpi/MpiMessagePassingMng.h>
#include <arccore/trace/ITraceMng.h>

#include <fstream>

#ifdef ALIEN_USE_ARCANE
#include <arcane/ISubDomain.h>
#include <arcane/utils/NotImplementedException.h>

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

extern "C" Arcane::ISubDomain* _arcaneGetDefaultSubDomain();
#endif

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

namespace Alien {

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

class AsciiDumper::Internal
{
 public:
  template <typename Dist> static bool isSequential(const Dist& dist)
  {
    return not dist.isParallel();
  }

  template <typename Dist>
  static void initializeViewer(
      PetscViewer& viewer, const Dist& dist, Arccore::String filename, const Style style)
  {
    if(dist.parallelMng() == nullptr)
      throw Arccore::FatalErrorException("dist parallel mng ptr is nullptr");
    std::cout << dist.isParallel() << std::endl;
    std::cout << dist.parallelMng() << std::endl;
    auto* parallel_mng =
        dynamic_cast<Arccore::MessagePassing::Mpi::MpiMessagePassingMng*>(
            dist.parallelMng());
    if(parallel_mng == nullptr)
      throw Arccore::FatalErrorException("parallel mng ptr is nullptr : not a MpiMessagePassingMng impl");
    const MPI_Comm* arcane_mpi_comm = parallel_mng->getMPIComm();
    if (arcane_mpi_comm == 0) {
      PetscViewerASCIIOpen(PETSC_COMM_SELF, filename.localstr(), &viewer);
    } else {
      const MPI_Comm* comm = arcane_mpi_comm;
      PetscViewerASCIIOpen(*comm, filename.localstr(), &viewer);
    }
    _pushFormat(viewer, style);
  }

  template <typename Dist>
  static void initializeViewer(PetscViewer& viewer, const Dist& dist, const Style style)
  {
    auto* parallel_mng =
        dynamic_cast<Arccore::MessagePassing::Mpi::MpiMessagePassingMng*>(
            dist.parallelMng());
    const MPI_Comm* arcane_mpi_comm = parallel_mng->getMPIComm();
    if (arcane_mpi_comm == 0) {
      PetscViewerASCIIGetStdout(PETSC_COMM_SELF, &viewer);
    } else {
      const MPI_Comm* comm = arcane_mpi_comm;
      PetscViewerASCIIGetStdout(*comm, &viewer);
    }
    _pushFormat(viewer, style);
  }

  static void destroyViewer(PetscViewer& viewer)
  {
#ifndef PETSC_VIEWERDESTROY_NEW
    PetscViewerDestroy(viewer);
#else /* PETSC_VIEWERDESTROY_NEW */
    PetscViewerDestroy(&viewer);
#endif /* PETSC_VIEWERDESTROY_NEW */
  }

private:
  static void _pushFormat(PetscViewer& viewer, const Style style)
  {
    switch (style) {
    case eDefaultStyle:
      PetscViewerPushFormat(viewer, PETSC_VIEWER_DEFAULT);
      break;
    case eMatlabStyle:
      PetscViewerPushFormat(viewer, PETSC_VIEWER_ASCII_MATLAB);
      break;
    case eInfoStyle:
      PetscViewerPushFormat(viewer, PETSC_VIEWER_ASCII_INFO_DETAIL);
      break;
    default:
      throw Arccore::NotImplementedException(A_FUNCINFO, "Unknown dump style");
    }
#ifdef PETSC_OPTIONSSETVALUE_OLD
    PetscOptionsSetValue("-mat_ascii_output_large", ""); // force output for large data
#else
    PetscOptionsSetValue(NULL, "-mat_ascii_output_large",""); // force output for large data
#endif
  }
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

AsciiDumper::AsciiDumper(const Style style, Arccore::ITraceMng* trace)
: m_style(style)
, m_trace(trace)
{
  PETScInternal::initPETSc();
}

/*---------------------------------------------------------------------------*/

void
AsciiDumper::dump(const Arccore::String filename, const IMatrix& a)
{
  const auto& dist = a.impl()->distribution();

  if(m_style == eSequentialFixedBlockSizeStype) {
    if(Internal::isSequential(dist)) {
      std::ofstream str(filename.localstr());
      _blockDump(a, str);
      return;
    } else {
#ifdef ALIEN_USE_ARCANE
      if (!m_trace)
        m_trace = _arcaneGetDefaultSubDomain()->traceMng();
#endif
      if (m_trace)
        m_trace->warning() << "Parallel run : dump with matlab mode";
      m_style = eMatlabStyle;
    }
  }
  const PETScMatrix& petsc_matrix = a.impl()->get<BackEnd::tag::petsc>();
  PetscViewer viewer = NULL;
  Internal::initializeViewer(viewer, dist, filename, m_style);
  MatView(petsc_matrix.internal()->m_internal, viewer);
  Internal::destroyViewer(viewer);
}

/*---------------------------------------------------------------------------*/

void
AsciiDumper::dump(const Arccore::String filename, const IVector& v)
{
  const auto& dist = v.impl()->distribution();

  if(m_style == eSequentialFixedBlockSizeStype) {
    if(Internal::isSequential(dist)) {
      std::ofstream str(filename.localstr());
      _blockDump(v, str);
      return;
    } else {
#ifdef ALIEN_USE_ARCANE
      if (!m_trace)
        m_trace = _arcaneGetDefaultSubDomain()->traceMng();
#endif
      if (m_trace)
        m_trace->warning() << "Parallel run : dump with matlab mode";
      m_style = eMatlabStyle;
    }
  }
  const PETScVector& petsc_vector = v.impl()->get<BackEnd::tag::petsc>();
  PetscViewer viewer = NULL;
  Internal::initializeViewer(viewer, dist, filename, m_style);
  VecView(petsc_vector.internal()->m_internal, viewer);
  Internal::destroyViewer(viewer);
}

/*---------------------------------------------------------------------------*/

void
AsciiDumper::
dump(const IMatrix & a)
{
  const auto& dist = a.impl()->distribution();

  if(m_style == eSequentialFixedBlockSizeStype) {
#ifdef ALIEN_USE_ARCANE
    if (!m_trace)
      m_trace = _arcaneGetDefaultSubDomain()->traceMng();
#endif
    if(Internal::isSequential(dist)) {
      std::ostringstream str;
      _blockDump(a, str);
      if (m_trace)
        m_trace->info() << str.str();
      return;
    } else {
      if (m_trace)
        m_trace->warning() << "Parallel run : dump with matlab mode";
      m_style = eMatlabStyle;
    }
  }
  if(m_style == eSequentialVariableBlockSizeStype) {
#ifdef ALIEN_USE_ARCANE
    if (!m_trace)
      m_trace = _arcaneGetDefaultSubDomain()->traceMng();
#endif
    if(Internal::isSequential(dist)) {
      std::ostringstream str;
      _vblockDump(a, str);
      if (m_trace)
        m_trace->info() << str.str();
      return;
    } else {
      if (m_trace)
        m_trace->warning() << "Parallel run : dump with matlab mode";
      m_style = eMatlabStyle;
    }
  }
  const PETScMatrix& petsc_matrix = a.impl()->get<BackEnd::tag::petsc>();
  PetscViewer viewer = nullptr;
  Internal::initializeViewer(viewer, dist, m_style);
  MatView(petsc_matrix.internal()->m_internal, viewer);
  // NB: ne pas détruire le viewer!! C'est un viewer partagé
}

/*---------------------------------------------------------------------------*/

void
AsciiDumper::
dump(const IVector & v)
{
  const auto& dist = v.impl()->distribution();

  if(m_style == eSequentialFixedBlockSizeStype) {
#ifdef ALIEN_USE_ARCANE
    if (!m_trace)
      m_trace = _arcaneGetDefaultSubDomain()->traceMng();
#endif
    if(Internal::isSequential(dist)) {
      std::ostringstream str;
      _blockDump(v, str);
      if (m_trace)
        m_trace->info() << str.str();
      return;
    } else {
      if (m_trace)
        m_trace->warning() << "Parallel run : dump with matlab mode";
      m_style = eMatlabStyle;
    }
  }
  const PETScVector& petsc_vector = v.impl()->get<BackEnd::tag::petsc>();
  PetscViewer viewer = nullptr;
  Internal::initializeViewer(viewer, dist, m_style);
  VecView(petsc_vector.internal()->m_internal, viewer);
  // NB: ne pas détruire le viewer!! C'est un viewer partagé
}

/*---------------------------------------------------------------------------*/

void
AsciiDumper::
_blockDump(const IMatrix& a, std::ostream & str)
{
  const auto& dist = a.impl()->distribution();
  const auto* block = a.impl()->block();

  if (block == nullptr)
    throw Arccore::FatalErrorException(
        "Builder is adapted to 'fixed block size' kind matrix");

  const Arccore::Integer local_size = dist.localRowSize();
  const Arccore::Integer block_size = block->size();

  typedef SimpleCSRMatrix<Arccore::Real> RealMatrix;

  const RealMatrix& internal_matrix = a.impl()->get<BackEnd::tag::simplecsr>();

  typedef RealMatrix::CSRStructInfo CSR;

  const CSR& csr = internal_matrix.getCSRProfile();

  str << "BlockMatrix[rows=" << local_size << ",cols=" << local_size
      << ",block_size=" << block_size << ",nnz=" << csr.getNnz()
      << ",ordering=" << csr.getColOrdering() << "]\n";

  Arccore::ConstArrayView<Arccore::Integer> rows = csr.getRowOffset();
  {
    str << "rows_start[" << rows.size() << "] { ";
    for (Arccore::Integer i = 0; i < rows.size(); ++i) {
      str << rows[i] << " ";
    }
    str << "}\n";
  }
  Arccore::ConstArrayView<Arccore::Integer> cols = csr.getCols();
  {
    str << "cols[" << cols.size() << "] { ";
    for (Arccore::Integer i = 0; i < cols.size(); ++i) {
      str << cols[i] << " ";
    }
    str << "}\n";
  }
  Arccore::ConstArrayView<Arccore::Real> values = internal_matrix.internal().getValues();
  {
    const Arccore::Integer block_size2 = block_size * block_size;
    for (Arccore::Integer i = 0; i < local_size; ++i) {
      const Arccore::Integer nb_cols = rows[i + 1] - rows[i];
      Arccore::Integer row_start = rows[i] * block_size2;
      for (Arccore::Integer j = 0; j < nb_cols; ++j) {
        Arccore::ConstArray2View<Arccore::Real> block(
            &values[row_start], block_size, block_size);
        row_start += block_size2;
        str << "block[" << i << "," << cols[j] << "] {\n";
        for (Arccore::Integer bi = 0; bi < block_size; ++bi) {
          for (Arccore::Integer bj = 0; bj < block_size; ++bj) {
            str << block[bi][bj] << " ";
          }
          str << "\n";
        }
        str << "}\n";
      }
    }
  }
}

/*---------------------------------------------------------------------------*/

void
AsciiDumper::
_vblockDump(const IMatrix& a, std::ostream & str)
{
  const auto& dist = a.impl()->distribution();
  const auto* row_vblock = a.impl()->rowBlock();
  const auto* col_vblock = a.impl()->colBlock();

  if(row_vblock==nullptr)
    throw Arccore::FatalErrorException(
        "Builder is adapted to 'variable block size' kind matrix");

  const Arccore::Integer local_size = dist.localRowSize();

  typedef SimpleCSRMatrix<Arccore::Real> RealMatrix;

  const RealMatrix& internal_matrix = a.impl()->get<BackEnd::tag::simplecsr>();

  typedef RealMatrix::CSRStructInfo CSR;

  const CSR& csr = internal_matrix.getCSRProfile();

  str << "VBlockMatrix[rows=" << local_size << ",cols=" << local_size
      << ",maxBlockSize=" << row_vblock->maxBlockSize() << "x" << col_vblock->maxBlockSize() << ",nnz=" << csr.getNnz()
      << ",ordering=" << csr.getColOrdering() << "]\n";

  Arccore::ConstArrayView<Arccore::Integer> rows = csr.getRowOffset();
  {
    str << "rows_start[" << rows.size() << "] { ";
    for (Arccore::Integer i = 0; i < rows.size(); ++i) {
      str << rows[i] << " ";
    }
    str << "}\n";
  }
  Arccore::ConstArrayView<Arccore::Integer> cols = csr.getCols();
  {
    str << "cols[" << cols.size() << "] { ";
    for (Arccore::Integer i = 0; i < cols.size(); ++i) {
      str << cols[i] << " ";
    }
    str << "}\n";
  }

  str << "blocks{ ";
  for (Arccore::Integer i = 0; i < local_size; ++i)
    for (Arccore::Integer j = rows[i]; j < rows[i + 1]; ++j)
      str << row_vblock->size(i) << "x" << col_vblock->size(cols[j]) << " ";
  str << "}\n";

  Arccore::ConstArrayView<Arccore::Real> values = internal_matrix.internal().getValues();
  {
    Arccore::Integer blockRowOffset = 0;
    for (Arccore::Integer i = 0; i < local_size; ++i) {
      for (Arccore::Integer j = rows[i]; j < rows[i + 1]; ++j) {
        Arccore::ConstArray2View<Arccore::Real> block(
            &values[blockRowOffset], row_vblock->size(i), col_vblock->size(cols[j]));
        blockRowOffset += row_vblock->size(i) * col_vblock->size(cols[j]);
        str << "block[" << i << "," << cols[j] << "] {\n";
        for (Arccore::Integer bi = 0; bi < row_vblock->size(i); ++bi) {
          for (Arccore::Integer bj = 0; bj < col_vblock->size(cols[j]); ++bj) {
            str << block[bi][bj] << " ";
          }
          str << "\n";
        }
        str << "}\n";
      }
    }
  }
}

/*---------------------------------------------------------------------------*/

void
AsciiDumper::
_blockDump(const IVector& v, std::ostream & str)
{
  const auto& dist = v.impl()->distribution();
  const auto* block = v.impl()->block();

  if (block == nullptr)
    throw Arccore::FatalErrorException(
        "Builder is adapted to 'fixed block size' kind vector");

  const Arccore::Integer local_size = dist.localSize();
  const Arccore::Integer block_size = block->size();

  typedef SimpleCSRVector<Arccore::Real> RealVector;

  const RealVector& internal_vector = v.impl()->get<BackEnd::tag::simplecsr>();

  str << "BlockVector[size=" << local_size << ",block_size=" << block_size << "]\n";

  Arccore::ConstArrayView<Arccore::Real> values = internal_vector.values();

  for (Arccore::Integer i = 0; i < local_size; ++i) {
    str << "block[" << i << "] { ";
    for (Arccore::Integer b = 0; b < block_size; ++b)
      str << values[i * block_size + b] << " ";
    str << "}\n";
  }
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

} // namespace Alien

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
