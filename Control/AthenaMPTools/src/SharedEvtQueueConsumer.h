/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ATHENAMPTOOLS_SHAREDEVTQUEUECONSUMER_H
#define ATHENAMPTOOLS_SHAREDEVTQUEUECONSUMER_H

#include "AthenaMPToolBase.h"

#include "AthenaInterprocess/SharedQueue.h"
#include "GaudiKernel/Timing.h"
#include "GaudiKernel/IEvtSelector.h"

#include <memory>
#include <queue>

class IEventSeek;
class IEvtSelectorSeek;
class IEventShare;
class IDataShare;
class IChronoStatSvc;

class SharedEvtQueueConsumer final : public AthenaMPToolBase
{
 public:
  SharedEvtQueueConsumer(const std::string& type
			 , const std::string& name
			 , const IInterface* parent);

  virtual ~SharedEvtQueueConsumer() override;
  
  virtual StatusCode initialize() override;
  virtual StatusCode finalize() override;

  // _________IAthenaMPTool_________   
  virtual int makePool ATLAS_NOT_THREAD_SAFE (int maxevt, int nprocs, const std::string& topdir) override;
  virtual StatusCode exec ATLAS_NOT_THREAD_SAFE () override;
  virtual StatusCode wait_once ATLAS_NOT_THREAD_SAFE (pid_t& pid) override;

  virtual void reportSubprocessStatuses() override;
  virtual void subProcessLogs(std::vector<std::string>&) override;

  // _____ Actual working horses ________
  virtual std::unique_ptr<AthenaInterprocess::ScheduledWork> bootstrap_func() override;
  virtual std::unique_ptr<AthenaInterprocess::ScheduledWork> exec_func() override;
  virtual std::unique_ptr<AthenaInterprocess::ScheduledWork> fin_func() override;

 private:

  // Decode process results
  // 1. Store number of processed events for FUNC_EXEC
  // 2. If doFinalize flag is set then serialize process finalizations
  int decodeProcessResult ATLAS_NOT_THREAD_SAFE (const AthenaInterprocess::ProcessResult* presult, bool doFinalize);

  Gaudi::Property<bool> m_useSharedReader{this, "UseSharedReader", false, "Work in pair with a SharedReader"};
  Gaudi::Property<bool> m_useSharedWriter{this, "UseSharedWriter", false, "Work in pair with a SharedWriter"};
  Gaudi::Property<bool> m_isRoundRobin{this, "IsRoundRobin", false, "Are we running in the 'reproducible mode'?"};
  Gaudi::Property<bool> m_debug{this, "Debug", false};
  Gaudi::Property<bool> m_readEventOrders{this, "ReadEventOrders", false};
  Gaudi::Property<int> m_nEventsBeforeFork{this, "EventsBeforeFork", 0};
  Gaudi::Property<std::string> m_eventOrdersFile{this, "EventOrdersFile", "athenamp_eventorders.txt"};

  int  m_rankId{-1};          // Each worker has its own unique RankID from the range (0,...,m_nprocs-1)
  int  m_nSkipEvents{0};

  ServiceHandle<IChronoStatSvc>  m_chronoStatSvc;
  SmartIF<IEventSeek>            m_evtSeek;
  SmartIF<IEvtSelectorSeek>      m_evtSelSeek;
  IEvtSelector::Context*         m_evtContext{nullptr};
  SmartIF<IEventShare>           m_evtShare;
  SmartIF<IDataShare>            m_dataShare;

  AthenaInterprocess::SharedQueue*  m_sharedEventQueue{nullptr};
  std::unique_ptr<AthenaInterprocess::SharedQueue>  m_sharedRankQueue;

  typedef System::ProcessTime::TimeValueType TimeValType;
  std::map<pid_t,std::pair<int,TimeValType>> m_eventStat; // Number of processed events by PID
  std::queue<pid_t>                          m_finQueue;         // PIDs of processes queued for finalization

  // "Persistent" event orders for reproducibility
  std::vector<int>               m_eventOrders;
  pid_t                          m_masterPid;  // In finalize() of the master process merge workers' saved orders into one
};

#endif
