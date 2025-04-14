/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ATHENAMPTOOLS_SHAREDEVTQUEUEPROVIDER_H
#define ATHENAMPTOOLS_SHAREDEVTQUEUEPROVIDER_H

#include "AthenaMPToolBase.h"
#include "GaudiKernel/IIncidentListener.h"
#include "AthenaInterprocess/SharedQueue.h"

class IEventShare;

class SharedEvtQueueProvider final : public extends<AthenaMPToolBase, IIncidentListener>
{
 public:
  SharedEvtQueueProvider(const std::string& type
			 , const std::string& name
			 , const IInterface* parent);

  virtual ~SharedEvtQueueProvider() override;

  // _________IAthenaMPTool_________
  virtual int makePool ATLAS_NOT_THREAD_SAFE (int maxevt, int nprocs, const std::string& topdir) override;
  virtual StatusCode exec ATLAS_NOT_THREAD_SAFE() override;

  virtual void subProcessLogs(std::vector<std::string>&) override;
  virtual AthenaMP::AllWorkerOutputs_ptr generateOutputReport() override;

  // _________IIncidentListener___________
  virtual void handle(const Incident& inc) override;

  // _____ Actual working horses ________
  virtual std::unique_ptr<AthenaInterprocess::ScheduledWork> bootstrap_func() override;
  virtual std::unique_ptr<AthenaInterprocess::ScheduledWork> exec_func() override;
  virtual std::unique_ptr<AthenaInterprocess::ScheduledWork> fin_func() override;

 private:
  SharedEvtQueueProvider();
  SharedEvtQueueProvider(const SharedEvtQueueProvider&);
  SharedEvtQueueProvider& operator= (const SharedEvtQueueProvider&);

  Gaudi::Property<bool> m_useSharedReader{this, "UseSharedReader", false, "Use shared reader"};
  Gaudi::Property<int>  m_nEventsBeforeFork{this, "EventsBeforeFork", 0, "Number of events before forking"};
  Gaudi::Property<int>  m_nChunkSize{this, "ChunkSize", 1};

  /**
   * We use this data member for adding negative numbers at the end of the event queue.
   * We cannot use m_nprocs for this purpose in order to avoid generating Output File
   * Reports by Shared Queue Providers.
   */
  int  m_nprocesses{-1};
  int  m_nChunkStart{0};      ///< The beginning of the current chunk
  int  m_nPositionInChunk{0}; ///< Position within the current chunk

  int  m_nEvtRequested{-1};    ///< Max event received from AppMgr
  int  m_nEvtCounted{0};       ///< The number of events this tool has counted itself in the input files
  
  AthenaInterprocess::SharedQueue*  m_sharedEventQueue{nullptr};
  SmartIF<IEventShare>              m_evtShare;

  // Add next event chunk to the queue
  void addEventsToQueue(); 

};

#endif
