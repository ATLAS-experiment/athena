/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration   
*/

#include "src/InDetToXAODSpacePointConversion.h"
#include "InDetMeasurementUtilities/SpacePointConversionUtilities.h"
#include "InDetMeasurementUtilities/ClusterConversionUtilities.h"
#include "InDetPrepRawData/SCT_ClusterCollection.h"
#include "InDetPrepRawData/PixelClusterCollection.h"

#include "InDetPrepRawData/SiCluster.h"
#include "TrkPrepRawData/PrepRawData.h"

namespace InDet {

  InDetToXAODSpacePointConversion::InDetToXAODSpacePointConversion(const std::string &name, 
								   ISvcLocator *pSvcLocator)
    : AthReentrantAlgorithm(name, pSvcLocator)
  {}
  
  StatusCode InDetToXAODSpacePointConversion::initialize()
  { 
    ATH_MSG_DEBUG("Initializing " << name() << " ...");

    ATH_CHECK( m_beamSpotKey.initialize() );

    ATH_CHECK( m_inSpacepointsPixel.initialize(m_processPixel) );
    ATH_CHECK( m_inSpacepointsStrip.initialize(m_processStrip) );
    ATH_CHECK( m_inSpacepointsOverlap.initialize(m_processStrip) );

    ATH_CHECK( m_outSpacepointsPixel.initialize(m_processPixel) );
    ATH_CHECK( m_outSpacepointsStrip.initialize(m_processStrip) );
    ATH_CHECK( m_outSpacepointsOverlap.initialize(m_processStrip) );

    ATH_CHECK( m_pixelDetEleCollKey.initialize(m_processPixel and m_convertClusters) );
    ATH_CHECK( m_stripDetEleCollKey.initialize(m_processStrip and m_convertClusters) );
    
    ATH_CHECK( m_outClustersPixel.initialize(m_processPixel and m_convertClusters) );
    ATH_CHECK( m_outClustersStrip.initialize(m_processStrip and m_convertClusters) );

    if (m_processPixel and m_convertClusters) {
      ATH_CHECK(detStore()->retrieve(m_pixelID,"PixelID"));
      ATH_CHECK(detStore()->retrieve(m_stripID,"SCT_ID"));
    }
    
    return StatusCode::SUCCESS; 
  }
  
  StatusCode InDetToXAODSpacePointConversion::execute(const EventContext& ctx) const
  { 
    ATH_MSG_DEBUG("Executing " << name() << " ...");
    
    // Conds
    SG::ReadCondHandle<InDet::BeamSpotData> beamSpotHandle { m_beamSpotKey, ctx };
    const InDet::BeamSpotData* beamSpot = *beamSpotHandle;
    auto vertex = beamSpot->beamVtx().position();

    // Cluster Containers if requested
    std::unique_ptr< xAOD::PixelClusterContainer > pixel_cluster_xaod_container = std::make_unique< xAOD::PixelClusterContainer >();
    std::unique_ptr< xAOD::PixelClusterAuxContainer > pixel_cluster_xaod_aux_container = std::make_unique< xAOD::PixelClusterAuxContainer >();
    pixel_cluster_xaod_container->setStore( pixel_cluster_xaod_aux_container.get() );

    std::unique_ptr< xAOD::StripClusterContainer > strip_cluster_xaod_container = std::make_unique< xAOD::StripClusterContainer >();
    std::unique_ptr< xAOD::StripClusterAuxContainer > strip_cluster_xaod_aux_container =  std::make_unique< xAOD::StripClusterAuxContainer >();
    strip_cluster_xaod_container->setStore( strip_cluster_xaod_aux_container.get() );
    
    // Convert
    if (m_processPixel.value()) {
      ATH_CHECK( convertPixel(ctx, pixel_cluster_xaod_container.get()) );
    }

    if (m_processStrip.value()) {
      std::unordered_map<Identifier, std::size_t> mapClusters{};
      ATH_CHECK( convertStrip(ctx, vertex,
			      strip_cluster_xaod_container.get(),
			      mapClusters) );
      ATH_CHECK( convertStripOverlap(ctx, vertex,
				     strip_cluster_xaod_container.get(),
				     mapClusters) );
    }

    // Save optional clusters
    if (m_convertClusters) {
      if (m_processPixel.value()) {
	SG::WriteHandle< xAOD::PixelClusterContainer > pixel_cluster_xaod_handle = SG::makeHandle( m_outClustersPixel, ctx );
	ATH_CHECK( pixel_cluster_xaod_handle.record( std::move(pixel_cluster_xaod_container),
						     std::move(pixel_cluster_xaod_aux_container) ) );
      }

      if (m_processStrip.value()) {
	SG::WriteHandle< xAOD::StripClusterContainer > strip_cluster_xaod_handle = SG::makeHandle( m_outClustersStrip, ctx );
	ATH_CHECK( strip_cluster_xaod_handle.record( std::move(strip_cluster_xaod_container),
						     std::move(strip_cluster_xaod_aux_container) ) );
      }
    }

    return StatusCode::SUCCESS; 
  }

