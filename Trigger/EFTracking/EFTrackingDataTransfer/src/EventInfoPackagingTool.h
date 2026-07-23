/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef EFTRACKINGDATATRANSFER_EVENTINFOPACKAGINGTOOL_H
#define EFTRACKINGDATATRANSFER_EVENTINFOPACKAGINGTOOL_H

// Package includes
#include "IPackagingTool.h"

// Framework includes
#include "AthenaBaseComps/AthAlgTool.h"

// STL includes
#include <string>

/**
 * @class EventInfoPackagingTool is responsibel for filling basic event info
 * @brief
 **/
class EventInfoPackagingTool : public extends<AthAlgTool, IPackagingTool> {
public:
  EventInfoPackagingTool(const std::string& type, const std::string& name, const IInterface* parent);
  virtual ~EventInfoPackagingTool() override;

  virtual StatusCode pack(OffloadMessage& msg,
                          const EventContext& context) const;
  virtual StatusCode unpack(const OffloadMessage& msg,
                            EventContext& context) const;

  virtual StatusCode initialize() override { return StatusCode::SUCCESS; }
  virtual StatusCode finalize() override { return StatusCode::SUCCESS; }

private:
  //Gaudi::Property<int> m_myInt{this, "MyInt", 0, "An Integer"};
};

#endif // EFTRACKINGDATATRANSFER_EVENTINFOPACKAGINGTOOL_H
