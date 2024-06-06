/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGHIHYPO_ITRIGHIEVENTSHAPEHYPOTOOL_H
#define TRIGHIHYPO_ITRIGHIEVENTSHAPEHYPOTOOL_H

#include "GaudiKernel/IAlgTool.h"
#include "TrigCompositeUtils/TrigCompositeUtils.h"
#include "TrigCompositeUtils/HLTIdentifier.h"
#include "xAODHIEvent/HIEventShapeContainer.h"

class ITrigHIEventShapeHypoTool : virtual public::IAlgTool {

  public:

    DeclareInterfaceID(ITrigHIEventShapeHypoTool, 1, 0);

    virtual ~ITrigHIEventShapeHypoTool() {};

    virtual StatusCode decide(const xAOD::HIEventShapeContainer*, bool&) const = 0;
    virtual const HLT::Identifier& getId() const = 0;

};

#endif