  StatusCode InDetToXAODSpacePointConversion::convertPixel(const EventContext& ctx,
							   xAOD::PixelClusterContainer* cluster_xaod_container) const
  {
    static const SG::AuxElement::Accessor< ElementLink< ::SpacePointCollection > > linkAcc("pixelSpacePointLink");
    
    const InDetDD::SiDetectorElementCollection* pixElements = nullptr;
    if (m_convertClusters) {
      SG::ReadCondHandle<InDetDD::SiDetectorElementCollection> pixelDetEleHandle = SG::makeHandle(m_pixelDetEleCollKey, ctx);
      if (not pixelDetEleHandle.isValid()) {
	ATH_MSG_FATAL(m_pixelDetEleCollKey.fullKey() << " is not available.");
	return StatusCode::FAILURE;
      }
      pixElements = pixelDetEleHandle.cptr();
    }
    
    // Input
    SG::ReadHandle< ::SpacePointContainer > pixel_handle = SG::makeHandle( m_inSpacepointsPixel, ctx );
    ATH_CHECK( pixel_handle.isValid() );
    const ::SpacePointContainer* pixel_container = pixel_handle.cptr();

    // Output
    std::unique_ptr< xAOD::SpacePointContainer > pixel_xaod_container = std::make_unique< xAOD::SpacePointContainer >();
    std::unique_ptr< xAOD::SpacePointAuxContainer > pixel_xaod_aux_container = std::make_unique< xAOD::SpacePointAuxContainer >();
    pixel_xaod_container->setStore( pixel_xaod_aux_container.get() );

    pixel_xaod_container->reserve(pixel_container->size());
    pixel_xaod_aux_container->reserve(pixel_container->size());

    // Conversion
    for (const ::SpacePointCollection *spc : *pixel_container) {
      for (const Trk::SpacePoint *sp : *spc) {
	const InDet::PixelSpacePoint *indetSP = dynamic_cast<const InDet::PixelSpacePoint *>(sp);
	
	pixel_xaod_container->push_back( new xAOD::SpacePoint() );
	ATH_CHECK( TrackingUtilities::convertTrkToXaodPixelSpacePoint(*indetSP, *pixel_xaod_container->back()) );

	// Also make cluster object, if requested
	if (m_convertClusters) {
	  const std::pair<const Trk::PrepRawData*, const Trk::PrepRawData*>& clusterList = sp->clusterList();
	  const InDet::PixelCluster* theCluster = dynamic_cast<const InDet::PixelCluster*>(clusterList.first);
	  if (theCluster == nullptr) {
	    ATH_MSG_FATAL("Cannot cast InDet::PixelCluster");
	  }

	  auto clusterId = clusterList.first->identify();
	  const InDetDD::SiDetectorElement *element = pixElements->getDetectorElement(m_pixelID->wafer_hash(m_pixelID->wafer_id(clusterId)));
	  if ( element == nullptr ) {
	    ATH_MSG_FATAL( "Invalid pixel detector element for cluster identifier " << clusterId );
	    return StatusCode::FAILURE;
	  }

	  xAOD::PixelCluster * pixelCl = new xAOD::PixelCluster();
	  cluster_xaod_container->push_back(pixelCl);
	  ATH_CHECK( TrackingUtilities::convertInDetToXaodCluster(*theCluster, *element, *pixelCl) );
	  pixel_xaod_container->back()->setMeasurements( {cluster_xaod_container->back()} );
	}

	// Add link to this space point
	ElementLink< ::SpacePointCollection > link(indetSP, *spc);
	linkAcc(*pixel_xaod_container->back()) = link;
      }
    }

    // Store
    SG::WriteHandle< xAOD::SpacePointContainer > pixel_xaod_handle = SG::makeHandle( m_outSpacepointsPixel, ctx );
    ATH_CHECK( pixel_xaod_handle.record( std::move(pixel_xaod_container), std::move(pixel_xaod_aux_container) ) );

    return StatusCode::SUCCESS;
  }

