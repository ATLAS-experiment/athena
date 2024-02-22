/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "src/PhysValTool.h"

namespace ActsTrk {

  PhysValTool::PhysValTool(const std::string & type,
			   const std::string& name,
			   const IInterface* parent)
    : ManagedMonitorToolBase(type, name, parent)
  {}
 
  StatusCode PhysValTool::initialize() 
  {
    ATH_MSG_DEBUG("Initializing " << name() << " ... ");
    
    ATH_CHECK(ManagedMonitorToolBase::initialize());
    ATH_CHECK(m_eventInfo.initialize());

    ATH_CHECK(m_pixelClusterContainerKey.initialize(m_doPixelClusters));
    ATH_CHECK(m_stripClusterContainerKey.initialize(m_doStripClusters));
    ATH_CHECK(m_hgtdClusterContainerKey.initialize(m_doHgtdClusters));
    
    ATH_CHECK(m_pixelSpacePointContainerKey.initialize(m_doPixelSpacePoints));
    ATH_CHECK(m_stripSpacePointContainerKey.initialize(m_doStripSpacePoints));
    ATH_CHECK(m_stripOverlapSpacePointContainerKey.initialize(m_doStripOverlapSpacePoints));

    ATH_CHECK(m_HGTDDetEleCollKey.initialize(m_doHgtdClusters));

    ATH_MSG_DEBUG("Properties:");
    ATH_MSG_DEBUG(m_doPixelClusters);
    ATH_MSG_DEBUG(m_doStripClusters);
    ATH_MSG_DEBUG(m_doHgtdClusters);
    ATH_MSG_DEBUG(m_doPixelSpacePoints);
    ATH_MSG_DEBUG(m_doStripSpacePoints);
    ATH_MSG_DEBUG(m_doStripOverlapSpacePoints);
    
    std::string folder = "SquirrelPlots/Acts";
    if (m_doPixelClusters) {
      m_pixelClusterValidationPlots = 
	std::make_unique< ActsTrk::PixelClusterValidationPlots >(nullptr, 
								 Form("%s/%s/", 
								      folder.c_str(),
								      m_pixelClusterContainerKey.key().c_str()));
    }

    if (m_doStripClusters) {
      m_stripClusterValidationPlots = 
	std::make_unique< ActsTrk::StripClusterValidationPlots >(nullptr, 
								 Form("%s/%s/", 
								      folder.c_str(),
								      m_stripClusterContainerKey.key().c_str()));
    }

    if (m_doPixelSpacePoints) {
      m_pixelSpacePointValidationPlots =
	std::make_unique< ActsTrk::PixelSpacePointValidationPlots >(nullptr,
								    Form("%s/%s/",
									 folder.c_str(),
									 m_pixelSpacePointContainerKey.key().c_str()));
    }

    if (m_doStripSpacePoints) {
      m_stripSpacePointValidationPlots =
	std::make_unique< ActsTrk::StripSpacePointValidationPlots >(nullptr,
								    Form("%s/%s/",
									 folder.c_str(),
									 m_stripSpacePointContainerKey.key().c_str()),
								    "Strip");
    }

    if (m_doStripOverlapSpacePoints) {
      m_stripOverlapSpacePointValidationPlots = 
	std::make_unique< ActsTrk::StripSpacePointValidationPlots >(nullptr,
								    Form("%s/%s/",
									 folder.c_str(),
									 m_stripOverlapSpacePointContainerKey.key().c_str()),
								    "StripOverlap");
    }

    // Schedule HGTD objects
    if (m_doHgtdClusters) {
      m_hgtdClusterValidationPlots =
	std::make_unique< ActsTrk::HgtdClusterValidationPlots >(nullptr,
								Form("%s/%s/",
								     folder.c_str(),
								     m_hgtdClusterContainerKey.key().c_str()));
    }
    
    if (m_doPixelClusters or m_doPixelSpacePoints)
      ATH_CHECK(detStore()->retrieve(m_pixelID, "PixelID"));
    if (m_doStripClusters or m_doStripSpacePoints or m_doStripOverlapSpacePoints)
      ATH_CHECK(detStore()->retrieve(m_stripID, "SCT_ID"));
    if (m_doHgtdClusters)
      ATH_CHECK(detStore()->retrieve(m_hgtdID, "HGTD_ID"));

    return StatusCode::SUCCESS;
  }

