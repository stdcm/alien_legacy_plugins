// -*- C++ -*-
#ifndef ALIEN_IFPSOLVERIMPL_IFPSOLVERINTERNAL_H
#define ALIEN_IFPSOLVERIMPL_IFPSOLVERINTERNAL_H
/* Author : havep at Fri Jul 20 13:21:25 2012
 * Generated by createNew
 */

#include <ALIEN/Kernels/IFP/IFPSolverPrecomp.h>
#include <alien/data/ISpace.h>

/*---------------------------------------------------------------------------*/

namespace Alien {

class MultiMatrixImpl;
class MultiVectorImpl;

} // namespace Alien

/*---------------------------------------------------------------------------*/

BEGIN_IFPSOLVERINTENRAL_NAMESPACE

/*---------------------------------------------------------------------------*/

class MatrixInternal
{
public:
  MatrixInternal(const MultiMatrixImpl * matrix_impl, Int64 timestamp=-1);
  virtual ~MatrixInternal();
  void init() ;
    Int64 timestamp() const {
    return m_timestamp ;
  }
public:
  bool m_filled               = false ;
  bool m_extra_filled         = false ;
  bool m_elliptic_split_tag   = false ;
  bool m_system_is_resizeable = false ;
  Int64 m_timestamp           = -1 ;

public:
  static bool isInstancied() { return m_static_multi_impl != NULL; }
  static const MultiMatrixImpl * multiImpl() { return m_static_multi_impl; }

private:
  static const MultiMatrixImpl * m_static_multi_impl;
};

/*---------------------------------------------------------------------------*/

class VectorInternal
{
private:
  friend class MatrixInternal;

public :
  VectorInternal(const MultiVectorImpl * vector_impl);
  virtual ~VectorInternal();

public:
  int  m_offset ;
  int  m_local_size ;
  int* m_rows ;
  bool m_filled;
  bool m_extra_filled;

public:
  static bool isInstancied() { return m_static_multi_impl != NULL; }
  static void setRepresentationSwitch(const bool s);
  static bool hasRepresentationSwitch();
  static void initRHS(const bool s);
  static int isRHS() ;

private:
  //! Tant que m_static_representation_switch est activé, les mises à jour en écriture sont désactivées
  static bool m_static_representation_switch;
  static const MultiVectorImpl * m_static_multi_impl;
  static int m_static_init_rhs ;
};

/*---------------------------------------------------------------------------*/

END_IFPSOLVERINTERNAL_NAMESPACE

/*---------------------------------------------------------------------------*/

#endif /* ALIEN_IFPSOLVERIMPL_IFPSOLVERINTERNAL_H */
