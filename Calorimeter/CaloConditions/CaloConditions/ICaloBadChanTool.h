/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ICaloBadChanTool_H
#define ICaloBadChanTool_H

#include "GaudiKernel/IAlgTool.h"
#include "GaudiKernel/EventContext.h"

#include "Identifier/Identifier.h"
#include "CaloConditions/CaloBadChannel.h"


class ICaloBadChanTool : public virtual IAlgTool {
public:
  DeclareInterfaceID(ICaloBadChanTool, 1, 0);

  virtual ~ICaloBadChanTool() {}

  virtual CaloBadChannel caloStatus(const EventContext& ctx,
                                    Identifier id) const = 0;

};

#endif
