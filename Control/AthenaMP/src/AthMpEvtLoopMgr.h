/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ATHENAMP_ATHMPEVTLOOPMGR_H
#define ATHENAMP_ATHMPEVTLOOPMGR_H

#include "GaudiKernel/IEventProcessor.h"
#include "AthenaBaseComps/AthService.h"
#include "GaudiKernel/ToolHandle.h"
#include "AthenaKernel/IDataShare.h"
#include "AthenaMPTools/IAthenaMPTool.h"
#include "AthenaInterprocess/FdsRegistry.h"
#include "AthenaInterprocess/IMPRunStop.h"
#include <memory>

class ISvcLocator;

class ATLAS_NOT_THREAD_SAFE AthMpEvtLoopMgr : public extends<AthService,
                                                             IEventProcessor,
                                                             AthenaInterprocess::IMPRunStop>
{
 public:
  AthMpEvtLoopMgr(const std::string& name, ISvcLocator* svcLocator);
  virtual ~AthMpEvtLoopMgr() = default;

  AthMpEvtLoopMgr() = delete;
  AthMpEvtLoopMgr(const AthMpEvtLoopMgr&) = delete;
  AthMpEvtLoopMgr& operator = (const AthMpEvtLoopMgr&) = delete;


  virtual StatusCode initialize() override;
  virtual StatusCode finalize() override;

  virtual StatusCode nextEvent(int maxevt) override;
  virtual StatusCode executeEvent(EventContext &&ctx) override;
  virtual StatusCode executeRun(int maxevt) override;
  virtual StatusCode stopRun() override;

  virtual EventContext createEventContext() override;

  virtual bool stopScheduled() const override {return m_scheduledStop;};

 private:
  ServiceHandle<IEventProcessor> m_evtProcessor{this,"EventLoopManager","AthenaEventLoopMgr"};
  SmartIF<IService>              m_evtSelector{nullptr};
  SmartIF<IDataShare>            m_dataShare;

  Gaudi::Property<int> m_nWorkers{this, "NWorkers", 0,
      "Number of AthenaMP worker processes"};

  Gaudi::Property<std::string> m_workerTopDir{this, "WorkerTopDir", "athenaMP_workers",
      "Sub-directory of the main run directory that contains run directories of all workers"};

  Gaudi::Property<std::string> m_outputReportName{this, "OutputReportFile", "AthenaMPOutputs",
      "ASCII file in the main run directory that lists outputs of all workers. Used by Job Transform"};

  Gaudi::Property<std::string> m_strategy{this, "Strategy", "",
      "Event processing strategy used by AthenaMP workers. E.g, Shared Queue, Round Robin"};

  Gaudi::Property<bool> m_isPileup{this, "IsPileup", false,
      "Is AthenaMP running a PileUp Digitization job?"};

  Gaudi::Property<bool> m_collectSubprocessLogs{this, "CollectSubprocessLogs", false,
      "Copy all workers' logs into the main log file at the end of the job?"};

  ToolHandleArray<IAthenaMPTool> m_tools{this,"Tools", {}};

  Gaudi::Property<int> m_nPollingInterval{this, "PollingInterval", 100,
      "Interval in milliseconds between checks of sub-processes statuses"};

  Gaudi::Property<int> m_nMemSamplingInterval{this, "MemSamplingInterval", 0,
      "Interval in seconds between taking memory usage samples. 0 - no sampling"};

  Gaudi::Property<int> m_nEventsBeforeFork{this, "EventsBeforeFork", 0,
      "Number of events to be processed by the main process before forking the workers. 0 - fork after BeginRun incident"};

  Gaudi::Property<unsigned int> m_eventPrintoutInterval{this, "EventPrintoutInterval", 1,
      "The value to be forwarded to the EventPrintoutInterval property of the AthenaEventLoopMgr"};

  StringArrayProperty m_execAtPreFork{this, "ExecAtPreFork", {},
      "The value to be forwarded to the ExecAtPreFork property of the AthenaEventLoopMgr"};

  int    m_nChildProcesses{0};    // Total number of child processes
  pid_t  m_masterPid{};           // PID of the main process
  bool   m_scheduledStop{false};  // Flag for early termination of the event loop (for the generators use-case)

  // vectors for collecting memory samples
  std::vector<unsigned long>     m_samplesRss;
  std::vector<unsigned long>     m_samplesPss;
  std::vector<unsigned long>     m_samplesSize;
  std::vector<unsigned long>     m_samplesSwap;
  
  StatusCode wait();
  StatusCode generateOutputReport(); 
  std::shared_ptr<AthenaInterprocess::FdsRegistry> extractFds();
  StatusCode updateSkipEvents(int skipEvents);
}; 

#endif
