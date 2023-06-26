/* Author : havep at Mon Jun 30 17:55:25 2008
 * Generated by createNew
 */

#include <alien/kernels/petsc/linear_solver/arcane/PETScPrecConfigJacobiService.h>
#include <ALIEN/axl/PETScPrecConfigJacobi_StrongOptions.h>

#include <arccore/message_passing/IMessagePassingMng.h>

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

namespace Alien {

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

/** Constructeur de la classe */

#ifdef ALIEN_USE_ARCANE
PETScPrecConfigJacobiService::PETScPrecConfigJacobiService(
    const Arcane::ServiceBuildInfo& sbi)
: ArcanePETScPrecConfigJacobiObject(sbi)
, PETScConfig(sbi.subDomain()->parallelMng()->isParallel())
{
  ;
}
#endif

PETScPrecConfigJacobiService::PETScPrecConfigJacobiService(
    Arccore::MessagePassing::IMessagePassingMng* parallel_mng,
    std::shared_ptr<IOptionsPETScPrecConfigJacobi> options)
: ArcanePETScPrecConfigJacobiObject(options)
, PETScConfig(parallel_mng->commSize() > 1)
{
  ;
}

//! Initialisation
void
PETScPrecConfigJacobiService::configure(
    PC& pc, const ISpace& space, const MatrixDistribution& distribution)
{
  alien_debug([&] { cout() << "configure PETSc block jacobi preconditioner"; });
  checkError("Set preconditioner", PCSetType(pc, PCBJACOBI));
  // KSPSetUp has been already called (cf needPrematureKSPSetUp)
  // you can set up sub solver using PCBJacobiGetSubKSP (for PCBJACOBI ie by block)
  // details in
  // http://www-unix.mcs.anl.gov/petsc/petsc-as/snapshots/petsc-current/src/ksp/ksp/examples/tutorials/ex7.c.html

  checkError("Preconditioner setup", PCSetUp(pc));

  // Extract the array of KSP contexts for the local blocks
  PetscInt nlocal, first;
  KSP* subksp;
  checkError("Preconditioner gets sub-solvers",
      PCBJacobiGetSubKSP(pc, &nlocal, &first, &subksp));

  int level = options()->level();
  double shift = options()->shift();

  for (Arccore::Integer i = first; i < nlocal; i++) {
    PC subpc;
    // checkError("Preconditioner sub-solver config",KSPSetType(subksp[i],KSPPREONLY));
    checkError("Preconditioner sub-preconditioner", KSPGetPC(subksp[i], &subpc));
    checkError("Preconditioner sub-preconditioner setup", PCSetType(subpc, PCILU));
    checkError(
        "Preconditioner sub-preconditioner config", PCFactorSetLevels(subpc, level));
    checkError(
        "Preconditioner sub-preconditioner config", PCFactorSetShiftAmount(subpc, shift));
  }
}

//! Check need of KSPSetUp before calling this PC configure
bool
PETScPrecConfigJacobiService::needPrematureKSPSetUp() const
{
  return true;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

ARCANE_REGISTER_SERVICE_PETSCPRECCONFIGJACOBI(Jacobi, PETScPrecConfigJacobiService);
ARCANE_REGISTER_SERVICE_PETSCPRECCONFIGJACOBI(ILU, PETScPrecConfigJacobiService);
ARCANE_REGISTER_SERVICE_PETSCPRECCONFIGJACOBI(BlockILU, PETScPrecConfigJacobiService);

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

} // namespace Alien

REGISTER_STRONG_OPTIONS_PETSCPRECCONFIGJACOBI();

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/