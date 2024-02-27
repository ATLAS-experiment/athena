/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRKANALYSIS_SPACEPOINTANALYSISALG_H
#define ACTSTRKANALYSIS_SPACEPOINTANALYSISALG_H

#include "AthenaMonitoring/AthMonitorAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "GeoPrimitives/GeoPrimitives.h"

#include "InDetIdentifier/PixelID.h"
#include "InDetIdentifier/SCT_ID.h"

#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "xAODInDetMeasurement/StripClusterContainer.h"
#include "xAODInDetMeasurement/SpacePointContainer.h"

namespace ActsTrk {

  class SpacePointAnalysisAlg final : 
    public AthMonitorAlgorithm {

  public:
    SpacePointAnalysisAlg(const std::string& name, ISvcLocator* pSvcLocator);
    virtual ~SpacePointAnalysisAlg() override = default;

    virtual StatusCode initialize() override;
    virtual StatusCode fillHistograms(const EventContext& ctx) const override;

  private:
    SG::ReadHandleKey< xAOD::SpacePointContainer > m_spacePointContainerKey {this, "SpacePointContainerKey", "ITkPixelSpacePoints", "Key of input space points"};

    Gaudi::Property< std::string > m_monGroupName {this, "MonGroupName", "ActsSpacePointAnalysisAlg"};
    Gaudi::Property< bool > m_usePixel {this, "UsePixel", true, "enable use of pixel ID or SCT ID"};
    Gaudi::Property< bool > m_useOverlap {this, "UseOverlap", false, "looking at strip strip space points"};
  };
}

#endif // ACTSTRKANALYSIS_SPACEPOINTANALYSISALG_H
