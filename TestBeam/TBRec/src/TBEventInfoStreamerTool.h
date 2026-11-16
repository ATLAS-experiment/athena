/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TBREC_TBEVENTINFOSTREAMERTOOL_H
#define TBREC_TBEVENTINFOSTREAMERTOOL_H

#include "TBEventStreamerTool.h"

#include <string>
#include <vector>

class TBEventInfoStreamerTool : public TBEventStreamerTool
{
 public: 
  TBEventInfoStreamerTool(const std::string& type,
			    const std::string& name,
			    const IInterface* parent);
  virtual ~TBEventInfoStreamerTool();

  virtual StatusCode initializeTool() override;

  virtual StatusCode accept() override;

 protected:

  ////////////////
  // Properties //
  ////////////////
 std::vector<int> m_acceptCodes;
 Gaudi::Property<std::vector<std::string>> m_acceptTypes{this, "AcceptedTypes", {"Physics"}, "accepted detector event types"};

 int m_selected_events{0};
};
#endif
