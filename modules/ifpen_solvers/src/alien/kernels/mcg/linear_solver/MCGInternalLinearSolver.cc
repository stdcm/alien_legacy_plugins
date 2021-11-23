/* Author : desrozis at Mon Mar 30 15:06:37 2009
 * Generated by createNew
 */
#define MPICH_SKIP_MPICXX 1
#include "mpi.h"
#include <tuple>
#include <iomanip>
#include <regex>

#include <arccore/message_passing_mpi/MpiMessagePassingMng.h>

#include <MCGS.h>

#include <alien/data/Space.h>
#include <alien/expression/solver/ILinearSolver.h>
#include <alien/expression/solver/ILinearAlgebra.h>
#include <alien/expression/solver/SolverStat.h>
#include <alien/expression/solver/SolverStater.h>
#include <alien/core/impl/MultiMatrixImpl.h>
#include <alien/core/impl/MultiVectorImpl.h>
#include <alien/kernels/simple_csr/SimpleCSRPrecomp.h>
#include <alien/kernels/simple_csr/algebra/SimpleCSRLinearAlgebra.h>
#include <alien/core/backend/LinearSolverT.h>
#include <alien/core/block/ComputeBlockOffsets.h>

#include "alien/kernels/mcg/algebra/MCGLinearAlgebra.h"
#include "alien/kernels/mcg/MCGPrecomp.h"
#include "alien/kernels/mcg/data_structure/MCGVector.h"
#include "alien/kernels/mcg/data_structure/MCGMatrix.h"
#include "alien/kernels/mcg/data_structure/MCGCompositeVector.h"
#include "alien/kernels/mcg/data_structure/MCGCompositeMatrix.h"
#include "alien/kernels/mcg/data_structure/MCGInternal.h"
#include "alien/kernels/mcg/linear_solver/MCGOptionTypes.h"
#include "alien/kernels/mcg/linear_solver/MCGInternalLinearSolver.h"

#include "ALIEN/axl/MCGSolver_IOptions.h"

