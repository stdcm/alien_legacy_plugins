/* Author : havep at Tue Feb 12 09:55:45 2013
 * Generated by createNew
 */
#include "alien/arcane_tools/io/MatrixRowPrinter.h"

#include <alien/arcane_tools/IIndexManager.h>
#include <alien/core/impl/MultiMatrixImpl.h>
#include <alien/core/impl/MultiVectorImpl.h>
#include <alien/kernels/petsc/data_structure/PETScInternal.h>
#include <alien/kernels/petsc/data_structure/PETScVector.h>
#include <alien/kernels/petsc/PETScBackEnd.h>

#include <petscmat.h>
#include <petscsys.h>
#include <petscversion.h>

#include "alien/arcane_tools/IIndexManager.h"
#include <cmath>

#include <arccore/base/NotImplementedException.h>

#include <arcane/utils/OStringStream.h>

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

namespace Alien {

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

namespace ArcaneTools {

  /*---------------------------------------------------------------------------*/
  /*---------------------------------------------------------------------------*/

  class MatrixRowPrinter::Internal
  {
   public:
    Internal(const IIndexManager* manager,
        const MatrixRowPrinter::ReverseMappingType reverse_mapping_type);
    Arccore::String identify(const Arccore::Integer global_index);

   private:
    //! Table des Entry rangés par Entry index
    Arccore::UniqueArray<IIndexManager::Entry> m_entries;
    //! Identifiant local maximal
    Arccore::Integer m_max_local_id;
    //! Table global_index -> Entry Index * m_max_local_id + position in Entry
    VMap<Arccore::Integer, Arccore::Integer> m_reverse_data;
    //! Reverse mapping type
    const MatrixRowPrinter::ReverseMappingType m_reverse_mapping_type;
  };

  /*---------------------------------------------------------------------------*/

  MatrixRowPrinter::Internal::Internal(const IIndexManager* manager,
      const MatrixRowPrinter::ReverseMappingType reverse_mapping_type)
  : m_reverse_mapping_type(reverse_mapping_type)
  {
    ALIEN_ASSERT((manager), ("Unexpected null IIndexManager"));
    ALIEN_ASSERT((reverse_mapping_type != eNone),
        ("MatrixRowPrinter::Internal only for identifying non 'None' reverse mapping"));

    m_max_local_id = 0;
    Arccore::Integer local_id_count = 0;
    for (IIndexManager::EntryEnumerator ie = manager->enumerateEntry(); ie.hasNext();
         ++ie) {
      const IIndexManager::Entry& entry = *ie;
      const Arccore::ConstArrayView<Arccore::Integer> localIds = entry.getAllLocalIds();
      for (Arccore::Integer i = 0, is = localIds.size(); i < is; ++i)
        m_max_local_id = std::max(m_max_local_id, localIds[i]);
      local_id_count += localIds.size();
    }

    m_max_local_id += 1;
// FIXME: What the fuck ?
// On vérifie que l'on peut encoder les positions via un produit entry_id * m_max_local_id
#ifdef WIN32
    if ((std::log((double)m_max_local_id)
            + std::log((double)manager->enumerateEntry().count()) / std::log(2.))
            + 1
        >= 8 * sizeof(Integer))
      throw FatalErrorException(A_FUNCINFO, "Index overflow");
#else /* WIN32 */
    if (log2(m_max_local_id) + log2(manager->enumerateEntry().count()) + 1
        >= 8 * sizeof(Arccore::Integer))
      throw Arccore::FatalErrorException(A_FUNCINFO, "Index overflow");
#endif /* WIN32 */
    m_reverse_data = VMap<Arccore::Integer, Arccore::Integer>(local_id_count);
    for (IIndexManager::EntryEnumerator ie = manager->enumerateEntry(); ie.hasNext();
         ++ie) {
      const Arccore::Integer entry_id = m_entries.size();
      const Arccore::Integer global_entry_offset = m_max_local_id * entry_id;
      const IIndexManager::Entry& entry = *ie;
      m_entries.push_back(entry);
      Arccore::ConstArrayView<Arccore::Integer> indexes = entry.getAllIndexes();
      Arccore::ConstArrayView<Arccore::Integer> localIds = entry.getAllLocalIds();
      for (Arccore::Integer i = 0, is = localIds.size(); i < is; ++i) {
        ALIEN_ASSERT((m_reverse_data.find(indexes[i]) == m_reverse_data.end()),
            ("Conflicting global index"));
        m_reverse_data[indexes[i]] = global_entry_offset + localIds[i];
      }
    }
  }

  /*---------------------------------------------------------------------------*/

  Arccore::String MatrixRowPrinter::Internal::identify(
      const Arccore::Integer global_index)
  {
    auto finder =
        m_reverse_data.find(global_index);
    if (finder == m_reverse_data.end())
#ifdef ALIEN_USE_ARCANE
      throw Arccore::FatalErrorException(
          A_FUNCINFO, Arccore::String::format("Invalid index {0}", global_index));
#else
      throw Arccore::FatalErrorException(
          A_FUNCINFO, Arccore::String::format("Invalid index {0}", global_index));
#endif
    const Arccore::Integer entry_id = finder.value() / m_max_local_id;
    const Arccore::Integer local_id = finder.value() % m_max_local_id;
    IIndexManager::IAbstractFamily::Item item =
        m_entries[entry_id].getFamily().item(local_id);
    switch (m_reverse_mapping_type) {
    case eNone:
      return Arccore::String::format("{0}", global_index);
    case eShortLid:
      return Arccore::String::format(
          "{0}={1}@{2}", global_index, m_entries[entry_id].getName(), local_id);
    case eShortUid:
      return Arccore::String::format(
          "{0}={1}@{2}", global_index, m_entries[entry_id].getName(), item.uniqueId());
    case eFull:
      return Arccore::String::format("{0}={1}@lid={2};uid={3};own={4}", global_index,
          m_entries[entry_id].getName(), local_id, item.uniqueId(), item.owner());
    default:
      throw Arccore::NotImplementedException(
          A_FUNCINFO, Arccore::String::format("Unexpected reverse mapping type ({0})",
                          m_reverse_mapping_type));
    }
  }

