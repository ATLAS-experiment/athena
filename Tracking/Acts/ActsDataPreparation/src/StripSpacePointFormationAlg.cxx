/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "StripSpacePointFormationAlg.h"

#include "InDetReadoutGeometry/SiDetectorElement.h"

#include "xAODInDetMeasurement/StripClusterAuxContainer.h"

#include "AthenaMonitoringKernel/Monitored.h"

namespace ActsTrk {

  //------------------------------------------------------------------------
  StripSpacePointFormationAlg::StripSpacePointFormationAlg(const std::string& name,
							   ISvcLocator* pSvcLocator)
    : AthReentrantAlgorithm(name, pSvcLocator)
    {}

  //-----------------------------------------------------------------------
  StatusCode StripSpacePointFormationAlg::initialize()
  {
    ATH_MSG_DEBUG( "Initializing " << name() << " ... " );

    ATH_CHECK( m_stripClusterContainerKey.initialize() );
    ATH_CHECK( m_stripSpacePointContainerKey.initialize() );
    ATH_CHECK( m_stripOverlapSpacePointContainerKey.initialize( m_processOverlapForStrip) );
    ATH_CHECK( m_stripDetEleCollKey.initialize() );
    ATH_CHECK( m_stripPropertiesKey.initialize() );

    ATH_CHECK( m_beamSpotKey.initialize() );

    if ( not m_monTool.empty() )
      ATH_CHECK( m_monTool.retrieve() );

    return StatusCode::SUCCESS;
  }

