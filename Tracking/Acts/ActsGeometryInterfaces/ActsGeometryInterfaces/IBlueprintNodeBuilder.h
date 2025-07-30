/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSGEOMETRYINTERFACES_IACTSBLUEPRINTNODEBUILDER_H
#define ACTSGEOMETRYINTERFACES_IACTSBLUEPRINTNODEBUILDER_H


#include "AthenaBaseComps/AthAlgTool.h"


#include "Acts/Geometry/BlueprintNode.hpp"
#include "Acts/Geometry/GeometryIdentifier.hpp"

namespace ActsTrk {

  /** Interface for the Blueprint node builder
    *  This interface is used to build a Blueprint node for the Acts tracking geometry.
    *  The derived classes corresponds to the different subsystems (e.g Itk, Calo, Muon).
    *  A Blueprint node can be passed in the build function and added as a child of another node 
    * (e.g. Calo or Itk as child Node of Muon System).
 */

class IBlueprintNodeBuilder : virtual public IAlgTool {

    public:

    DeclareInterfaceID(IBlueprintNodeBuilder, 1, 0);


    virtual std::shared_ptr<Acts::Experimental::BlueprintNode> buildBlueprintNode(const Acts::GeometryContext& gctx, 
                                                                  std::shared_ptr<Acts::Experimental::BlueprintNode>&& childNode) = 0;

  
};

}

#endif