  StatusCode PhysValTool::bookHistograms() 
  {
    ATH_MSG_DEBUG("Booking histograms for " << name() << " ... " );

    if (m_doPixelClusters) ATH_CHECK(bookCollection(m_pixelClusterValidationPlots.get()));
    if (m_doStripClusters) ATH_CHECK(bookCollection(m_stripClusterValidationPlots.get()));
    if (m_doHgtdClusters) ATH_CHECK(bookCollection(m_hgtdClusterValidationPlots.get()));
      
    if (m_doPixelSpacePoints) ATH_CHECK(bookCollection(m_pixelSpacePointValidationPlots.get()));
    if (m_doStripSpacePoints) ATH_CHECK(bookCollection(m_stripSpacePointValidationPlots.get()));
    if (m_doStripOverlapSpacePoints) ATH_CHECK(bookCollection(m_stripOverlapSpacePointValidationPlots.get()));

    return StatusCode::SUCCESS;
  }

  StatusCode PhysValTool::fillHgtdClusters(const EventContext& ctx,
					   float beamSpotWeight) {
    ATH_MSG_DEBUG("Analysing HGTD Clusters");
    SG::ReadHandle< xAOD::HGTDClusterContainer > inputHgtdClusterContainer = SG::makeHandle( m_hgtdClusterContainerKey, ctx );
    if (not inputHgtdClusterContainer.isValid()) {
      ATH_MSG_FATAL("xAOD::HGTDClusterContainer with key " << m_hgtdClusterContainerKey.key() << " is not available...");
      return StatusCode::FAILURE;
    }
    const xAOD::HGTDClusterContainer *hgtdClusterContainer = inputHgtdClusterContainer.cptr();

    SG::ReadCondHandle<InDetDD::HGTD_DetectorElementCollection> hgtdDetEleHandle(m_HGTDDetEleCollKey, ctx);
    const InDetDD::HGTD_DetectorElementCollection* hgtdElements(*hgtdDetEleHandle);
    if (not hgtdDetEleHandle.isValid() or hgtdElements==nullptr) {
      ATH_MSG_FATAL(m_HGTDDetEleCollKey.fullKey() << " is not available.");
      return StatusCode::FAILURE;
    }
    
    // Fill plots
    for (const xAOD::HGTDCluster* cluster : *hgtdClusterContainer) {
      m_hgtdClusterValidationPlots->fill(cluster, *hgtdElements, beamSpotWeight, m_hgtdID);
    }
    
    return StatusCode::SUCCESS;
  }

  StatusCode PhysValTool::fillPixelClusters(const EventContext& ctx,
					    float beamSpotWeight) {
    ATH_MSG_DEBUG("Analysing Pixel Clusters");
    SG::ReadHandle< xAOD::PixelClusterContainer > inputPixelClusterContainer = SG::makeHandle( m_pixelClusterContainerKey, ctx );
    if (not inputPixelClusterContainer.isValid()) {
      ATH_MSG_FATAL("xAOD::PixelClusterContainer with key " << m_pixelClusterContainerKey.key() << " is not available...");
      return StatusCode::FAILURE;
    }
    const xAOD::PixelClusterContainer *pixelClusterContainer = inputPixelClusterContainer.cptr();
    
    for (const xAOD::PixelCluster* cluster : *pixelClusterContainer) {
      m_pixelClusterValidationPlots->fill(cluster, beamSpotWeight, m_pixelID);
    }
    
    return StatusCode::SUCCESS;
  }
  
  StatusCode PhysValTool::fillStripClusters(const EventContext& ctx,
					    float beamSpotWeight) {
    ATH_MSG_DEBUG("Analysing Strip Clusters");
    SG::ReadHandle< xAOD::StripClusterContainer > inputStripClusterContainer = SG::makeHandle( m_stripClusterContainerKey, ctx );
    if (not inputStripClusterContainer.isValid()) {
      ATH_MSG_FATAL("xAOD::StripClusterContainer with key " << m_stripClusterContainerKey.key() << " is not available...");
      return StatusCode::FAILURE;
    }
    const xAOD::StripClusterContainer *stripClusterContainer = inputStripClusterContainer.cptr();

    for (const xAOD::StripCluster* cluster : *stripClusterContainer) {
      m_stripClusterValidationPlots->fill(cluster, beamSpotWeight, m_stripID);
    }
    
    return StatusCode::SUCCESS;
  }


  StatusCode PhysValTool::fillPixelSpacePoints(const EventContext& ctx,
					       float beamSpotWeight) {
    ATH_MSG_DEBUG("Analysing Pixel Space Points");
    SG::ReadHandle< xAOD::SpacePointContainer > inputPixelSpacePointContainer = SG::makeHandle( m_pixelSpacePointContainerKey, ctx );
    if (not inputPixelSpacePointContainer.isValid()) {
      ATH_MSG_FATAL("xAOD::SpacePointContainer with key " << m_pixelSpacePointContainerKey.key() << " is not available...");
      return StatusCode::FAILURE;
    }
    const xAOD::SpacePointContainer *pixelSpacePointContainer = inputPixelSpacePointContainer.cptr();

    for (const xAOD::SpacePoint* spacePoint : *pixelSpacePointContainer) {
      m_pixelSpacePointValidationPlots->fill(spacePoint, beamSpotWeight, m_pixelID);
    }
    
    return StatusCode::SUCCESS;
  }
  
