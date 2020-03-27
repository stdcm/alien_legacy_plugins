/* Author : havep at Tue Jul 29 18:45:26 2008
 * Generated by createNew
 */

#ifndef PETSCPRECCONFIGHYPRESERVICE_H
#define PETSCPRECCONFIGHYPRESERVICE_H

#include <ALIEN/Kernels/PETSc/PETScPrecomp.h>
#include <ALIEN/Alien-ExternalPackagesPrecomp.h>

#include <alien/utils/parameter_manager/BaseParameterManager.h>

#include <ALIEN/Kernels/PETSc/LinearSolver/IPETScPC.h>
#include <ALIEN/Kernels/PETSc/LinearSolver/IPETScKSP.h>
#include <ALIEN/Kernels/PETSc/LinearSolver/PETScConfig.h>

struct PETScPrecConfigHypreOptions {
  enum eType 
    {
      PILUT,
      AMG,
      ParaSails,
      Euclid
    };
};

#include <ALIEN/axl/PETScPrecConfigHypre_axl.h>

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

// BEGIN_LINEARALGEBRA2SERVICE_NAMESPACE

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
namespace Alien {

class ALIEN_EXTERNALPACKAGES_EXPORT PETScPrecConfigHypreService
: public ArcanePETScPrecConfigHypreObject,
  public PETScConfig
{
 public:
  /** Constructeur de la classe */
#ifdef ALIEN_USE_ARCANE
  PETScPrecConfigHypreService(const Arcane::ServiceBuildInfo& sbi);
#endif
  PETScPrecConfigHypreService(Arccore::MessagePassing::IMessagePassingMng* parallel_mng,
      std::shared_ptr<IOptionsPETScPrecConfigHypre> options);

  /** Destructeur de la classe */
  virtual ~PETScPrecConfigHypreService() {}

  void configure(PC & pc,
                 const ISpace& space,
                 const MatrixDistribution& distribution);

  //! Check need of KSPSetUp before calling this PC configure
  virtual bool needPrematureKSPSetUp() const { return false; }

};
} // namespace Alien
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

// END_LINEARALGEBRA2SERVICE_NAMESPACE

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#endif //PETSCPRECCONFIGHYPRESERVICE_H
