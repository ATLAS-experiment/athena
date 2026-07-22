/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef EFTRACKINGDATATRANSFER_EXECUTEONGRPCCALL_H
#define EFTRACKINGDATATRANSFER_EXECUTEONGRPCCALL_H

#include "AthenaKernel/IEventExecutionTool.h"

// Framework includes
#include "AthenaBaseComps/AthAlgTool.h"
// STL includes
#include <memory>
#include <string>


/**
 * @class ExecuteOngRPCCall tool that waits for gRPC call before invoking
 *executeEvent
 **/
class ExecuteOngRPCCall : public extends<AthAlgTool, IEventExecutionTool> {
 public:
  ExecuteOngRPCCall(const std::string& type, const std::string& name,
                    const IInterface* parent);

  virtual StatusCode executeEvent(MinimalEventLoopMgr* el,
                                  EventContext&& ctx) override;

  virtual StatusCode initialize() override;
  virtual StatusCode finalize() override;
 private:
  Gaudi::Property<std::string> m_address{
      this, "address", "0.0.0.0:50051",
      "IP:PORT address on which this tool will receive the data"};
};

#endif  // EFTRACKINGDATATRANSFER_EXECUTEONGRPCCALL_H