namespace Alien {

std::unique_ptr<MCGInternalLinearSolver::AlienKOpt2MCGKOpt>
    MCGInternalLinearSolver::AlienKOpt2MCGKOpt::m_instance;

MCGInternalLinearSolver::AlienKOpt2MCGKOpt::AlienKOpt2MCGKOpt()
{
  m_option_translate[{ MCGOptionTypes::CPU_CBLAS_BCSR, false, false }] =
      MCGSolver::CPUCBLAS;
  m_option_translate[{ MCGOptionTypes::CPU_CBLAS_BCSR, true, false }] =
      MCGSolver::MPI_CPUCBLAS;
  m_option_translate[{ MCGOptionTypes::CPU_CBLAS_BCSR, false, true }] =
      MCGSolver::OMP_CPUCBLAS;
  m_option_translate[{ MCGOptionTypes::CPU_CBLAS_BCSR, true, true }] =
      MCGSolver::MPI_OMP_CPUCBLAS;

  m_option_translate[{ MCGOptionTypes::CPU_AVX_BCSR, false, false }] =
      MCGSolver::CPUAVX;
  m_option_translate[{ MCGOptionTypes::CPU_AVX_BCSR, true, false }] =
      MCGSolver::MPI_CPUAVX;
  m_option_translate[{ MCGOptionTypes::CPU_AVX_BCSR, false, true }] =
      MCGSolver::OMP_CPUAVX;
  m_option_translate[{ MCGOptionTypes::CPU_AVX_BCSR, true, true }] =
      MCGSolver::MPI_OMP_CPUAVX;

  m_option_translate[{ MCGOptionTypes::CPU_AVX2_BCSP, false, false }] =
      MCGSolver::CPUAVX2;
  m_option_translate[{ MCGOptionTypes::CPU_AVX2_BCSP, true, false }] =
      MCGSolver::MPI_CPUAVX2;
  m_option_translate[{ MCGOptionTypes::CPU_AVX2_BCSP, false, true }] =
      MCGSolver::OMP_CPUAVX2;
  m_option_translate[{ MCGOptionTypes::CPU_AVX2_BCSP, true, true }] =
      MCGSolver::MPI_OMP_CPUAVX2;

  m_option_translate[{ MCGOptionTypes::CPU_AVX512_BCSP, false, false }] =
      MCGSolver::CPUAVX512;
  m_option_translate[{ MCGOptionTypes::CPU_AVX512_BCSP, true, false }] =
      MCGSolver::MPI_CPUAVX512;
  m_option_translate[{ MCGOptionTypes::CPU_AVX512_BCSP, false, true }] =
      MCGSolver::OMP_CPUAVX512;
  m_option_translate[{ MCGOptionTypes::CPU_AVX512_BCSP, true, true }] =
      MCGSolver::MPI_OMP_CPUAVX512;

  m_option_translate[{ MCGOptionTypes::GPU_CUBLAS_BELL, false, false }] =
      MCGSolver::GPUCUBLASBELLSpmv;
  m_option_translate[{ MCGOptionTypes::GPU_CUBLAS_BELL, true, false }] =
      MCGSolver::MPI_GPUCUBLASBELLSpmv;

  m_option_translate[{ MCGOptionTypes::GPU_CUBLAS_BCSP, false, false }] =
      MCGSolver::GPUCUBLASBCSPSpmv;
  m_option_translate[{ MCGOptionTypes::GPU_CUBLAS_BCSP, true, false }] =
      MCGSolver::MPI_GPUCUBLASBCSPSpmv;
}

MCGSolver::eKernelType
MCGInternalLinearSolver::AlienKOpt2MCGKOpt::getKernelOption(
    const KConfigType& kernel_config)
{
  if (!m_instance) {
    m_instance.reset(new AlienKOpt2MCGKOpt);
  }

  const auto& p = m_instance->m_option_translate.find(kernel_config);

  if (p != m_instance->m_option_translate.cend()) {
    return p->second;
  } else {
    m_instance->alien_fatal([&] { m_instance->cout() << "Unknow kernel configuration"; });
    throw FatalErrorException(__PRETTY_FUNCTION__); // just build for warning
  }
}

/*---------------------------------------------------------------------------*/
MCGInternalLinearSolver::MCGInternalLinearSolver(
    Arccore::MessagePassing::IMessagePassingMng* parallel_mng, IOptionsMCGSolver* options)
: m_parallel_mng(parallel_mng)
, m_options(options)
{
  m_dir_enum[std::string("+Z")] = *((int*)"+Z-Z");
  m_dir_enum[std::string("-Z")] = *((int*)"-Z+Z");
  m_dir_enum[std::string("+X")] = *((int*)"+X-X");
  m_dir_enum[std::string("-X")] = *((int*)"-X+X");
  m_dir_enum[std::string("+Y")] = *((int*)"+Y-Y");
  m_dir_enum[std::string("-Y")] = *((int*)"-Y+Y");
  m_dir_enum[std::string("+D")] = *((int*)"+D-D");
  m_dir_enum[std::string("-D")] = *((int*)"-D+D");

  // check version
  const std::string expected_version("v1.2");
  const std::regex expected_revision_regex("^"+ expected_version +".*");
  m_version = MCGSolver::LinearSolver::getRevision();

  if(!std::regex_match(m_version,expected_revision_regex))
  {
    alien_info([&] { cout()<<"MCGSolver version mismatch: expect " << expected_version << " get " << m_version ; });
  }

  const std::regex end_regex("-[[:alnum:]]+$");
  m_version = std::regex_replace(m_version,end_regex,"");
}

/*---------------------------------------------------------------------------*/

MCGInternalLinearSolver::~MCGInternalLinearSolver()
{
  delete m_system;
  delete m_solver;
  delete m_machine_info;
  delete m_mpi_info;
}
/*---------------------------------------------------------------------------*/

std::shared_ptr<Alien::ILinearAlgebra>
MCGInternalLinearSolver::algebra() const
{
  return std::shared_ptr<Alien::ILinearAlgebra>(new Alien::MCGLinearAlgebra());
}
/*---------------------------------------------------------------------------*/

void
MCGInternalLinearSolver::init()
{
  m_init_timer.start();

  if (m_parallel_mng == nullptr)
    return;

  m_output_level = m_options->output();
  if(m_output_level > 0)
  {
    alien_info([&]{
        printf("MCVSolver version %s\n",MCGSolver::LinearSolver::getRevision().c_str());
    });
  }
  m_use_unit_diag = false;
  m_keep_diag_opt = m_options->keepDiagOpt();
  m_normalize_opt = m_options->normalizeOpt();

  m_use_mpi = m_parallel_mng->commSize() > 1;

  auto mpi_mng =
      dynamic_cast<Arccore::MessagePassing::Mpi::MpiMessagePassingMng*>(m_parallel_mng);

  m_machine_info = new MCGSolver::MachineInfo;
  m_machine_info->init(m_parallel_mng->commRank() == 0);

  if (m_use_mpi) {
    MPI_Comm comm = *static_cast<const MPI_Comm*>(mpi_mng->getMPIComm());
    m_mpi_info = new mpi::MPIInfo;
    m_mpi_info->init(comm, false);
  }
  m_use_thread = m_options->useThread();
  m_num_thread = m_options->numThread(); // deprecated

  m_max_iteration = m_options->maxIterationNum();
  m_precision = m_options->stopCriteriaValue();
  m_precond_opt = m_options->preconditioner();

  m_solver_opt = m_options->solver();
  // ConstArrayView<String> parameter = m_options->parameter() ;

  m_solver = new MCGSolver::LinearSolver();

  m_solver->setMachineInfo(m_machine_info);
  if (m_use_mpi)
    m_solver->initMPIInfo(m_mpi_info);

  m_solver->setOpt(MCGSolver::OutputLevel, m_output_level - 1);
  m_solver->setOpt(MCGSolver::SolverMaxIter, m_max_iteration);
  m_solver->setOpt(MCGSolver::SolverEps, m_precision);
  if (m_use_thread) {
    const char* env_num_thread = getenv("OMP_NUM_THREADS");
    if (env_num_thread != nullptr) {
      m_num_thread = std::atoi(env_num_thread);
      if (m_output_level > 0) {
        alien_info([&] {
          printf(
              "MCGInternalLinearSolver: set num_thread to %d from env\n", m_num_thread);
        });
      }
    }
    m_solver->setOpt(MCGSolver::UseOmpThread, m_use_thread);
    m_solver->setOpt(MCGSolver::NumThread, m_num_thread);
    m_solver->setOpt(MCGSolver::SharedMemPart, MCGSolver::Graph::Partitioner::METIS_KWAY);
  }
  m_solver->setOpt(MCGSolver::NormalizeOpt, m_normalize_opt);

  if (m_options->exportSystem())
    m_solver->setOpt(MCGSolver::ExportSystemFileName,
        std::string(localstr(m_options->exportSystemFileName())));

  m_solver->setOpt(MCGSolver::SolverType,m_solver_opt);

  m_solver->setOpt(MCGSolver::PrecondOpt,m_precond_opt);

  m_solver->setOpt(MCGSolver::PolyOrder, m_options->polyOrder());
  m_solver->setOpt(MCGSolver::PolyFactor, m_options->polyFactor());
  m_solver->setOpt(MCGSolver::PolyFactorMaxIter, m_options->polyFactorNumIter());

  m_solver->setOpt(MCGSolver::BlockJacobiNumOfIter, m_options->bjNumIter());
  m_solver->setOpt(MCGSolver::BlockJacobiLocalSolverOpt,m_options->bjLocalPrecond());

  m_solver->setOpt(MCGSolver::FPILUSolverNumIter, m_options->fpilu0SolveNumIter());
  m_solver->setOpt(MCGSolver::FPILUFactorNumIter, m_options->fpilu0FactoNumIter());

  m_solver->setOpt(MCGSolver::SpPrec, m_options->spPrec());

  if(!m_options->ILUk().empty()){
    m_solver->setOpt(MCGSolver::ILUkLevel, m_options->ILUk()[0]->levelOfFill());
    // override spPrec
    m_solver->setOpt(MCGSolver::SpPrec, m_options->ILUk()[0]->sp());
  }

#if 0
  switch (m_precond_opt) {
  case MCGOptionTypes::PolyPC:
    m_solver->setOpt(MCGSolver::PrecondOpt,MCGSolver::PrecPoly);
    break;
  case MCGOptionTypes::FixpILU0PC:
    m_solver->setOpt(MCGSolver::PrecondOpt,MCGSolver::PrecFixPointILU0);
    break;
  case MCGOptionTypes::ILU0PC:
    m_solver->setOpt(MCGSolver::PrecondOpt,MCGSolver::PrecILU0);
     break;
  case MCGOptionTypes::BlockILU0PC:
    m_solver->setOpt((Integer)MCGSolver::PrecondOpt, (Integer)MCGSolver::PrecBlockILU0);
    break;
  case MCGOptionTypes::BlockJacobiPC:
    m_solver->setOpt((Integer)MCGSolver::PrecondOpt,(Integer)MCGSolver::PrecBlockJacobi);
    break;
#if 0
	case MCGOptionTypes::ColorBlockILU0PC :
	  m_dir = std::string(localstr(m_options->colorilu0Dir())) ;
	  m_solver->setOpt((Integer)MCGSolver::PrecondOpt,(Integer)MCGSolver::Precond::ColorBlockILU0) ;
    m_solver->setOpt(MCGSolver::CBILU0ColorMethod,std::string(localstr(m_options->colorilu0Algo())));
    m_solver->setOpt(MCGSolver::CBILU0Dir,((short *)&(m_dir_enum[m_dir]))[0]);
    m_solver->setOpt(MCGSolver::CBILU0OpDir,((short *)&(m_dir_enum[m_dir]))[1]);
	  break ;
  case MCGOptionTypes::CprPC :
    m_solver->setOpt((Integer)MCGSolver::PrecondOpt,(Integer)MCGSolver::Precond::Cpr) ;
    m_solver->setOpt((Integer)MCGSolver::ReorderOpt,m_options->reorderOpt()?1:0) ;
    m_solver->setOpt((Integer)MCGSolver::InterfaceOpt,m_options->interfaceOpt()) ;
    m_solver->setOpt((Integer)MCGSolver::CxrSolver,m_options->cprSolver()) ;
    m_solver->setOpt((Integer)MCGSolver::RelaxSolver,m_options->relaxSolver()) ;
    m_solver->setOpt(MCGSolver::AmgAlgo, std::string(localstr(m_options->amgAlgo())));
    break ;
#endif
  case MCGOptionTypes::NonePC:
    m_solver->setOpt((Integer)MCGSolver::PrecondOpt, (Integer)MCGSolver::PrecNone);
    break;
  default:;
  }
#endif

  m_solver->init(AlienKOpt2MCGKOpt::getKernelOption(
      { m_options->kernel(), m_use_mpi, m_use_thread }));

  m_init_timer.stop();
}

void
MCGInternalLinearSolver::updateParallelMng(
    Arccore::MessagePassing::IMessagePassingMng* pm)
{
  // TODO: do we really want to do that ?
  m_parallel_mng = pm;
}

/*---------------------------------------------------------------------------*/

void
MCGInternalLinearSolver::end()
{}

Integer
MCGInternalLinearSolver::_solve(const MCGMatrixType& A, const MCGVectorType& b,
    MCGVectorType& x, MCGSolver::PartitionInfo* part_info)
{
  if (m_output_level > 0) {
    alien_info([&] {
      cout() << "MCGInternalLinearSolver::_solve A:" << A.m_matrix[0][0].get()
             << " b:" << &b << " x:" << &x;
    });
  }

  Integer error = -1;

  m_system_timer.start();
  if (m_system == nullptr) {
    m_system = _createSystem(A, b, x, part_info);
    // if(!m_edge_weight.empty()) {
    // m_system->setEdgeWeight(m_edge_weight);
    // }
  }
  // set/update
  m_system_timer.stop();

  m_solve_timer.start();
  error = m_solver->solve(m_system, &m_mcg_status);
  m_solve_timer.stop();

  return error;
}

Integer
MCGInternalLinearSolver::_solve(const MCGMatrixType& A, const MCGVectorType& b,
    const MCGVectorType& x0, MCGVectorType& x, MCGSolver::PartitionInfo* part_info)
{
  if (m_output_level > 0)
    alien_info([&] {
      cout() << "MCGInternalLinearSolver::_solve with x0"
             << " A:" << &A << " b:" << &b << " x0:" << &x0 << " x:" << &x;
    });

  Integer error = -1;

  if (m_system == nullptr) {
    m_system_timer.start();
    m_system = _createSystem(A, b, x0, x, part_info);
    m_system_timer.stop();
  }

  m_solve_timer.start();
  error = m_solver->solve(m_system, &m_mcg_status);
  m_solve_timer.stop();

  return error;
}

Integer
MCGInternalLinearSolver::_solve(const MCGMatrixType& A, const MCGCompositeVectorType& b,
    MCGCompositeVectorType& x, MCGSolver::PartitionInfo* part_info)
{
  if (m_output_level > 0)
    alien_info([&] { cout() << "MCGInternalLinearSolver::_solve with composite"; });

  Integer error = -1;

  if (m_system == nullptr) {
    m_system_timer.start();
    m_system = _createSystem(A, b, x, part_info);
    m_system_timer.stop();
  }

  m_solve_timer.start();
  error = m_solver->solve(m_system, &m_mcg_status);
  m_solve_timer.stop();

  return error;
}

Integer
MCGInternalLinearSolver::_solve(const MCGMatrixType& A, const MCGCompositeVectorType& b,
    const MCGCompositeVectorType& x0, MCGCompositeVectorType& x,
    MCGSolver::PartitionInfo* part_info)
{
  if (m_output_level > 0)
    alien_info([&] { cout() << "MCGInternalLinearSolver::_solve composite with x0"; });

  Integer error = -1;

  if (m_system == nullptr) {
    m_system_timer.start();
    m_system = _createSystem(A, b, x0, x, part_info);
    m_system_timer.stop();
  }

  m_solve_timer.start();
  error = m_solver->solve(m_system, &m_mcg_status);
  m_solve_timer.stop();

  return error;
}

MCGSolver::LinearSystem<double>*
MCGInternalLinearSolver::_createSystem(const MCGMatrixType& A, const MCGVectorType& b,
    MCGVectorType& x, MCGSolver::PartitionInfo* part_info)
{
  MCGSolver::LinearSystem<double>* system = MCGSolver::LinearSystem<double>::create(
      A.m_matrix[0][0].get(), &b.m_bvector, &x.m_bvector, part_info, m_mpi_info);
  return system;
}

MCGSolver::LinearSystem<double>*
MCGInternalLinearSolver::_createSystem(const MCGMatrixType& A, const MCGVectorType& b,
    const MCGVectorType& x0, MCGVectorType& x, MCGSolver::PartitionInfo* part_info)
{
  MCGSolver::LinearSystem<double>* system =
      MCGSolver::LinearSystem<double>::create(A.m_matrix[0][0].get(), &b.m_bvector,
          &x.m_bvector, &x0.m_bvector, part_info, m_mpi_info);
  return system;
}

MCGSolver::LinearSystem<double>*
MCGInternalLinearSolver::_createSystem(const MCGMatrixType& A,
    const MCGCompositeVectorType& b, MCGCompositeVectorType& x,
    MCGSolver::PartitionInfo* part_info)
{
  MCGSolver::LinearSystem<double>* system = MCGSolver::LinearSystem<double>::create(
      A.m_matrix[0][0].get(), A.m_matrix[1][1].get(), A.m_matrix[1][0].get(),
      A.m_matrix[0][1].get(), &b.m_bvector[0], &b.m_bvector[1], &x.m_bvector[0],
      &x.m_bvector[1], part_info, nullptr, m_mpi_info);
  return system;
}

MCGSolver::LinearSystem<double>*
MCGInternalLinearSolver::_createSystem(const MCGMatrixType& A,
    const MCGCompositeVectorType& b, const MCGCompositeVectorType& x0,
    MCGCompositeVectorType& x, MCGSolver::PartitionInfo* part_info)
{
  MCGSolver::LinearSystem<double>* system =
      MCGSolver::LinearSystem<double>::create(A.m_matrix[0][0].get(),
          A.m_matrix[1][1].get(), A.m_matrix[1][0].get(), A.m_matrix[0][1].get(),
          &b.m_bvector[0], &b.m_bvector[1], &x.m_bvector[0], &x.m_bvector[1],
          &x0.m_bvector[0], &x0.m_bvector[1], part_info, nullptr, m_mpi_info);
  return system;
}

/*---------------------------------------------------------------------------*/

const Alien::SolverStatus&
MCGInternalLinearSolver::getStatus() const
{
  return m_status;
}

void
MCGInternalLinearSolver::printInfo() const
{
  double total_solve_time = m_solve_timer.getElapse() + m_system_timer.getElapse();
  alien_info([&] {
    cout() << "\n|----------------------------------------------------|\n"
                "| Linear Solver        : MCGSolver " << m_version << "\n"
                "|----------------------------------------------------|\n"
           << std::scientific << std::setprecision(4)
           << "| total num of iter           : " << m_total_iter_num << "\n"
           << "| solve num                   : " << m_solve_num << "\n"
           << "| init time                   : " << m_init_timer.getElapse() << "\n"
           << "| prepare time                : " << m_prepare_timer.getElapse() << "\n"
           << "| total solver time           : " << total_solve_time << "\n"
           << "| |--system time              : " << m_system_timer.getElapse() << " "
           << m_system_timer.getElapse() / total_solve_time << "\n"
           << "| |--solve time               : " << m_solve_timer.getElapse() << " "
           << m_solve_timer.getElapse() / total_solve_time << "\n"
           << "|    |--internal setup time   : " << m_int_total_setup_time << " "
           << m_int_total_setup_time / total_solve_time << "\n"
           << "|    |--internal allocate time: " << m_int_total_allocate_time << " "
           << m_int_total_allocate_time / total_solve_time << "\n"
           << "|    |--internal init time    : " << m_int_total_init_time << " "
           << m_int_total_init_time / total_solve_time << "\n"
           << "|    |--internal udpdate time : " << m_int_total_update_time << " "
           << m_int_total_update_time / total_solve_time << "\n"
           << "|    |--internal solve time   : " << m_int_total_solve_time << " "
           << m_int_total_solve_time / total_solve_time << "\n"
           << "|    |--internal finish time  : " << m_int_total_finish_time << " "
           << m_int_total_finish_time / total_solve_time << "\n"
           << std::defaultfloat
           << "|----------------------------------------------------|\n";
  });
}

bool
MCGInternalLinearSolver::solve(IMatrix const& A, IVector const& b, IVector& x)
{
  m_prepare_timer.start();
  Integer error = -1;

  if (m_parallel_mng == nullptr)
    return true;

  if (m_output_level > 0) {
    alien_info([&] {
      cout() << "MCGInternalLinearSolver::solve A timestamp: " << A.impl()->timestamp();
      cout() << "MCGInternalLinearSolver::solve b timestamp: " << b.impl()->timestamp();
      cout() << "MCGInternalLinearSolver::solve x timestamp: " << x.impl()->timestamp();
    });
  }

  using namespace Alien;
  using namespace Alien::MCGInternal;

  MCGSolver::PartitionInfo* part_info = nullptr;

  if (A.impl()->hasFeature("composite")) {
    MCGCompositeMatrix const& matrix = A.impl()->get<BackEnd::tag::mcgsolver_composite>();
    MCGCompositeVector const& rhs = b.impl()->get<BackEnd::tag::mcgsolver_composite>();
    MCGCompositeVector& sol = x.impl()->get<BackEnd::tag::mcgsolver_composite>(true);

    if (m_use_mpi) {
#if 1
      throw Alien::FatalErrorException(
          "composite not yet supported with MCGSolver when using MPI");
#else
      ConstArrayView<int> offsets;
      ConstArrayView<int> woffsets;
      UniqueArray<Integer> blockOffsets;
      // Why block size is duplicated
      int block_size;
      Integer nproc = m_parallel_mng->commSize();
      if (A.impl()->block()) {
        computeBlockOffsets(matrix.distribution(), *A.impl()->block(), blockOffsets);
        int blockSize = A.impl()->block()->size();
#ifdef ALIEN_USE_ARCANE
        offsets = blockOffsets.constView();
#else
        offsets = ConstArrayView<int>(blockOffsets);
#endif
        block_size = blockSize;
      } else {
        Integer loffset = matrix.distribution().rowOffset();
        UniqueArray<Integer> scalarOffsets;

        scalarOffsets.resize(nproc + 1);
        m_parallel_mng->allGather(ConstArrayView<int>(1, &loffset),
            ArrayView<int>(nproc, dataPtr(scalarOffsets)));
        scalarOffsets[nproc] = matrix.distribution().globalRowSize();
#ifdef ALIEN_USE_ARCANE
        offsets = scalarOffsets.constView();
#else
        offsets = ConstArrayView<int>(scalarOffsets);
#endif
        block_size = 1;
      }

      const int* offsets1_tmp = matrix.get_row_offset1();
      const int* offsets2_tmp = matrix.get_row_offset2();

      // m_parallel_context->getPartitionInfo().init((int*)dataPtr(offsets),offsets.size(),block_size)
      // ;
      m_parallel_context->getPartitionInfo().init(
          (int*)offsets1_tmp, (std::size_t)(nproc + 1), block_size);

      // XT (24/03/2016) : there is something wrong here -> two different values of
      // offsets are used in the former version. Need to understand what they represent.
      // m_parallel_context->getExtraPartitionInfo().init((int*)dataPtr(woffsets),woffsets.size())
      // ;
      m_parallel_context->getExtraPartitionInfo().init(
          (int*)offsets2_tmp, (std::size_t)(nproc + 1));
      /*
      //ConstArrayView<int> offsets = matrix.space().structInfo().getOffsets() ;
      const Block* block
      ConstArrayView<int> offsets = A.impl()->block()->getOffsets();
      int block_size =  A.impl()->block()->size() ;
      m_parallel_context->getPartitionInfo().init((int*)dataPtr(offsets),offsets.size(),block_size)
      ;
      // COUPLED SYSTEM DISTRIBUTION
      //ConstArrayView<int> offsets1 = matrix.space1().structInfo().getOffsets() ;
      ConstArrayView<int> offsets1 = A.impl()->block()->getOffsets() ;
      m_parallel_context->getExtraPartitionInfo().init((int*)dataPtr(offsets1),offsets1.size())
      ;
      */
#endif
    }

    m_prepare_timer.stop();

    error = _solve(*matrix.internal(), *rhs.internal(), *sol.internal());
  } else {
    const MCGMatrix& matrix = A.impl()->get<BackEnd::tag::mcgsolver>();

    const MCGVector& rhs = b.impl()->get<BackEnd::tag::mcgsolver>();
    MCGVector& sol = x.impl()->get<BackEnd::tag::mcgsolver>(true);

    if (m_use_mpi) {
      ConstArrayView<int> offsets;
      UniqueArray<Integer> blockOffsets;
      int block_size;

      if (A.impl()->block()) {
        computeBlockOffsets(matrix.distribution(), *A.impl()->block(), blockOffsets);
        int blockSize = A.impl()->block()->size();
#ifdef ALIEN_USE_ARCANE
        offsets = blockOffsets.constView();
#else
        offsets = ConstArrayView<int>(blockOffsets);
#endif
        block_size = blockSize;
        part_info = new MCGSolver::PartitionInfo;
        part_info->init((int*)offsets.data(), offsets.size(), block_size);
      } else {
        Integer loffset = matrix.distribution().rowOffset();
        Integer nproc = m_parallel_mng->commSize();
        UniqueArray<Integer> scalarOffsets;
        scalarOffsets.resize(nproc + 1);

        mpAllGather(m_parallel_mng, ConstArrayView<int>(1, &loffset),
            ArrayView<int>(nproc, dataPtr(scalarOffsets)));

        scalarOffsets[nproc] = matrix.distribution().globalRowSize();
#ifdef ALIEN_USE_ARCANE
        offsets = scalarOffsets.constView();
#else
        offsets = ConstArrayView<int>(scalarOffsets);
#endif
        block_size = 1;
        part_info = new MCGSolver::PartitionInfo;
        part_info->init(offsets.data(), offsets.size(), block_size);
      }
    }

    m_prepare_timer.stop();
    error = _solve(*matrix.internal(), *rhs.internal(), *sol.internal(), part_info);

    delete part_info;
  }

  m_status.residual = m_mcg_status.m_residual;
  m_status.iteration_count = m_mcg_status.m_num_iter;
  m_solve_num += 1;
  m_total_iter_num += m_mcg_status.m_num_iter;

  m_int_total_setup_time += m_mcg_status.m_setup_time;
  m_int_total_finish_time += m_mcg_status.m_finish_time;
  m_int_total_solve_time += m_mcg_status.m_solve_time;
  m_int_total_allocate_time += m_mcg_status.m_allocate_time;
  m_int_total_init_time += m_mcg_status.m_init_time;
  m_int_total_update_time += m_mcg_status.m_update_time;

  if (error == 0) {
    m_status.succeeded = true;
    m_status.error = 0;
    if (m_output_level > 0) {
      printInfo();

      alien_info([&] {
        cout() << "Resolution info      :";
        cout() << "Resolution status    : OK";
        cout() << "Residual             : " << m_mcg_status.m_residual;
        cout() << "Number of iterations : " << m_mcg_status.m_num_iter;
      });
    }
    return true;
  } else {
    m_status.succeeded = false;
    m_status.error = m_mcg_status.m_error;
    if (m_output_level > 0) {
      printInfo();

      alien_info([&] {
        cout() << "Resolution status      : Error";
        cout() << "Error code             : " << m_mcg_status.m_error;
      });
    }
    return false;
  }
}

void
MCGInternalLinearSolver::setEdgeWeight(const IMatrix& E)
{
  const MCGMatrix& ew_matrix = E.impl()->get<BackEnd::tag::mcgsolver>();

  const auto* edge_weightp = ew_matrix.internal()->m_matrix[0][0]->getVal();
  const auto n_edge = ew_matrix.internal()->m_matrix[0][0]->getProfile().getNElems();

  m_edge_weight.resize(n_edge);
  std::copy(edge_weightp, edge_weightp + n_edge, m_edge_weight.begin());
}

ILinearSolver*
MCGInternalLinearSolverFactory(
    Arccore::MessagePassing::IMessagePassingMng* p_mng, IOptionsMCGSolver* options)
{
  return new MCGInternalLinearSolver(p_mng, options);
}
} // namespace Alien