  StatusCode PhysValTool::fillStripSpacePoints(const EventContext& ctx,
					       float beamSpotWeight) {
    ATH_MSG_DEBUG("Analysing Strip Space Points");
    SG::ReadHandle< xAOD::SpacePointContainer > inputStripSpacePointContainer = SG::makeHandle( m_stripSpacePointContainerKey, ctx );
    if (not inputStripSpacePointContainer.isValid()) {
      ATH_MSG_FATAL("xAOD::SpacePointContainer with key " << m_stripSpacePointContainerKey.key() << " is not available...");
      return StatusCode::FAILURE;
    }
    const xAOD::SpacePointContainer *stripSpacePointContainer = inputStripSpacePointContainer.cptr();

    for (const xAOD::SpacePoint* spacePoint : *stripSpacePointContainer) {
      m_stripSpacePointValidationPlots->fill(spacePoint, beamSpotWeight, m_stripID);
    }
    
    return StatusCode::SUCCESS;
  }

  StatusCode PhysValTool::fillStripOverlapSpacePoints(const EventContext& ctx,
						      float beamSpotWeight) {
    ATH_MSG_DEBUG("Analysing Strip Overlap Space Points");
    SG::ReadHandle< xAOD::SpacePointContainer > inputStripOverlapSpacePointContainer = SG::makeHandle( m_stripOverlapSpacePointContainerKey, ctx );
    if (not inputStripOverlapSpacePointContainer.isValid()) {
      ATH_MSG_FATAL("xAOD::SpacePointContainer with key " << m_stripOverlapSpacePointContainerKey.key() << " is not available...");
      return StatusCode::FAILURE;
    }
    const xAOD::SpacePointContainer *stripOverlapSpacePointContainer = inputStripOverlapSpacePointContainer.cptr();

    for(const xAOD::SpacePoint* spacePoint : *stripOverlapSpacePointContainer) {
      m_stripOverlapSpacePointValidationPlots->fill(spacePoint, beamSpotWeight, m_stripID);
    }
    
    return StatusCode::SUCCESS;
  }
  
  
  StatusCode PhysValTool::fillHistograms() 
  {
    ATH_MSG_DEBUG("Filling histograms for " << name() << " ... ");
    
    const EventContext& ctx = Gaudi::Hive::currentContext();

    // Get Event Info
    SG::ReadHandle<xAOD::EventInfo> eventInfoHandle = SG::makeHandle(m_eventInfo, ctx);
    if (not eventInfoHandle.isValid()) {
      ATH_MSG_FATAL("Could not retrieve EventInfo with key " << m_eventInfo.key());
      return StatusCode::FAILURE;
    }
    const xAOD::EventInfo* eventInfo = eventInfoHandle.cptr();
    float beamSpotWeight = eventInfo->beamSpotWeight();

    if (m_doPixelClusters) ATH_CHECK(fillPixelClusters(ctx, beamSpotWeight));
    if (m_doStripClusters) ATH_CHECK(fillStripClusters(ctx, beamSpotWeight));
    if (m_doHgtdClusters) ATH_CHECK(fillHgtdClusters(ctx, beamSpotWeight));

    if (m_doPixelSpacePoints) ATH_CHECK(fillPixelSpacePoints(ctx, beamSpotWeight));
    if (m_doStripSpacePoints) ATH_CHECK(fillStripSpacePoints(ctx, beamSpotWeight));
    if (m_doStripOverlapSpacePoints) ATH_CHECK(fillStripOverlapSpacePoints(ctx, beamSpotWeight));
    
    return StatusCode::SUCCESS;
  }
  
  StatusCode PhysValTool::procHistograms() 
  {
    ATH_MSG_DEBUG("Finalising hists for " << name() << "...");
    if (m_doPixelClusters) m_pixelClusterValidationPlots->finalize();
    if (m_doStripClusters) m_stripClusterValidationPlots->finalize();
    if (m_doHgtdClusters) m_hgtdClusterValidationPlots->finalize();
    if (m_doPixelSpacePoints) m_pixelSpacePointValidationPlots->finalize();
    if (m_doStripSpacePoints) m_stripSpacePointValidationPlots->finalize();
    if (m_doStripOverlapSpacePoints) m_stripOverlapSpacePointValidationPlots->finalize();
    return StatusCode::SUCCESS;
  }

}
