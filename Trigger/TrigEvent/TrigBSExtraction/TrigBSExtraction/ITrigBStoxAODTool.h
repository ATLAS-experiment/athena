/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGBSEXTRACTION_ITRIGBSTOXAODTOOL_H
#define TRIGBSEXTRACTION_ITRIGBSTOXAODTOOL_H
#include "GaudiKernel/IAlgTool.h"
#include "GaudiKernel/ToolHandle.h"

//fwd declaration
class EventContext;
namespace HLT{class Navigation;}

/**
 * @brief Interface of Tool used by TrigBSExtraction to convert to xAOD
 */
class ITrigBStoxAODTool : public virtual IAlgTool {
public:
  DeclareInterfaceID(ITrigBStoxAODTool, 1, 0);

  virtual StatusCode convert(const EventContext& ctx, HLT::Navigation*) = 0;
  virtual StatusCode rewireNavigation(HLT::Navigation*) = 0;
  virtual StatusCode setTrigPassBits(HLT::Navigation*) = 0;
};

#endif // TRIGBSEXTRACTION_ITRIGBSTOXAODTOOL_H
