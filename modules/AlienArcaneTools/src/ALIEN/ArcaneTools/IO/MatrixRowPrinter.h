// -*- C++ -*-
#ifndef ALIEN_UTILS_MATRIXROWPRINTER_H
#define ALIEN_UTILS_MATRIXROWPRINTER_H
/* Author : havep at Tue Feb 12 09:55:45 2013
 * Generated by createNew
 */

#include <map>
#include <alien/utils/Precomp.h>
#include <ALIEN/Kernels/PETSc/DataStructure/PETScMatrix.h>
#include <ALIEN/Kernels/PETSc/DataStructure/PETScVector.h>
#include <ALIEN/Kernels/PETSc/DataStructure/PETScMatrix.h>
#include <alien/core/impl/MultiMatrixImpl.h>
#include <alien/core/impl/MultiVectorImpl.h>
#include <alien/data/IMatrix.h>
#include "ALIEN/ArcaneTools/IIndexManager.h"

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

namespace Alien {

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

namespace ArcaneTools {

  /*---------------------------------------------------------------------------*/
  /*---------------------------------------------------------------------------*/

  class MatrixRowPrinter
  {
   public:
    enum ReverseMappingType
    {
      eNone,
      eShortLid,
    eShortUid,
    eFull
  };

 public:
  //! Object to get row data
  /*! May be optimized if necessary */
  typedef std::map<Arccore::Integer, Arccore::Real> RowData;

 public:
  /** Constructeur de la classe */
  /*! @a reverse_mapping_type select what display is used if the matrix has an
   * index-manager
   * - eNone : no reverse mapping
   * - eShortLid : reverse mapping with local id
   * - eShortUid : reverse mapping with unique id
   * - eFull : reverse mapping with local id, unique id and owner
   */
  MatrixRowPrinter(const IMatrix& matrix,
                   const IIndexManager& index_mng,
                   const ReverseMappingType reverse_mapping_type);
  
  /** Destructeur de la classe */
  virtual ~MatrixRowPrinter();

 public:
  //! Print a matrix row (using global numbering)
  Arccore::String operator()(const Arccore::Integer global_line_index) const
  {
    return print(global_line_index);
  }

  //! Print a matrix row (using global numbering)
  Arccore::String print(const Arccore::Integer global_line_index) const;

  //! Dump a vector
  RowData extract(const Arccore::Integer global_line_index) const;

 private:
  //! Check error of PETSc commands
  void checkError(const Arccore::String& msg, int ierr) const;

 private:
  class Internal;
  Internal* m_internal;
  const PETScMatrix& m_petsc_matrix;
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

} // namespace Alien

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#endif /* ALIEN_UTILS_MATRIXROWPRINTER_H */