  /*---------------------------------------------------------------------------*/
  /*---------------------------------------------------------------------------*/

  MatrixRowPrinter::MatrixRowPrinter(const IMatrix& matrix,
      const IIndexManager& index_mng, const ReverseMappingType reverse_mapping_type)
  : m_internal(NULL)
  , m_petsc_matrix(matrix.impl()->get<Alien::BackEnd::tag::petsc>())
  {
    const auto& dist = m_petsc_matrix.distribution();
    Arccore::Integer global_size, offset, local_size;
    index_mng.stats(global_size, offset, local_size);
    if (global_size != dist.globalRowSize() || local_size != dist.localRowSize()
        || offset != dist.rowOffset()) {
      throw Arccore::FatalErrorException(
          A_FUNCINFO, "Distribution doesn't match IIndexManager");
    }
    if (reverse_mapping_type != eNone)
      m_internal = new Internal(&index_mng, reverse_mapping_type);
  }

  /*---------------------------------------------------------------------------*/

  MatrixRowPrinter::~MatrixRowPrinter()
  {
    delete m_internal; // toujours valide même si vaut NULL
  }

  /*---------------------------------------------------------------------------*/

  Arccore::String MatrixRowPrinter::print(const Arccore::Integer global_line_index) const
  {
    PetscInt rstart, rend;
    checkError("Extract Matrix OwnershipRange",
        MatGetOwnershipRange(m_petsc_matrix.internal()->m_internal, &rstart, &rend));
#ifndef ALIEN_USE_ARCANE
    class OStringStream
    {
     public:
      std::stringstream& operator()() { return oss; }
      Arccore::String str() { return oss.str(); }
      std::stringstream oss;
    };

    OStringStream oss;
#else
    Arcane::OStringStream oss;
#endif

    if (global_line_index < rstart or global_line_index >= rend) {
      if (m_internal) {
        oss() << global_line_index << "=NonLocalRow -> undefined non local row";
      } else {
        oss() << global_line_index << " -> undefined non local row";
      }
      return oss.str();
    }

    const PetscInt* cols = PETSC_NULL;
    const PetscScalar* vals = PETSC_NULL;
    PetscInt ncols = 0;
    checkError("Extract row", MatGetRow(m_petsc_matrix.internal()->m_internal,
                                  global_line_index, &ncols, &cols, &vals));
    if (m_internal) {
      oss() << m_internal->identify(global_line_index) << " -> ";
      for (Arccore::Integer i = 0; i < ncols; ++i)
        oss() << "(" << m_internal->identify(cols[i]) << ":" << vals[i] << ") ";
    } else {
      oss() << global_line_index << " -> ";
      for (Arccore::Integer i = 0; i < ncols; ++i)
        oss() << "(" << cols[i] << ":" << vals[i] << ") ";
    }

    checkError("Restore row", MatRestoreRow(m_petsc_matrix.internal()->m_internal,
                                  global_line_index, &ncols, &cols, &vals));
    return oss.str();
  }

  /*---------------------------------------------------------------------------*/

  std::map<Arccore::Integer, Arccore::Real> MatrixRowPrinter::extract(
      const Arccore::Integer global_line_index) const
  {
    PetscInt rstart, rend;
    checkError("Extract Matrix OwnershipRange",
        MatGetOwnershipRange(m_petsc_matrix.internal()->m_internal, &rstart, &rend));
    if (global_line_index < rstart or global_line_index >= rend) {
      return std::map<Arccore::Integer, Arccore::Real>();
    }

    const PetscInt* cols = PETSC_NULL;
    const PetscScalar* vals = PETSC_NULL;
    PetscInt ncols = 0;
    checkError("Extract row", MatGetRow(m_petsc_matrix.internal()->m_internal,
                                  global_line_index, &ncols, &cols, &vals));
    RowData result;
    for (Arccore::Integer i = 0; i < ncols; ++i)
      result[cols[i]] = vals[i];
    checkError("Restore row", MatRestoreRow(m_petsc_matrix.internal()->m_internal,
                                  global_line_index, &ncols, &cols, &vals));
    return result;
  }

  /*---------------------------------------------------------------------------*/

  void MatrixRowPrinter::checkError(const Arccore::String& msg, int ierr) const
  {
    if (ierr != 0) {
      const char* text;
      char* specific;
      PetscErrorMessage(ierr, &text, &specific);
      throw Arccore::FatalErrorException(
          A_FUNCINFO, Arccore::String::format("{0} failed : {1} / {2} [code={3}]", msg,
                          text, specific, ierr));
    }
  }

  /*---------------------------------------------------------------------------*/

} // namespace Alien

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/