  //-------------------------------------------------------------------------
  StatusCode StripSpacePointFormationAlg::execute (const EventContext& ctx) const
  {
    auto timer = Monitored::Timer<std::chrono::milliseconds>( "TIME_execute" );
    auto nReceivedSPsStrip = Monitored::Scalar<int>( "numStripSpacePoints" , 0 );
    auto nReceivedSPsStripOverlap = Monitored::Scalar<int>( "numStripOverlapSpacePoints" , 0 );
    auto mon = Monitored::Group( m_monTool, timer, nReceivedSPsStrip, nReceivedSPsStripOverlap );

    SG::ReadHandle<xAOD::StripClusterContainer> inputStripClusterContainer( m_stripClusterContainerKey, ctx );
    if (!inputStripClusterContainer.isValid()){
        ATH_MSG_FATAL("xAOD::StripClusterContainer with key " << m_stripClusterContainerKey.key() << " is not available...");
        return StatusCode::FAILURE;
    }
    const xAOD::StripClusterContainer* inputClusters = inputStripClusterContainer.cptr();
    ATH_MSG_DEBUG("Retrieved " << inputClusters->size() << " clusters from container " << m_stripClusterContainerKey.key());

    
    auto stripSpacePointContainer = SG::WriteHandle<xAOD::SpacePointContainer>( m_stripSpacePointContainerKey, ctx );
    ATH_MSG_DEBUG( "--- Strip Space Point Container `" << m_stripSpacePointContainerKey.key() << "` created ..." );
    ATH_CHECK( stripSpacePointContainer.record( std::make_unique<xAOD::SpacePointContainer>(),
						std::make_unique<xAOD::SpacePointAuxContainer>() ));
    xAOD::SpacePointContainer* spacePoints = stripSpacePointContainer.ptr();

    
    xAOD::SpacePointContainer* overlapSpacePoints = nullptr;
    SG::WriteHandle<xAOD::SpacePointContainer> stripOverlapSpacePointContainer;
    if (m_processOverlapForStrip) {
      stripOverlapSpacePointContainer = SG::WriteHandle<xAOD::SpacePointContainer>( m_stripOverlapSpacePointContainerKey, ctx );
      ATH_MSG_DEBUG( "--- Strip Overlap Space Point Container `" << m_stripOverlapSpacePointContainerKey.key() << "` created ..." );
      ATH_CHECK( stripOverlapSpacePointContainer.record( std::make_unique<xAOD::SpacePointContainer>(),
							 std::make_unique<xAOD::SpacePointAuxContainer>() ));
      overlapSpacePoints = stripOverlapSpacePointContainer.ptr();     
    }


    // Early exit in case we have no clusters
    // We still are saving an empty space point container in SG
    if (inputClusters->empty()) {
      ATH_MSG_DEBUG("No input clusters found, we stop space point formation");
      return StatusCode::SUCCESS;
    }

    
    SG::ReadCondHandle<InDet::BeamSpotData> beamSpotHandle { m_beamSpotKey, ctx };
    const InDet::BeamSpotData* beamSpot = *beamSpotHandle;
    auto vertex = beamSpot->beamVtx().position();



    SG::ReadCondHandle<InDetDD::SiDetectorElementCollection> stripDetEleHandle(m_stripDetEleCollKey, ctx);
    const InDetDD::SiDetectorElementCollection* stripElements(*stripDetEleHandle);
    if (not stripDetEleHandle.isValid() or stripElements==nullptr) {
      ATH_MSG_FATAL(m_stripDetEleCollKey.fullKey() << " is not available.");
      return StatusCode::FAILURE;
    }

    SG::ReadCondHandle<InDet::SiElementPropertiesTable> stripProperties(m_stripPropertiesKey, ctx);
    const InDet::SiElementPropertiesTable* properties = stripProperties.retrieve();
    if (properties==nullptr) {
      ATH_MSG_FATAL("Pointer of SiElementPropertiesTable (" << m_stripPropertiesKey.fullKey() << ") could not be retrieved");
      return StatusCode::FAILURE;
    }


    // Produce space points
    std::vector<StripSP> sps;
    std::vector<StripSP> osps;
    sps.reserve(inputStripClusterContainer->size() * 0.5);
    osps.reserve(inputStripClusterContainer->size() * 0.5);

    ATH_CHECK( m_spacePointMakerTool->produceSpacePoints(ctx,
							 *inputClusters,
							 *properties,
							 *stripElements,
							 vertex,
							 sps,
							 osps,
							 m_processOverlapForStrip) );

    // using trick for fast insertion
    spacePoints->reserve(sps.size());

    std::vector<xAOD::SpacePoint*> sp_collection;
    sp_collection.reserve(sps.size());
    for (std::size_t i(0); i<sps.size(); ++i)
      sp_collection.push_back(new xAOD::SpacePoint());
    spacePoints->insert(spacePoints->end(), sp_collection.begin(), sp_collection.end());

    // fill
    for (std::size_t i(0); i<sps.size(); ++i) {
      auto& toAdd = sps.at(i);

      // make link to Clusters
      std::vector< const xAOD::UncalibratedMeasurement* > els(
							      {inputClusters->at(toAdd.measurementIndexes[0]),
							       inputClusters->at(toAdd.measurementIndexes[1])}
							      );
      spacePoints->at(i)->setSpacePoint(toAdd.idHashes, 
					toAdd.globPos,
					toAdd.cov_r,
					toAdd.cov_z,
					els,
					toAdd.topHalfStripLength,
					toAdd.bottomHalfStripLength,
					toAdd.topStripDirection,
					toAdd.bottomStripDirection,
					toAdd.stripCenterDistance,
					toAdd.topStripCenter);
    }

    nReceivedSPsStrip = stripSpacePointContainer->size();
    
    // overlap space points now
    if (not m_processOverlapForStrip or not overlapSpacePoints) {
      return StatusCode::SUCCESS;
    }
    
    overlapSpacePoints->reserve(osps.size());

    std::vector<xAOD::SpacePoint*> sp_overlap_collection;
    sp_overlap_collection.reserve(osps.size());
    for (std::size_t i(0); i<osps.size(); ++i)
      sp_overlap_collection.push_back(new xAOD::SpacePoint());
    overlapSpacePoints->insert(overlapSpacePoints->end(), sp_overlap_collection.begin(), sp_overlap_collection.end());
      
    for (std::size_t i(0); i<osps.size(); ++i) {
      auto& toAdd = osps.at(i);
      std::vector< const xAOD::UncalibratedMeasurement* > oels(
							       {inputClusters->at(toAdd.measurementIndexes[0]),
								inputClusters->at(toAdd.measurementIndexes[1])}
							       );
      overlapSpacePoints->at(i)->setSpacePoint(toAdd.idHashes, 
					       toAdd.globPos,
					       toAdd.cov_r,
					       toAdd.cov_z,
					       oels,
					       toAdd.topHalfStripLength,
					       toAdd.bottomHalfStripLength,
					       toAdd.topStripDirection,
					       toAdd.bottomStripDirection,
					       toAdd.stripCenterDistance,
					       toAdd.topStripCenter);
    }
    
    nReceivedSPsStripOverlap = overlapSpacePoints->size();

    return StatusCode::SUCCESS;
  }

}
