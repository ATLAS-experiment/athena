/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef AthenaKernel_IEVENTEXECUTIONTOOL_H
#define AthenaKernel_IEVENTEXECUTIONTOOL_H

#include "GaudiKernel/IAlgTool.h"
#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/MinimalEventLoopMgr.h"
/**
 * @class IEventExecutionTool
 * @brief Interace for tools that are responsible of invoking exceuteEvent
 * Relevant implementations will in fact do additional actions, like waiting for remote procedure call/MPI call etc
 * and then once the algorithsm in the event are executed the tool could send back the response
 **/

class MinimalEventLoopMgr;
class IEventExecutionTool : virtual public IAlgTool {
public: 
  DeclareInterfaceID(IEventExecutionTool, 1, 0);

  virtual StatusCode executeEvent(MinimalEventLoopMgr*,  EventContext&& ctx) = 0;
  virtual StatusCode completeEvent(MinimalEventLoopMgr*,  const EventContext& ctx) = 0;

  virtual ~IEventExecutionTool() override {}
}; 

#endif // AthenaKernel_IEVENTEXECUTIONTOOL_H
