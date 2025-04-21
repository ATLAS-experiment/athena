/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "SharedWriterTool.h"
#include "AthenaInterprocess/ProcessGroup.h"

#include "AthenaKernel/IEventShare.h"
#include "AthenaKernel/IDataShare.h"
#include "AthenaKernel/IAthenaSharedWriterSvc.h"
#include "CxxUtils/xmalloc.h"
#include "GaudiKernel/IEvtSelector.h"
#include "GaudiKernel/IConversionSvc.h"
#include "GaudiKernel/IIoComponentMgr.h"

#include "AthenaBaseComps/AthCnvSvc.h"
#include <filesystem>

SharedWriterTool::SharedWriterTool(const std::string& type
				   , const std::string& name
				   , const IInterface* parent)
  : AthenaMPToolBase(type,name,parent)
  , m_rankId(0)
  , m_sharedRankQueue(nullptr)
{
  m_subprocDirPrefix = "shared_writer";
}

SharedWriterTool::~SharedWriterTool()
{
}

StatusCode SharedWriterTool::initialize()
{
  ATH_MSG_DEBUG("In initialize");

  ATH_CHECK(AthenaMPToolBase::initialize());
  m_cnvSvc = serviceLocator()->service("AthenaPoolCnvSvc");
  ATH_CHECK(m_cnvSvc.isValid());

  return StatusCode::SUCCESS;
}

StatusCode SharedWriterTool::finalize()
{
  ATH_MSG_DEBUG("In finalize");

  delete m_sharedRankQueue;
  return StatusCode::SUCCESS;
}

int SharedWriterTool::makePool(int /*maxevt*/, int nprocs, const std::string& topdir)
{
  ATH_MSG_DEBUG("In makePool " << getpid());

  if(topdir.empty()) {
    ATH_MSG_ERROR("Empty name for the top directory!");
    return -1;
  }

  m_nprocs = (nprocs==-1?sysconf(_SC_NPROCESSORS_ONLN):nprocs) + 1;
  m_subprocTopDir = topdir;

  SmartIF<IProperty> propertyServer(m_cnvSvc);
  if(!propertyServer) {
    ATH_MSG_ERROR("Unable to cast conversion service to IProperty");
    return -1;
  }
  else {
    std::string propertyName = "ParallelCompression";
    bool parallelCompression(false);
    BooleanProperty parallelCompressionProp(propertyName,parallelCompression);
    if(propertyServer->getProperty(&parallelCompressionProp).isFailure()) {
      ATH_MSG_INFO("Conversion service does not have ParallelCompression property");
    }
    else {
      SmartIF<IProperty> poolSvc(serviceLocator()->service("PoolSvc"));
      if(!poolSvc) {
        ATH_MSG_ERROR("Error retrieving PoolSvc");
      }
      else if(parallelCompressionProp.value()) {
        if (poolSvc->setProperty("FileOpen", "update").isFailure()) {
          ATH_MSG_ERROR("Could not change PoolSvc FileOpen Property");
        }
      }
    }
  }

  // Create rank queue and fill it
  m_sharedRankQueue = new AthenaInterprocess::SharedQueue("SharedWriterTool_RankQueue_"+m_randStr,1,sizeof(int));
  if(!m_sharedRankQueue->send_basic<int>(0)) {
    ATH_MSG_ERROR("Unable to send int to the ranks queue!");
    return -1;
  }

  // Create the process group and map_async bootstrap
  m_processGroup = new AthenaInterprocess::ProcessGroup(1);
  ATH_MSG_INFO("Created shared writer process");
  if(mapAsyncFlag(AthenaMPToolBase::FUNC_BOOTSTRAP))
    return -1;
  ATH_MSG_INFO("Shared writer process bootstrapped");
  return 1;
}

