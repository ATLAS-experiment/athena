/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ATHENASERVICES_MPICLUSTERSVC_H_
#define ATHENASERVICES_MPICLUSTERSVC_H_

#include <memory>
#include <string>

#include "AthenaBaseComps/AthService.h"
#include "AthenaKernel/ClusterMessage.h"
#include "AthenaKernel/IMPIClusterSvc.h"
#include "SQLiteDBSvc/ISQLiteDBSvc.h"
#include "SQLiteDBSvc/Statement.h"
#include "mpi3/environment.hpp"

namespace mpi3 = boost::mpi3;

/** @class MPIClusterSvc
 * @brief A service managing communications within a cluster using MPI
 *
 */
class MPIClusterSvc : public extends<AthService, IMPIClusterSvc> {
 public:
  /// Constructor
  MPIClusterSvc(const std::string& name, ISvcLocator* svcLoc)
      : extends(name, svcLoc) {}

  /// Initialize
  virtual StatusCode initialize() override final;

  /// Finalize
  virtual StatusCode finalize() override final;

  /// Return number of ranks
  virtual int numRanks() const override final;

  /// Return our rank
  virtual int rank() const override final;

  /// Insert a barrier
  /// No rank will continue until all ranks reach this point
  virtual void barrier() override final;

  /// Abort the MPI run
  virtual void abort() override final;

  /// Send an MPI message
  virtual void sendMessage(
      int destRank, ClusterMessage message,
      ClusterComm communicator = ClusterComm::Default) override final;

  /// Block until we receive an MPI message
  virtual ClusterMessage waitReceiveMessage(
      ClusterComm communicator = ClusterComm::Default) override final;

  /// Return the data communicator
  virtual mpi3::communicator& data_communicator() override final {
    return m_datacom;
  }

  /// Add (begin) an event in the log
  virtual void log_addEvent(int eventIdx, std::int64_t run_number,
                            std::int64_t event_number,
                            std::int64_t request_time_ns) override final;
  /// Complete an event in the log
  virtual void log_completeEvent(std::int64_t run_number,
                                 std::int64_t event_number,
                                 std::int64_t status) override final;

 private:
  std::unique_ptr<mpi3::environment> m_env;
  mpi3::communicator m_world;
  // Communicator for payload of event data messages
  mpi3::communicator m_datacom;
  int m_rank = -1;

  // MPI Log DB
  ServiceHandle<ISQLiteDBSvc> m_mpiLog{this, "LogDatabaseSvc", "",
                                       "SQLiteDBSvc for the MPI event log"};
  SQLite::Statement m_mpiLog_addEvent;
  SQLite::Statement m_mpiLog_completeEvent;
};
#endif  // ATHENASERVICES_MPICLUSTERSVC_H_
