/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTOXAOD_SPACEPOINT_CONVERSION_H
#define INDETTOXAOD_SPACEPOINT_CONVERSION_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "xAODInDetMeasurement/PixelClusterAuxContainer.h"

#include "xAODInDetMeasurement/StripClusterContainer.h"
#include "xAODInDetMeasurement/StripClusterAuxContainer.h"

#include "xAODInDetMeasurement/SpacePointContainer.h"
#include "xAODInDetMeasurement/SpacePointAuxContainer.h"

#include "TrkSpacePoint/SpacePointContainer.h" 
#include "TrkSpacePoint/SpacePointOverlapCollection.h"

#include "BeamSpotConditionsData/BeamSpotData.h"

#include "InDetIdentifier/PixelID.h"
#include "InDetIdentifier/SCT_ID.h"
#include "InDetReadoutGeometry/SiDetectorElementCollection.h"

#include <unordered_map>

namespace InDet {

  class InDetToXAODSpacePointConversion :
    public AthReentrantAlgorithm {
  public:
    /// Constructor with parameters:
    InDetToXAODSpacePointConversion(const std::string &name, ISvcLocator *pSvcLocator);
    
    //@name Usual algorithm methods
    //@{
    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override;
    //@{

  private:
    StatusCode convertPixel(const EventContext& ctx,
			    xAOD::PixelClusterContainer* cluster_xaod_container) const;
    StatusCode convertStrip(const EventContext& ctx, 
			    const Amg::Vector3D& vertex,
			    xAOD::StripClusterContainer* cluster_xaod_container,
			    std::unordered_map<Identifier, std::size_t>& mapClusters) const;
    StatusCode convertStripOverlap(const EventContext& ctx, 
				   const Amg::Vector3D& vertex,
				   xAOD::StripClusterContainer* cluster_xaod_container,
				   std::unordered_map<Identifier, std::size_t>& mapClusters) const;

  private:
    SG::ReadCondHandleKey<InDet::BeamSpotData> m_beamSpotKey { this, "BeamSpotKey", "BeamSpotData", 
	"SG key for beam spot" };

    SG::ReadHandleKey< ::SpacePointContainer > m_inSpacepointsPixel {this, "InputPixelSpacePointsName", "ITkPixelSpacePoints", 
	"Input Pixel space points container"};
    SG::ReadHandleKey< ::SpacePointContainer > m_inSpacepointsStrip {this, "InputStripSpacePointsName", "ITkStripSpacePoints", 
	"Input Strip space points container"};
    SG::ReadHandleKey< ::SpacePointOverlapCollection > m_inSpacepointsOverlap {this, "InputStripOverlapSpacePointsName", "ITkOverlapSpacePoints",
	"Input Strip overlap space points container"};

    SG::WriteHandleKey< xAOD::PixelClusterContainer > m_outClustersPixel {this, "OutputPixelClustersName", "ITkPixelClusters",
      "Output Pixel cluster container"};
    SG::WriteHandleKey< xAOD::StripClusterContainer > m_outClustersStrip {this, "OutputStripClustersName", "ITkStripClusters",
      "Output Strip cluster container"};
    
    SG::WriteHandleKey< xAOD::SpacePointContainer > m_outSpacepointsPixel {this, "OutputPixelSpacePointsName", "ITkPixelSpacePoints",
	"Output Pixel space points container"};
    SG::WriteHandleKey< xAOD::SpacePointContainer > m_outSpacepointsStrip {this, "OutputStripSpacePointsName", "ITkStripSpacePoints",
	"Output Strip space points container"};
    SG::WriteHandleKey< xAOD::SpacePointContainer > m_outSpacepointsOverlap {this, "OutputStripOverlapSpacePointsName", "ITkStripOverlapSpacePoints",
	"Output Strip Overlap space points container"};

    SG::ReadCondHandleKey<InDetDD::SiDetectorElementCollection> m_pixelDetEleCollKey {this, "PixelDetEleCollKey", "ITkPixelDetectorElementCollection", "Key of SiDetectorElementCollection for Pixel"};
    SG::ReadCondHandleKey<InDetDD::SiDetectorElementCollection> m_stripDetEleCollKey {this, "StripDetEleCollKey", "ITkStripDetectorElementCollection", "Key of SiDetectorElementCollection for Strip"};
    
    Gaudi::Property<bool> m_convertClusters {this, "ConvertClusters", false};
    Gaudi::Property<bool> m_processPixel {this, "ProcessPixel", true};
    Gaudi::Property<bool> m_processStrip {this, "ProcessStrip", true};

    const PixelID* m_pixelID {nullptr};
    const SCT_ID* m_stripID {nullptr};
  };

}

#endif

