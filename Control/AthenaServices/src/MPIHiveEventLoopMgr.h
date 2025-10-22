// -*- C++ -*-

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ATHENASERVICES_MPIHIVEEVENTLOOPMGR_H
#define ATHENASERVICES_MPIHIVEEVENTLOOPMGR_H
/** @file MPIHiveEventLoopMgr.h
    @brief The MPI event loop manager.

*/

// Base class headers
#include "AthenaHiveEventLoopMgr.h"

// Athena headers
#include "AthenaKernel/IMPIClusterSvc.h"
#include "EventInfo/EventID.h" /* number_type */

// Gaudi headers
#include "Gaudi/Property.h"

// Standard includes
#include <string>

/** @class MPIHiveEventLoopMgr
    @brief The MPI event loop manager.

    @details As with AthenaHiveEventLoopMgr but in a multi-node MPI environment.
    This class is derived from and implemented in terms of
   AthenaHiveEventLoopMgr.
*/
class MPIHiveEventLoopMgr : public AthenaHiveEventLoopMgr {
  // for Hive
 protected:
  /// Reference to the MPIClusterSvc
  ServiceHandle<IMPIClusterSvc> m_clusterSvc{this, "MPIClusterSvc", "",
                                             "MPIClusterSvc"};

  /// Drain the local scheduler of any (at least one) completed events
  StatusCode drainLocalScheduler();
  /// Insert an event into the local scheduler
  StatusCode insertEvent(int eventIdx, bool& endOfStream,
                         std::int64_t requestTime_ns);
  /// Worker event loop (runs on worker, requests events over MPI)
  StatusCode workerEventLoop();
  /// Master event loop (runs on master, provides events over MPI)
  StatusCode masterEventLoop(int maxEvt);

  // Keep track of how many failed events we've had
  int m_contiguousFailedEvts{0};
  int m_totalFailedEvts{0};
  // Keeps track of events already processed
  int m_nLocalCreatedEvts{0};
  int m_nLocalSkippedEvts{0};
  int m_nLocalFinishedEvts{0};

 public:
  /// Standard Constructor
  MPIHiveEventLoopMgr(const std::string& name, ISvcLocator* svcLoc);
  /// Standard Destructor
  virtual ~MPIHiveEventLoopMgr();
  /// implementation of IAppMgrUI::initalize
  virtual StatusCode initialize() override;
  /// implementation of IAppMgrUI::finalize
  virtual StatusCode finalize() override;
  /// implementation of IAppMgrUI::nextEvent. maxevt==0 returns immediately
  virtual StatusCode nextEvent(int maxevt) override;

 private:
  /// @property First event
  UnsignedIntegerProperty m_firstEventIndex{
      this, "FirstEventIndex", 0, "First event index (Exec.SkipEvents)"};
  int m_evtSelectorCurrentPos = 0;

  StoreGateSvc* eventStore() const;
};

#endif  // ATHENASERVICES_MPIHIVEEVENTLOOPMGR_H
