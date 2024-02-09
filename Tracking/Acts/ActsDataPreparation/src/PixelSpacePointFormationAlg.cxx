/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "PixelSpacePointFormationAlg.h"

#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "PixelReadoutGeometry/PixelModuleDesign.h"

#include "xAODInDetMeasurement/PixelClusterAuxContainer.h"

#include "AthenaMonitoringKernel/Monitored.h"

namespace ActsTrk {

  //------------------------------------------------------------------------
  PixelSpacePointFormationAlg::PixelSpacePointFormationAlg(const std::string& name,
							   ISvcLocator* pSvcLocator)
    : AthReentrantAlgorithm(name, pSvcLocator)
    {}

  //-----------------------------------------------------------------------
  StatusCode PixelSpacePointFormationAlg::initialize()
  {
    ATH_MSG_DEBUG( "Initializing " << name() << " ... " );

    ATH_CHECK( m_pixelClusterContainerKey.initialize() );
    ATH_CHECK( m_pixelSpacePointContainerKey.initialize() );
    ATH_CHECK( m_pixelDetEleCollKey.initialize() );
    ATH_CHECK( m_spacePointMakerTool.retrieve() );

    if ( not m_monTool.empty() )
      ATH_CHECK( m_monTool.retrieve() );

    return StatusCode::SUCCESS;
  }

  //-------------------------------------------------------------------------
  StatusCode PixelSpacePointFormationAlg::execute (const EventContext& ctx) const
  {
    auto timer = Monitored::Timer<std::chrono::milliseconds>( "TIME_execute" );
    auto nReceivedSPsPixel = Monitored::Scalar<int>( "numPixSpacePoints" , 0 );
    auto mon = Monitored::Group( m_monTool, timer, nReceivedSPsPixel );

    SG::ReadHandle<xAOD::PixelClusterContainer> inputPixelClusterContainer( m_pixelClusterContainerKey, ctx );
    if (!inputPixelClusterContainer.isValid()){
      ATH_MSG_FATAL("xAOD::PixelClusterContainer with key " << m_pixelClusterContainerKey.key() << " is not available...");
      return StatusCode::FAILURE;
    }
    const xAOD::PixelClusterContainer *pixelClusters = inputPixelClusterContainer.cptr();
    ATH_MSG_DEBUG("Retrieved " << pixelClusters->size() << " clusters from container " << m_pixelClusterContainerKey.key());
    
    auto pixelSpacePointContainer = SG::WriteHandle<xAOD::SpacePointContainer>( m_pixelSpacePointContainerKey, ctx );
    ATH_MSG_DEBUG( "--- Pixel Space Point Container `" << m_pixelSpacePointContainerKey.key() << "` created ..." );
    ATH_CHECK(pixelSpacePointContainer.record( std::make_unique<xAOD::SpacePointContainer>(),
					       std::make_unique<xAOD::SpacePointAuxContainer>() ));
    xAOD::SpacePointContainer *pixelSpacePoints = pixelSpacePointContainer.ptr();
    // Reserve space
    pixelSpacePoints->reserve(pixelClusters->size());

    // Early exit in case we have no clusters
    // We still are saving an empty space point container in SG
    if (pixelClusters->empty()) {
      ATH_MSG_DEBUG("No input clusters found, we stop space point formation");
      return StatusCode::SUCCESS;
    }
    
    SG::ReadCondHandle<InDetDD::SiDetectorElementCollection> pixelDetEleHandle(m_pixelDetEleCollKey, ctx);
    const InDetDD::SiDetectorElementCollection* pixelElements(*pixelDetEleHandle);
    if (not pixelDetEleHandle.isValid() or pixelElements==nullptr) {
      ATH_MSG_FATAL(m_pixelDetEleCollKey.fullKey() << " is not available.");
      return StatusCode::FAILURE;
    }

    // using trick for fast insertion
    std::vector< xAOD::SpacePoint* > preCollection;
    preCollection.reserve(pixelClusters->size());
    for ( std::size_t i(0); i<inputPixelClusterContainer->size(); ++i)
      preCollection.push_back( new xAOD::SpacePoint() );
    pixelSpacePoints->insert(pixelSpacePoints->end(), preCollection.begin(), preCollection.end());

    /// production of ActsTrk::SpacePoint from pixel clusters
    /// Pixel space points are created directly from the clusters global position
    for (std::size_t idx(0); idx<inputPixelClusterContainer->size(); ++idx) {
      auto cluster = inputPixelClusterContainer->at(idx);
      const InDetDD::SiDetectorElement* pixelElement = pixelElements->getDetectorElement(cluster->identifierHash());
      if (pixelElement == nullptr) {
        ATH_MSG_FATAL("Element pointer is nullptr");
        return StatusCode::FAILURE;
      }

      ATH_CHECK( m_spacePointMakerTool->producePixelSpacePoint(*cluster,
      							       *pixelSpacePoints->at(idx),
      							       *pixelElement ) );
    }

    nReceivedSPsPixel = pixelSpacePointContainer->size();

    return StatusCode::SUCCESS;
  }

} //namespace
