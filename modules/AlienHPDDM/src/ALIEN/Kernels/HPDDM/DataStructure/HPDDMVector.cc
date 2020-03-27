#include "HPDDMVector.h"
/* Author : mesriy at Tue Jul 24 15:28:21 2012
 * Generated by createNew
 */

#include <ALIEN/Kernels/HPDDM/DataStructure/HPDDMInternal.h>
#include <ALIEN/Kernels/HPDDM/HPDDMBackEnd.h>
#include <alien/core/block/Block.h>

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

BEGIN_NAMESPACE(Alien)

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
template<typename ValueT>
HPDDMVector<ValueT>::
HPDDMVector(const MultiVectorImpl * multi_impl)
: IVectorImpl(multi_impl, AlgebraTraits<BackEnd::tag::hpddm>::name())
  , m_local_offset(0) {}

/*---------------------------------------------------------------------------*/
template<typename ValueT>
HPDDMVector<ValueT>::~HPDDMVector()
{}

/*---------------------------------------------------------------------------*/
template<typename ValueT>
void
HPDDMVector<ValueT>::init(const VectorDistribution& dist, const bool need_allocate)
{
  if (need_allocate)
    allocate();
}

/*---------------------------------------------------------------------------*/
template<typename ValueT>
void
HPDDMVector<ValueT>::allocate()
{
  const VectorDistribution& dist = this->distribution();
  m_local_offset = dist.offset();

  //m_internal.reset(new VectorInternal(this->scalarizedLocalSize()));

}

/*---------------------------------------------------------------------------*/

template<typename ValueT>
void
HPDDMVector<ValueT>::setValues(const int nrow, const ValueT* values)
{
}

/*---------------------------------------------------------------------------*/
template<typename ValueT>
void
HPDDMVector<ValueT>::getValues(const int nrow, ValueT* values) const
{
  //for (int i = 0; i < nrow; ++i)
  //  values[i] = m_internal->m_data[i];
}

template<typename ValueT>
void
HPDDMVector<ValueT>::
dump() const
{
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
template class HPDDMVector<double> ;

END_NAMESPACE

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
