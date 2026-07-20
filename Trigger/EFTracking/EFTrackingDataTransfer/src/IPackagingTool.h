/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef EFTRACKINGDATATRANSFER_IPACKAGINGTOOL_H
#define EFTRACKINGDATATRANSFER_IPACKAGINGTOOL_H

#include "EFTrackingDataTransfer/Message.pb.h"
#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/IAlgTool.h"

/**
 * @class IPackagingTool
 * @brief tools responsible of packaging data into protobuf OffloadMessage
 *packets
 **/
class IPackagingTool : virtual public IAlgTool {
 public:
  DeclareInterfaceID(IPackagingTool, 1, 0);

  virtual ~IPackagingTool() override {}

  virtual StatusCode pack(OffloadMessage& msg,
                          const EventContext& context) const = 0;
  virtual StatusCode unpack(const OffloadMessage& msg,
                          const EventContext& context) const = 0;

  };

#endif  // EFTRACKINGDATATRANSFER_IPACKAGINGTOOL_H
