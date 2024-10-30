/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#pragma once

#include <list>
#include <vector>
#include <string>
#include <map>

#include "GaudiKernel/AlgTool.h"
#include "TrkSpacePoint/SpacePoint.h"

namespace InDet {

  /** 
   * @interface ISpacepointFeatureCalculationTool
   * @brief Calculate features for spacepoints to be used as inputs to the GNN-based track finder.
   * @author xiangyang.ju@cern.ch
   */
  class ISpacepointFeatureTool : virtual public IAlgTool
  {
    public:
    ///////////////////////////////////////////////////////////////////
    /// @name InterfaceID
    ///////////////////////////////////////////////////////////////////
    //@{
    DeclareInterfaceID(ISpacepointFeatureTool, 1, 0);
    //@}

    ///////////////////////////////////////////////////////////////////
    /// Main methods for calculating features for spacepoints
    ///////////////////////////////////////////////////////////////////
    virtual std::map<std::string, float> getFeatures(const Trk::SpacePoint*) const =0;
  };
}