StatusCode SharedWriterTool::exec()
{
  ATH_MSG_DEBUG("In exec " << getpid());

  if(mapAsyncFlag(AthenaMPToolBase::FUNC_EXEC))
    return StatusCode::FAILURE;
  ATH_MSG_INFO("Shared writer started write events");

  // Set exit flag on writer
  if(m_processGroup->map_async(0,0)){
    ATH_MSG_ERROR("Unable to set exit to the writer");
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}

void SharedWriterTool::subProcessLogs(std::vector<std::string>& filenames)
{
  filenames.clear();
  std::filesystem::path writer_rundir(m_subprocTopDir);
  writer_rundir/= std::filesystem::path(m_subprocDirPrefix);
  filenames.push_back(writer_rundir.string()+std::string("/AthenaMP.log"));
}

AthenaMP::AllWorkerOutputs_ptr SharedWriterTool::generateOutputReport()
{
  AthenaMP::AllWorkerOutputs_ptr jobOutputs(new AthenaMP::AllWorkerOutputs());
  return jobOutputs;
}

std::unique_ptr<AthenaInterprocess::ScheduledWork> SharedWriterTool::bootstrap_func()
{
  // It's possible to debug SharedWriter just like any other AthenaMP worker.
  // The following procedure provides a minimal explanation on how this can be achieved:
  //
  // Terminal #1:
  // * Run athena w/ debugging enabled, e.g. athena.py --debugWorker --stdcmalloc --nprocs=8 [...]
  // * In this mode, workers will be stopped after fork(), waiting for SIGUSR1 to be resumed
  // * Find the PID of the worker to be debugged (printed by the job in stdout)
  //
  // Terminal #2:
  // * Attach gdb to the relevant worker, i.e. gdb python PID
  // * Once the symbols are loaded, one can perform any gdb action such as setting breakpoints etc.
  // * Once ready, send SIGUSR1 to the worker to resume work, i.e. signal SIGUSR1 (in gdb)
  //
  // Terminal #3:
  // * Send SIGUSR1 to the remaining workers (easiest to use htop)
  //
  // However, note that sometimes Shared I/O infrastructure struggles with timing problems,
  // such as server/client(s) starting/stopping too early/later. Debugging can change this
  // behavior so please keep this in mind.
  if(m_debug) waitForSignal();

  std::unique_ptr<AthenaInterprocess::ScheduledWork> outwork(new AthenaInterprocess::ScheduledWork);
  outwork->data = CxxUtils::xmalloc(sizeof(int));
  *(int*)(outwork->data) = 1; // Error code: for now use 0 success, 1 failure
  outwork->size = sizeof(int);

  // ...
  // (possible) TODO: extend outwork with some error message, which will be eventually
  // reported in the master proces
  // ...

  // ________________________ Get RankID ________________________
  //
  if(!m_sharedRankQueue->receive_basic<int>(m_rankId)) {
    ATH_MSG_ERROR("Unable to get rank ID!");
    return outwork;
  }
  // Writer dir: mkdir
  std::filesystem::path writer_rundir(m_subprocTopDir);
  writer_rundir /= std::filesystem::path(m_subprocDirPrefix);

  if(mkdir(writer_rundir.string().c_str(),S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH)==-1) {
    ATH_MSG_ERROR("Unable to make writer run directory: " << writer_rundir.string() << ". " << fmterror(errno));
    return outwork;
  }

  // __________ Redirect logs unless we want to attach debugger ____________
  if(!m_debug) {
    if(redirectLog(writer_rundir.string()))
      return outwork;

    ATH_MSG_INFO("Logs redirected in the AthenaMP Shared Writer PID=" << getpid());
  }

  // Update Io Registry
  if(updateIoReg(writer_rundir.string()))
    return outwork;

  ATH_MSG_INFO("Io registry updated in the AthenaMP Shared Writer PID=" << getpid());

  // _______________________ Handle saved PFC (if any) ______________________
  std::filesystem::path abs_writer_rundir = std::filesystem::absolute(writer_rundir);
  if(handleSavedPfc(abs_writer_rundir))
    return outwork;

  // Reopen file descriptors
  if(reopenFds())
    return outwork;

  ATH_MSG_INFO("File descriptors re-opened in the AthenaMP Shared Writer PID=" << getpid());

  // Try to initialize AthenaRootSharedWriterSvc early on
  SmartIF<IAthenaSharedWriterSvc> sharedWriterSvc(serviceLocator()->service("AthenaRootSharedWriterSvc"));
  if(!sharedWriterSvc) {
    ATH_MSG_WARNING("Error retrieving AthenaRootSharedWriterSvc from SharedWriterTool::bootstrap_func()");
  }

  // Use IDataShare to make ConversionSvc a Share Server
  SmartIF<IDataShare> cnvSvc(m_cnvSvc);
  if (!cnvSvc || !cnvSvc->makeServer(-m_nprocs - 1 - 1024 * m_rankId).isSuccess()) {
    ATH_MSG_ERROR("Failed to make the conversion service a share server");
    return outwork;
  }
  else {
    ATH_MSG_DEBUG("Successfully made the conversion service a share server");
  }

  // ________________________ I/O reinit ________________________
  if(!m_ioMgr->io_reinitialize().isSuccess()) {
    ATH_MSG_ERROR("Failed to reinitialize I/O");
    return outwork;
  } else {
    ATH_MSG_DEBUG("Successfully reinitialized I/O");
  }

  // Writer dir: chdir
  if(chdir(writer_rundir.string().c_str())==-1) {
    ATH_MSG_ERROR("Failed to chdir to " << writer_rundir.string());
    return outwork;
  }

  // Declare success and return
  *(int*)(outwork->data) = 0;
  return outwork;
}

std::unique_ptr<AthenaInterprocess::ScheduledWork> SharedWriterTool::exec_func()
{
  ATH_MSG_INFO("Exec function in the AthenaMP Shared Writer PID=" << getpid());
  bool all_ok=true;

  SmartIF<IAthenaSharedWriterSvc> sharedWriterSvc(serviceLocator()->service("AthenaRootSharedWriterSvc"));
  if(!sharedWriterSvc) {
    ATH_MSG_ERROR("Error retrieving AthenaRootSharedWriterSvc");
    all_ok=false;
  } else if(!sharedWriterSvc->share(m_nprocs, m_nMotherProcess.value()).isSuccess()) {
    ATH_MSG_ERROR("Exec function could not share data");
    all_ok=false;
  }
  AthCnvSvc* cnvSvc = dynamic_cast<AthCnvSvc*>(m_cnvSvc.get());
  if (cnvSvc == 0 || !cnvSvc->disconnectOutput("").isSuccess()) {
    ATH_MSG_ERROR("Exec function could not disconnectOutput");
    all_ok=false;
  }

  if(m_appMgr->stop().isFailure()) {
    ATH_MSG_ERROR("Unable to stop AppMgr");
    all_ok=false;
  }
  else {
    if(m_appMgr->finalize().isFailure()) {
      std::cerr << "Unable to finalize AppMgr" << std::endl;
      all_ok=false;
    }
  }

  std::unique_ptr<AthenaInterprocess::ScheduledWork> outwork(new AthenaInterprocess::ScheduledWork);
  outwork->data = CxxUtils::xmalloc(sizeof(int));
  *(int*)(outwork->data) = (all_ok?0:1); // Error code: for now use 0 success, 1 failure
  outwork->size = sizeof(int);

  // ...
  // (possible) TODO: extend outwork with some error message, which will be eventually
  // reported in the master proces
  // ...
  return outwork;
}

std::unique_ptr<AthenaInterprocess::ScheduledWork> SharedWriterTool::fin_func()
{
  // Dummy
  std::unique_ptr<AthenaInterprocess::ScheduledWork> outwork(new AthenaInterprocess::ScheduledWork);
  outwork->data = CxxUtils::xmalloc(sizeof(int));
  *(int*)(outwork->data) = 0; // Error code: for now use 0 success, 1 failure
  outwork->size = sizeof(int);
  return outwork;
}
