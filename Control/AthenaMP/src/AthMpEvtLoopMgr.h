/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ATHENAMP_ATHMPEVTLOOPMGR_H
#define ATHENAMP_ATHMPEVTLOOPMGR_H

#include "GaudiKernel/IEventProcessor.h"
#include "AthenaBaseComps/AthService.h"
#include "GaudiKernel/ToolHandle.h"
#include "AthenaInterprocess/FdsRegistry.h"
#include "AthenaInterprocess/IMPRunStop.h"
#include <memory>

class IAthenaMPTool;
class ISvcLocator;

class ATLAS_NOT_THREAD_SAFE AthMpEvtLoopMgr : public extends<AthService,
                                                             IEventProcessor,
                                                             AthenaInterprocess::IMPRunStop>
{
 public:
  AthMpEvtLoopMgr(const std::string& name, ISvcLocator* svcLocator);
  virtual ~AthMpEvtLoopMgr();

  virtual StatusCode initialize() override;
  virtual StatusCode finalize() override;

  virtual StatusCode nextEvent(int maxevt) override;
  virtual StatusCode executeEvent(EventContext &&ctx) override;
  virtual StatusCode executeRun(int maxevt) override;
  virtual StatusCode stopRun() override;

  virtual EventContext createEventContext() override;

  virtual bool stopScheduled() const override {return m_scheduledStop;};

 private:
  ServiceHandle<IEventProcessor> m_evtProcessor;
  SmartIF<IService>              m_evtSelector;
  int                            m_nWorkers;
  std::string                    m_workerTopDir;
  std::string                    m_outputReportName;
  std::string                    m_strategy;
  bool                           m_isPileup;
  bool                           m_collectSubprocessLogs;
  ToolHandleArray<IAthenaMPTool> m_tools;
  int                            m_nChildProcesses;
  int                            m_nPollingInterval;      // in milliseconds
  int                            m_nMemSamplingInterval;  // in seconds
  int                            m_nEventsBeforeFork;
  unsigned int                   m_eventPrintoutInterval;
  StringArrayProperty            m_execAtPreFork;
  pid_t                          m_masterPid;
  bool                           m_scheduledStop{false};

  // vectors for collecting memory samples
  std::vector<unsigned long>     m_samplesRss;
  std::vector<unsigned long>     m_samplesPss;
  std::vector<unsigned long>     m_samplesSize;
  std::vector<unsigned long>     m_samplesSwap;
  
  AthMpEvtLoopMgr();
  AthMpEvtLoopMgr(const AthMpEvtLoopMgr&);
  AthMpEvtLoopMgr& operator = (const AthMpEvtLoopMgr&);

  StatusCode wait();
  StatusCode generateOutputReport(); 
  std::shared_ptr<AthenaInterprocess::FdsRegistry> extractFds();
  StatusCode updateSkipEvents(int skipEvents);
}; 

#endif
