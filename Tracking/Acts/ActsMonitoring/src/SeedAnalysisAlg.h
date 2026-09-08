/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRKANALYSIS_SEEDANALYSISALG_H
#define ACTSTRKANALYSIS_SEEDANALYSISALG_H

#include "AthenaMonitoring/AthMonitorAlgorithm.h"
#include "TrkTruthData/PRD_MultiTruthCollection.h"

#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "xAODInDetMeasurement/StripClusterContainer.h"

#include "InDetPrepRawData/PixelClusterContainer.h"
#include "InDetPrepRawData/SCT_ClusterContainer.h"

#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include "ActsEvent/SeedContainer.h"

#include "BeamSpotConditionsData/BeamSpotData.h"
#include "MagFieldConditions/AtlasFieldCacheCondObj.h"
#include "MagFieldElements/AtlasFieldCache.h"

#include "GaudiKernel/ToolHandle.h"
#include "ActsToolInterfaces/ITrackParamsEstimationTool.h"
#include "ActsGeometryInterfaces/ITrackingGeometrySvc.h"
#include "ActsGeometryInterfaces/IGeometryRealmConvTool.h"
#include "ActsEvent/ContextUtility.h"

#include "xAODEventInfo/EventInfo.h"

namespace ActsTrk {

  class SeedAnalysisAlg final :
    public AthMonitorAlgorithm {
  public:
    SeedAnalysisAlg(const std::string& name, ISvcLocator* pSvcLocator);
    virtual ~SeedAnalysisAlg() override = default;

    virtual StatusCode initialize() override;
    virtual StatusCode fillHistograms(const EventContext& ctx) const override;

  private:
    StatusCode fillTruthHistograms(const EventContext& ctx,
				   const ActsTrk::SeedContainer& seed_container,
				   std::vector<int>&,
				   std::vector<double>&) const;

    const Identifier identify(const xAOD::PixelCluster&) const;
    const Identifier identify(const xAOD::StripCluster&) const;

  private:
    void matchParticleToSeedClusters(const PRD_MultiTruthCollection* prdTruth,
				     const Identifier& id,
				     std::map<int, int>& countMap) const;
    std::pair<int, double> findSeedMajorityTruthParticle(const std::map<int, int>& countMap) const;

    std::array<float, 7> estimateParameters(const ActsTrk::Seed& seed,
                                            float pTPerHelixRadius) const;
      
  private:
    ToolHandle< ActsTrk::ITrackParamsEstimationTool > m_paramEstimationTool {this, "TrackParamsEstimationTool", "", "Track Param Estimation from Seeds"};

    PublicToolHandle<IGeometryRealmConvTool> m_geometryConvTool{this, "GeometryRealmConvTool", ""};

    SG::ReadHandleKey< ActsTrk::SeedContainer > m_inputSeedColletionKey {this,  "InputSeedCollection", "", ""}; 
    ServiceHandle<ActsTrk::ITrackingGeometrySvc> m_trackingGeometrySvc{this, "TrackingGeometrySvc", "ActsTrackingGeometrySvc"};

    SG::ReadHandleKey< PRD_MultiTruthCollection > m_prdTruth {this, "ITkClustersTruth", "", ""};
    SG::ReadCondHandleKey< InDetDD::SiDetectorElementCollection > m_detEleCollKey {this, "DetectorElements", "", "Key of input SiDetectorElementCollection"};

    SG::ReadCondHandleKey< InDet::BeamSpotData > m_beamSpotKey {this, "BeamSpotKey", "BeamSpotData", "SG key for beam spot"};
    SG::ReadCondHandleKey< AtlasFieldCacheCondObj > m_fieldCondObjInputKey {this, "AtlasFieldCacheCondObj", "fieldCondObj",
        "Name of the Magnetic Field conditions object key"};

    Gaudi::Property< std::string > m_monGroupName
      {this, "MonGroupName", "ActsSeedAnalysisAlg"};

    Gaudi::Property< bool > m_useTopSp {this, "useTopSp", false, "Use top SP. By default, use bottom SP."};
    Gaudi::Property< bool > m_usePixel {this, "UsePixel", true, ""};

    /** @brief Auxiliary class to access the calibration context.
        TODO: Could also be used for magnetic field and geometry? */
    ContextUtility m_ctxProvider{this};
  };

}

#endif
