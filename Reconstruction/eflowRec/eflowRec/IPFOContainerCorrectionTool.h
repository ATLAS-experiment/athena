/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @class IPFOContainerCorrectionTool
 * @brief This is an interface for tools that apply corrections to xAOD::FlowElementContainer objects.
 * The tools implementing this interface should provide the logic to correct the energy or other PFO object properties
 */

#ifndef EFLOWREC_IPFOCONTAINERCORRECTIONTOOL_H
#define EFLOWREC_IPFOCONTAINERCORRECTIONTOOL_H

#include "xAODPFlow/FlowElementContainer.h"
#include "GaudiKernel/IAlgTool.h"

class IPFOContainerCorrectionTool : public virtual IAlgTool
{
public:
    virtual ~IPFOContainerCorrectionTool() = default;
    DeclareInterfaceID(IPFOContainerCorrectionTool, 1, 0);

    virtual void correctContainer(xAOD::FlowElementContainer& pfos) const = 0;
};
#endif // EFLOWREC_IPFOCONTAINERCORRECTIONTOOL_H