  StatusCode InDetToXAODSpacePointConversion::convertStrip(const EventContext& ctx,
							   const Amg::Vector3D& vertex,
							   xAOD::StripClusterContainer* cluster_xaod_container,
							   std::unordered_map<Identifier, std::size_t>& mapClusters) const
  {
    const InDetDD::SiDetectorElementCollection* stripElements = nullptr;
    if (m_convertClusters) {
      SG::ReadCondHandle<InDetDD::SiDetectorElementCollection> stripDetEleHandle(m_stripDetEleCollKey, ctx);
      if (not stripDetEleHandle.isValid()) {
	ATH_MSG_FATAL(m_stripDetEleCollKey.fullKey() << " is not available.");
	return StatusCode::FAILURE;
      }    
      stripElements = stripDetEleHandle.cptr();
    }
    
    static const SG::AuxElement::Accessor< ElementLink< ::SpacePointCollection > > linkAcc("sctSpacePointLink");

    // Input
    SG::ReadHandle< ::SpacePointContainer > strip_handle = SG::makeHandle( m_inSpacepointsStrip, ctx );
    ATH_CHECK( strip_handle.isValid() );
    const ::SpacePointContainer* strip_container = strip_handle.cptr();
    
    // Output
    std::unique_ptr< xAOD::SpacePointContainer > strip_xaod_container = std::make_unique< xAOD::SpacePointContainer >();
    std::unique_ptr< xAOD::SpacePointAuxContainer > strip_xaod_aux_container = std::make_unique< xAOD::SpacePointAuxContainer >();
    strip_xaod_container->setStore( strip_xaod_aux_container.get() );
    
    strip_xaod_container->reserve(strip_container->size());
    strip_xaod_aux_container->reserve(strip_container->size());
    
    // Conversion
    for (const ::SpacePointCollection *spc : *strip_container) {
      for (const Trk::SpacePoint *sp : *spc) {
	const InDet::SCT_SpacePoint *indetSP = dynamic_cast<const InDet::SCT_SpacePoint *>(sp);

	strip_xaod_container->push_back( new xAOD::SpacePoint() );	
	ATH_CHECK( TrackingUtilities::convertTrkToXaodStripSpacePoint(*indetSP, vertex, *strip_xaod_container->back()) );

	// Also make cluster object, if requested
        if (m_convertClusters) {
	  const std::pair<const Trk::PrepRawData*, const Trk::PrepRawData*>& clusterList = sp->clusterList();
          const InDet::SCT_Cluster* theCluster1 = dynamic_cast<const InDet::SCT_Cluster*>(clusterList.first);
	  const InDet::SCT_Cluster* theCluster2 = dynamic_cast<const InDet::SCT_Cluster*>(clusterList.second);
          if (theCluster1 == nullptr or
	      theCluster2 == nullptr) {
            ATH_MSG_FATAL("Cannot cast InDet::SCT_Cluster");
          }
	  
	  auto clusterId1 = clusterList.first->identify();
	  auto clusterId2 = clusterList.second->identify();
	  const InDetDD::SiDetectorElement *element1 = stripElements->getDetectorElement(m_stripID->wafer_hash(m_stripID->wafer_id(clusterId1)));
	  const InDetDD::SiDetectorElement *element2 = stripElements->getDetectorElement(m_stripID->wafer_hash(m_stripID->wafer_id(clusterId2)));
    if ( element1 == nullptr ) {
      ATH_MSG_FATAL( "Invalid strip detector element for cluster (1) identifiers " << clusterId1 );
      return StatusCode::FAILURE;
    }
    if ( element2 == nullptr ) {
      ATH_MSG_FATAL( "Invalid strip detector element for cluster (2) identifiers " << clusterId2 );
      return StatusCode::FAILURE;
    }
    //tries to insert an entry with key clusterId. Returns false and does nothing if the key already exists.
    auto insertEntry = [&mapClusters, cluster_xaod_container](const auto & clusterId)-> bool{
      const auto & [p,inserted] = mapClusters.try_emplace(clusterId, cluster_xaod_container->size());
      return inserted;
    };
	  // if cluster is not there, add entry and add cluster to container
	  if (insertEntry(clusterId1)){
	    xAOD::StripCluster * stripCl1 = new xAOD::StripCluster();
	    cluster_xaod_container->push_back(stripCl1);
	    ATH_CHECK( TrackingUtilities::convertInDetToXaodCluster(*theCluster1, *element1, *stripCl1) );
	  }
	  if (insertEntry(clusterId2)) {
	    xAOD::StripCluster * stripCl2 = new xAOD::StripCluster();
	    cluster_xaod_container->push_back(stripCl2);
	    ATH_CHECK( TrackingUtilities::convertInDetToXaodCluster(*theCluster2, *element2, *stripCl2) );
	  }
	  // Get the clusters so that we can add a link to them
          xAOD::StripCluster * stripCl1 = cluster_xaod_container->at(mapClusters[clusterId1]);
	  xAOD::StripCluster * stripCl2 = cluster_xaod_container->at(mapClusters[clusterId2]);
	  strip_xaod_container->back()->setMeasurements( {stripCl1, stripCl2} );
	}
	
	// Add link to this space point
	ElementLink< ::SpacePointCollection > link(indetSP, *spc);
	linkAcc(*strip_xaod_container->back()) = link;
      }
    }

    // Store
    SG::WriteHandle< xAOD::SpacePointContainer > strip_xaod_handle = SG::makeHandle( m_outSpacepointsStrip, ctx );
    ATH_CHECK( strip_xaod_handle.record( std::move(strip_xaod_container), std::move(strip_xaod_aux_container) ) );

    return StatusCode::SUCCESS;
  }


