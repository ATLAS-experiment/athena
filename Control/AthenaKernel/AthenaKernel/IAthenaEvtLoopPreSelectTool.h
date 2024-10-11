/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ATHENAKERNEL_IATHENAEVTLOOPPRESELECTTOOL_H
#define ATHENAKERNEL_IATHENAEVTLOOPPRESELECTTOOL_H

/** @file IAthenaEvtLoopPreSelectTool.h
 *  @brief This file contains the class definition for the IAthenaEvtLoopPreSelectTool class.
 *  @author Marjorie Shapiro <mdshapiro@lbl.gov>
 **/

// Gaudi
#include "GaudiKernel/IAlgTool.h"

/** @class IAthenaEvtLoopPreSelectTool 
 *  @brief This class provides the interface for AthenaEvtLoopPreSelectTool classes used by AthenaEventLoopMgr
 **/

class EventIDBase;

class IAthenaEvtLoopPreSelectTool : virtual public extend_interfaces<IAlgTool> {

public:    

  DeclareInterfaceID(IAthenaEvtLoopPreSelectTool, 1, 0 );

  /// called for each event to decide if the event should be passed to the EventSelector
  virtual bool passEvent(const EventIDBase& pEvent) = 0;
};

#endif
