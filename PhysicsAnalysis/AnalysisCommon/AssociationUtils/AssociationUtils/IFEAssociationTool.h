/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ASSOCIATIONUTILS_IFEASSOCIATIONTOOL_H
#define ASSOCIATIONUTILS_IFEASSOCIATIONTOOL_H

#include "AsgTools/IAsgTool.h"
#include "ColumnarMode/ColumnarMode.h"

class EventContext;

namespace ADD_CMODE(ORUtils) {

class IFEAssociationTool : public virtual asg::IAsgTool {
  ASG_TOOL_INTERFACE(IFEAssociationTool)

public:
#ifndef XAOD_STANDALONE
  virtual StatusCode buildAssociations(const EventContext& ctx) const = 0;
#else
  virtual StatusCode buildAssociations() = 0;
#endif
};

} // namespace ADD_CMODE(ORUtils)

#endif