// -*- C++ -*-
#ifndef ALIEN_HTSIMPL_HTSVECTOR_H
#define ALIEN_HTSIMPL_HTSVECTOR_H
/* Author : mesriy at Tue Jul 24 14:28:21 2012
 * Generated by createNew
 */

#include <ALIEN/Kernels/HTS/HTSPrecomp.h>
#include <alien/core/impl/IVectorImpl.h>
#include <alien/data/ISpace.h>
#include <alien/distribution/VectorDistribution.h>

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

BEGIN_HTSINTERNAL_NAMESPACE

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
template<typename ValueT,bool is_mpi>
class VectorInternal;

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

END_HTSINTERNAL_NAMESPACE

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

namespace Alien {

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
template <typename ValueT, bool is_mpi> class HTSVector : public IVectorImpl
{
 public:
  typedef HTSInternal::VectorInternal<ValueT, is_mpi> VectorInternal;

 public:

  HTSVector(const MultiVectorImpl * multi_impl);

  virtual ~HTSVector();

public:

  void init(const VectorDistribution & dist, const bool need_allocate);
  void allocate();

  void free() { }
  void clear() { }

public:

  void setValues(const int nrows, ValueT const* values);

  void getValues(const int nrows, ValueT* values) const;

public:

  VectorInternal * internal() { return m_internal.get() ; }

  const VectorInternal * internal() const{ return m_internal.get() ; }

  void dump() const;
private:

  bool assemble();

private :

  std::unique_ptr<VectorInternal> m_internal;
  int m_local_offset;
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

} // namespace Alien

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#endif /* ALIEN_HTSIMPL_HTSVECTOR_H */