  StatusCode InDetToXAODSpacePointConversion::convertStripOverlap(const EventContext& ctx,
								  const Amg::Vector3D& vertex,
								  xAOD::StripClusterContainer* cluster_xaod_container,
								  std::unordered_map<Identifier, std::size_t>& mapClusters) const
  {
    const InDetDD::SiDetectorElementCollection* stripElements = nullptr;
    if (m_convertClusters) {
      SG::ReadCondHandle<InDetDD::SiDetectorElementCollection> stripDetEleHandle(m_stripDetEleCollKey, ctx);
      if (not stripDetEleHandle.isValid()) {
        ATH_MSG_FATAL(m_stripDetEleCollKey.fullKey() << " is not available.");
        return StatusCode::FAILURE;
      }
      stripElements = stripDetEleHandle.cptr();
    }
    
    // Inputs
    SG::ReadHandle< ::SpacePointOverlapCollection > strip_overlap_handle = SG::makeHandle( m_inSpacepointsOverlap, ctx );
    ATH_CHECK( strip_overlap_handle.isValid() );
    const ::SpacePointOverlapCollection* strip_overlap_container = strip_overlap_handle.cptr();

    // Outputs
    std::unique_ptr< xAOD::SpacePointContainer > strip_overlap_xaod_container = std::make_unique< xAOD::SpacePointContainer >();
    std::unique_ptr< xAOD::SpacePointAuxContainer > strip_overlap_xaod_aux_container = std::make_unique< xAOD::SpacePointAuxContainer >();
    strip_overlap_xaod_container->setStore( strip_overlap_xaod_aux_container.get() );

    strip_overlap_xaod_container->reserve(strip_overlap_container->size());
    strip_overlap_xaod_aux_container->reserve(strip_overlap_container->size());

    // Conversion
    static const SG::AuxElement::Accessor< ElementLink< ::SpacePointOverlapCollection > > stripSpacePointLinkAcc("stripOverlapSpacePointLink");

    for (const Trk::SpacePoint *sp : *strip_overlap_container) {
      const InDet::SCT_SpacePoint *indetSP = dynamic_cast<const InDet::SCT_SpacePoint *>(sp);
      
      strip_overlap_xaod_container->push_back( new xAOD::SpacePoint() );
      ATH_CHECK( TrackingUtilities::convertTrkToXaodStripSpacePoint(*indetSP, vertex, *strip_overlap_xaod_container->back()) );

      // Also make cluster object, if requested
      if (m_convertClusters) {
	const std::pair<const Trk::PrepRawData*, const Trk::PrepRawData*>& clusterList = sp->clusterList();
	const InDet::SCT_Cluster* theCluster1 = dynamic_cast<const InDet::SCT_Cluster*>(clusterList.first);
	const InDet::SCT_Cluster* theCluster2 = dynamic_cast<const InDet::SCT_Cluster*>(clusterList.second);
	if (theCluster1 == nullptr or
	    theCluster2 == nullptr) {
	  ATH_MSG_FATAL("Cannot cast InDet::SCT_Cluster");
	}
	
	auto clusterId1 = clusterList.first->identify();
	auto clusterId2 = clusterList.second->identify();
	const InDetDD::SiDetectorElement *element1 = stripElements->getDetectorElement(m_stripID->wafer_hash(m_stripID->wafer_id(clusterId1)));
	const InDetDD::SiDetectorElement *element2 = stripElements->getDetectorElement(m_stripID->wafer_hash(m_stripID->wafer_id(clusterId2)));
	
	if ( element1 == nullptr ) {
	  ATH_MSG_FATAL( "Invalid strip detector element for cluster (1) identifiers " << clusterId1 );
	  return StatusCode::FAILURE;
	}
	if ( element2 == nullptr ) {
	  ATH_MSG_FATAL( "Invalid strip detector element for cluster (2) identifiers " << clusterId2 );
	  return StatusCode::FAILURE;
	}
  //tries to insert an entry with key clusterId. Returns false and does nothing if the key already exists.
  auto insertEntry = [&mapClusters, cluster_xaod_container](const auto & clusterId)-> bool{
    const auto & [p,inserted] = mapClusters.try_emplace(clusterId, cluster_xaod_container->size());
    return inserted;
  };
	if (insertEntry(clusterId1)) {
	  xAOD::StripCluster * stripCl1 = new xAOD::StripCluster();
	  cluster_xaod_container->push_back(stripCl1);
	  ATH_CHECK( TrackingUtilities::convertInDetToXaodCluster(*theCluster1, *element1, *stripCl1) );
	}
	if (insertEntry(clusterId2)) {
	  xAOD::StripCluster * stripCl2 = new xAOD::StripCluster();
	  cluster_xaod_container->push_back(stripCl2);
	  ATH_CHECK( TrackingUtilities::convertInDetToXaodCluster(*theCluster2, *element2, *stripCl2) );
  }
	
	xAOD::StripCluster * stripCl1 = cluster_xaod_container->at(mapClusters[clusterId1]);
	xAOD::StripCluster * stripCl2 = cluster_xaod_container->at(mapClusters[clusterId2]);
	strip_overlap_xaod_container->back()->setMeasurements( {stripCl1, stripCl2} );
      }
      
      ElementLink< ::SpacePointOverlapCollection > TrkLink(sp, *strip_overlap_container);
      stripSpacePointLinkAcc( *strip_overlap_xaod_container->back() ) = TrkLink;
    }

    // Store
    SG::WriteHandle< xAOD::SpacePointContainer > strip_overlap_xaod_handle = SG::makeHandle( m_outSpacepointsOverlap, ctx );
    ATH_CHECK( strip_overlap_xaod_handle.record( std::move(strip_overlap_xaod_container), std::move(strip_overlap_xaod_aux_container) ) );

    return StatusCode::SUCCESS;
  }

}
