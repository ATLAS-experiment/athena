/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#pragma once

// System include(s).
#include <list>
#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <map>

#include "AthenaBaseComps/AthAlgTool.h"
#include "ISpacepointFeatureTool.h"
#include "InDetIdentifier/PixelID.h"
#include "InDetIdentifier/SCT_ID.h"
#include "InDetPrepRawData/SiCluster.h"


namespace InDet{
  /**
   * @class InDet::SpacepointFeatureTool
   * @brief InDet::SpacepointFeatureTool is a tool that calculates spacepoint features for GNNTrackFinder.
   * @author xiangyang.ju@cern.ch
   */

  class SpacepointFeatureTool: public extends<AthAlgTool, ISpacepointFeatureTool>
  {
    public:
    SpacepointFeatureTool(const std::string& type, const std::string& name, const IInterface* parent);

    StatusCode initialize() override final;

    virtual std::map<std::string, float> getFeatures(const Trk::SpacePoint*) const override final;

    protected:

    SpacepointFeatureTool() = delete;
    SpacepointFeatureTool(const SpacepointFeatureTool&) =delete;
    SpacepointFeatureTool &operator=(const SpacepointFeatureTool&) = delete;

    const PixelID *m_pixelID{};
    const SCT_ID *m_SCT_ID{};

    private:
    void cartesion_to_spherical(const Amg::Vector3D &xyzVec, float &eta_, float &phi_) const;
    StatusCode getSCTClusterShapeInfo_fn(const InDet::SiCluster *si_cluster, 
        float &charge_count, int &pixel_count, float &loc_eta, float &loc_phi, 
        float &glob_eta, float &glob_phi, 
        float &localDir0, float &localDir1, float &localDir2,
        float &lengthDir0, float &lengthDir1, float &lengthDir2,
        float &eta_angle, float &phi_angle
        ) const;
  };

}
