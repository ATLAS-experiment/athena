/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef EFTRACKINGDATATRANSFER_ASYNCGRPCOMPUTEALG_H
#define EFTRACKINGDATATRANSFER_ASYNCGRPCOMPUTEALG_H

// Framework includes
#include "AthenaBaseComps/AthAsynchronousAlgorithm.h"

// STL includes
#include <grpcpp/grpcpp.h>

#include <string>

#include "EFTrackingDataTransfer/Message.pb.h"
using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

#include "EFTrackingDataTransfer/Message.grpc.pb.h"
#include "IPackagingTool.h"

/**
 * @class AsyncgRPCComputeAlg
 * @brief
 **/
class AsyncgRPCComputeAlg : public AthAsynchronousAlgorithm {
 public:
  // Inherit the base class's constructor(s).
  using AthAsynchronousAlgorithm::AthAsynchronousAlgorithm;

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& context) const override;
  virtual StatusCode restoreAfterSuspend() const override;
  virtual StatusCode finalize() override;

 private:
  ToolHandleArray<IPackagingTool> m_packingTools{
      this,
      "PackagingTools",
      {},
      "Tools that fetch data from current context and encodes it into probuf "
      "for sending"};
  // there will be a tool to decode the data back
  mutable std::unique_ptr<UniversalOffloadService::Stub> m_stub;
};

#endif  // EFTRACKINGDATATRANSFER_ASYNCGRPCOMPUTEALG_H
