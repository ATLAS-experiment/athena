/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ATHENAKERNEL_IMPICLUSTERSVC_H
#define ATHENAKERNEL_IMPICLUSTERSVC_H

#include <cstdint>
#include <string>

#include "GaudiKernel/IInterface.h"

struct ClusterMessage;

namespace boost::mpi3 {
class communicator;
}

enum class ClusterComm { Default, EventData };

/// Interface for the MPIClusterSvc, which manages internode communications
/// in AthenaMPI
class IMPIClusterSvc : virtual public IInterface {
 public:
  /// InterfaceID
  DeclareInterfaceID(IMPIClusterSvc, 1, 0);

  /// Number of ranks
  virtual int numRanks() const = 0;

  /// MPI rank
  virtual int rank() const = 0;

  /// MPI Barrier
  virtual void barrier() = 0;

  /// MPI Abort -- For use if there's an error during initialization
  virtual void abort() = 0;

  /// Send a message
  virtual void sendMessage(int destRank, ClusterMessage message,
                           ClusterComm communicator = ClusterComm::Default) = 0;

  /// Wait to receive a message
  virtual ClusterMessage waitReceiveMessage(
      ClusterComm communicator = ClusterComm::Default) = 0;

  /// Provide the MPI3 data communicator
  virtual boost::mpi3::communicator& data_communicator() = 0;

  /// Run at start of event to add it to the log
  virtual void log_addEvent(int eventIdx, std::int64_t run_number,
                            std::int64_t event_number,
                            std::int64_t request_time_ns,
                            std::size_t slot) = 0;

  /// Run at end of event to complete it in the log
  virtual void log_completeEvent(std::int64_t run_number,
                                 std::int64_t event_number,
                                 std::int64_t status) = 0;
};

#endif  // ATHENAKERNEL_IMPICLUSTERSVC_H
