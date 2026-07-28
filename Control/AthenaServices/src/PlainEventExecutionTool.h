/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ATHENASERVICES_PLAINEVENTEXECUTIONTOOL_H
#define ATHENASERVICES_PLAINEVENTEXECUTIONTOOL_H

// Package includes
#include "AthenaKernel/IEventExecutionTool.h"

// Framework includes
#include "AthenaBaseComps/AthAlgTool.h"

// STL includes
#include <string>

/**
 * @class PlainEventExecutionTool
 * @brief This one just executes the event
 **/
class PlainEventExecutionTool : public extends<AthAlgTool, IEventExecutionTool> {
public:
  PlainEventExecutionTool(const std::string& type, const std::string& name, const IInterface* parent);
  virtual ~PlainEventExecutionTool() override {}

  virtual StatusCode executeEvent(MinimalEventLoopMgr* el,  EventContext&& ctx) {
    ATH_MSG_ALWAYS("Execution diverted to the tool");
    return el->executeEvent(std::move(ctx));
  }
  virtual StatusCode completeEvent(MinimalEventLoopMgr*,  const EventContext& ctx) override {
    return StatusCode::SUCCESS;
  }

  virtual StatusCode initialize() override { return StatusCode::SUCCESS; }
  virtual StatusCode finalize() override{ return StatusCode::SUCCESS; };

private:
  //Gaudi::Property<int> m_myInt{this, "MyInt", 0, "An Integer"};
};

#endif // ATHENASERVICES_PLAINEVENTEXECUTIONTOOL_H
