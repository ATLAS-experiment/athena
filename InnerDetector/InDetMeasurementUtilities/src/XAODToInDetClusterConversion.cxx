/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "src/XAODToInDetClusterConversion.h"
#include "InDetMeasurementUtilities/ClusterConversionUtilities.h"

#include "InDetIdentifier/PixelID.h"
#include "InDetIdentifier/SCT_ID.h"
#include "HGTD_Identifier/HGTD_ID.h"
#include "xAODInDetMeasurement/ContainerAccessor.h"

#include "PixelReadoutGeometry/PixelModuleDesign.h"
#include "SCT_ReadoutGeometry/StripStereoAnnulusDesign.h"
#include "StoreGate/WriteDecorHandle.h"

#include <map>
#include <cmath>

namespace InDet {

 
  StatusCode XAODToInDetClusterConversion::initialize() {
    ATH_MSG_INFO( "Initializing " << name() << " ... " );

    // Pixel Clusters
    ATH_CHECK( m_pixelDetEleCollKey.initialize(m_processPixel) );
    if (m_processPixel) {
      ATH_CHECK( detStore()->retrieve(m_pixelID,"PixelID") );
    }
    ATH_CHECK( m_inputPixelClusterContainerKey.initialize(m_processPixel) );
    ATH_CHECK( m_pixelClusterContainerLinkKey.initialize(m_processPixel) );
    ATH_CHECK( m_outputPixelClusterContainerKey.initialize(m_processPixel) );
    ATH_CHECK(m_pixelClusterOffSetKey.initialize(m_processPixel));
    ATH_CHECK( m_pixelClusterLinkKey.initialize(m_processPixel));

    // Strip Clusters
    ATH_CHECK( m_stripDetEleCollKey.initialize(m_processStrip) );
    if (m_processStrip) {
        ATH_CHECK( detStore()->retrieve(m_stripID, "SCT_ID") );
    }
    ATH_CHECK( m_inputStripClusterContainerKey.initialize(m_processStrip) );
    ATH_CHECK( m_stripClusterContainerLinkKey.initialize(m_processStrip) );
    ATH_CHECK( m_outputStripClusterContainerKey.initialize(m_processStrip) );
    ATH_CHECK( m_stripClusterOffSetKey.initialize(m_processStrip));
    ATH_CHECK( m_stripClusterLinkKey.initialize(m_processStrip));

    ATH_CHECK( m_lorentzAngleTool.retrieve(EnableTool{not m_lorentzAngleTool.empty()}) );

    // Hgtd Clusters
    ATH_CHECK( m_HGTDDetEleCollKey.initialize(m_processHgtd) );
    if (m_processHgtd) {
      ATH_CHECK( detStore()->retrieve(m_hgtdID, "HGTD_ID") );
    }
    ATH_CHECK( m_inputHgtdClusterContainerKey.initialize(m_processHgtd) );
    ATH_CHECK( m_outputHgtdClusterContainerKey.initialize(m_processHgtd) );
    ATH_CHECK( m_hgdtClusterLinkKey.initialize(m_processHgtd));

    return StatusCode::SUCCESS;
  }

  StatusCode XAODToInDetClusterConversion::execute(const EventContext& ctx) const
  {
    ATH_MSG_DEBUG( "Executing " << name() << " ... ");
    if (m_processPixel) {
      ATH_MSG_DEBUG("Converting Pixel Clusters: xAOD -> InDet");
      ATH_CHECK( convertPixelClusters(ctx) );
    }

    if (m_processStrip) {
      ATH_MSG_DEBUG("Converting Strip Clusters: xAOD -> InDet");
      ATH_CHECK( convertStripClusters(ctx) );
    }

    if (m_processHgtd) {
      ATH_MSG_DEBUG("Converting HGTD Clusters: xAOD -> InDet");
      ATH_CHECK( convertHgtdClusters(ctx) );
    }

    return StatusCode::SUCCESS;
  }

  StatusCode XAODToInDetClusterConversion::convertPixelClusters(const EventContext& ctx) const {
    const InDetDD::SiDetectorElementCollection* pixElements{};
    ATH_CHECK(SG::get(pixElements, m_pixelDetEleCollKey, ctx));

    const xAOD::PixelClusterContainer *inputPixelClusters{};
    ATH_CHECK(SG::get(inputPixelClusters, m_inputPixelClusterContainerKey, ctx));
  
    using Link_t = ElementLink< InDet::PixelClusterCollection >;
    SG::WriteDecorHandle<xAOD::PixelClusterContainer, Link_t> dec_link{m_pixelClusterLinkKey,ctx};
    SG::WriteHandle outputPixelClusterContainer{m_outputPixelClusterContainerKey, ctx};
    ATH_CHECK( outputPixelClusterContainer.record (std::make_unique<InDet::PixelClusterContainer>(m_pixelID->wafer_hash_max(), EventContainers::Mode::OfflineFast)) );
    ATH_MSG_DEBUG( "Container '" << m_outputPixelClusterContainerKey.key() << "' initialised" );

    ATH_CHECK( outputPixelClusterContainer.symLink( m_pixelClusterContainerLinkKey ) );


    // Conversion
    // Access to the cluster from a given detector element is possible
    // via the ContainerAccessor.
    ContainerAccessor<xAOD::PixelCluster, IdentifierHash, 1>
      pixelAccessor ( *inputPixelClusters,
          [] (const xAOD::PixelCluster& cl) -> IdentifierHash { return cl.identifierHash(); },
          pixElements->size());

    const auto& allIdHashes = pixelAccessor.allIdentifiers();
    for (const auto& hashId : allIdHashes) {
      const InDetDD::SiDetectorElement *element = pixElements->getDetectorElement(hashId);
      if ( element == nullptr ) {
        ATH_MSG_FATAL( "Invalid pixel detector element for hash " << hashId);
        return StatusCode::FAILURE;
      }

      // The readout design is a property of the detector element, so resolve it once
      // here rather than for every cluster on the element.
      const InDetDD::PixelModuleDesign* design = TrackingUtilities::pixelModuleDesign(*element);
      if ( design == nullptr ) {
        ATH_MSG_FATAL( "Invalid pixel module design for hash " << hashId);
        return StatusCode::FAILURE;
      }

      std::unique_ptr<InDet::PixelClusterCollection> collection = std::make_unique<InDet::PixelClusterCollection>(hashId);

      // Get the detector element and range for the idHash
      for (const auto& this_range : pixelAccessor.rangesForIdentifierDirect(hashId)) {
        for (auto start = this_range.first; start != this_range.second; ++start) {
          const xAOD::PixelCluster* in_cluster = *start;
          auto cluster = TrackingUtilities::convertXaodToInDetCluster(*in_cluster, *element, *design, *m_pixelID);
          if (!cluster) continue;
          cluster->setHashAndIndex(hashId, collection->size());

          // Add to Collection
          collection->push_back(cluster.get());
          dec_link(*in_cluster) = Link_t{cluster.release(), *collection, ctx};
        }
      }

      InDet::PixelClusterContainer::IDC_WriteHandle lock = outputPixelClusterContainer->getWriteHandle(hashId);
      ATH_CHECK(lock.addOrDelete( std::move(collection) ));

    } // loop on hashIds

    auto offsets = std::make_unique<std::vector<unsigned int>>(m_pixelID->wafer_hash_max(), 0);
    unsigned int counter(0);
    for (const auto coll : *outputPixelClusterContainer) {
      (*offsets)[coll->identifyHash()] = counter;
      counter += coll->size();
    }
    SG::WriteHandle offSetHandle{m_pixelClusterOffSetKey ,ctx};
    ATH_CHECK(offSetHandle.record(std::move(offsets)));

    return StatusCode::SUCCESS;
  }

  StatusCode XAODToInDetClusterConversion::convertStripClusters(const EventContext& ctx) const {
    const InDetDD::SiDetectorElementCollection* stripElements{};
    ATH_CHECK(SG::get(stripElements, m_stripDetEleCollKey, ctx));
    const xAOD::StripClusterContainer *inputStripClusters{};
    ATH_CHECK(SG::get(inputStripClusters, m_inputStripClusterContainerKey, ctx));
    
    using Link_t = ElementLink< InDet::SCT_ClusterCollection >;
    SG::WriteDecorHandle<xAOD::StripClusterContainer, Link_t> dec_link{m_stripClusterLinkKey,ctx};

    SG::WriteHandle outputStripClusterContainer{m_outputStripClusterContainerKey, ctx};
    ATH_CHECK( outputStripClusterContainer.record (std::make_unique<InDet::SCT_ClusterContainer>(m_stripID->wafer_hash_max(), EventContainers::Mode::OfflineFast)) );
    ATH_MSG_DEBUG( "Container '" << m_outputStripClusterContainerKey.key() << "' initialised" );

    ATH_CHECK( outputStripClusterContainer.symLink( m_stripClusterContainerLinkKey ) );

    // Conversion
    // Access to the cluster from a given detector element is possible
    // via the ContainerAccessor.
    ContainerAccessor<xAOD::StripCluster, IdentifierHash, 1>
      stripAccessor ( *inputStripClusters,
          [] (const xAOD::StripCluster& cl) -> IdentifierHash { return cl.identifierHash(); },
          stripElements->size());



    const auto& allIdHashes = stripAccessor.allIdentifiers();
    for (const auto& hashId : allIdHashes) {
      const InDetDD::SiDetectorElement *element = stripElements->getDetectorElement(hashId);
      if ( element == nullptr ) {
        ATH_MSG_FATAL( "Invalid strip detector element for hash " << hashId);
        return StatusCode::FAILURE;
      }

      bool isBarrel = element->isBarrel();
      double shift = not isBarrel ? m_lorentzAngleTool->getLorentzShift(hashId, ctx) : 0.;

      // The readout design is a property of the detector element, so resolve it once
      // here rather than for every cluster on the element.
      const InDetDD::SCT_ModuleSideDesign* design = TrackingUtilities::stripModuleSideDesign(*element);
      if ( design == nullptr ) {
        ATH_MSG_FATAL( "Invalid strip module design for hash " << hashId);
        return StatusCode::FAILURE;
      }

      std::unique_ptr<InDet::SCT_ClusterCollection> collection = std::make_unique<InDet::SCT_ClusterCollection>(hashId);


      // Get the detector element and range for the idHash
      for (const auto& this_range : stripAccessor.rangesForIdentifierDirect(hashId)) {
        for (auto start = this_range.first; start != this_range.second; ++start) {
          const xAOD::StripCluster* in_cluster = *start;

          auto cluster = TrackingUtilities::convertXaodToInDetCluster(*in_cluster, *element, *design, *m_stripID, shift);
          if (!cluster) continue;
          cluster->setHashAndIndex(hashId, collection->size());


          // Add to Collection
          collection->push_back( cluster.get() );
          dec_link(*in_cluster) = Link_t{cluster.release(), *collection, ctx};
        }
      }

      InDet::SCT_ClusterContainer::IDC_WriteHandle lock = outputStripClusterContainer->getWriteHandle(hashId);
      ATH_CHECK(lock.addOrDelete( std::move(collection) ));

    }

    auto offsets = std::make_unique<std::vector<unsigned int>>(m_stripID->wafer_hash_max(), 0);
    unsigned int counter(0);
    for (const auto coll : *outputStripClusterContainer) {
      (*offsets)[coll->identifyHash()] = counter;
      counter += coll->size();
    }
    SG::WriteHandle offSetHandle{m_stripClusterOffSetKey ,ctx};
    ATH_CHECK(offSetHandle.record(std::move(offsets)));

    return StatusCode::SUCCESS;
  }

  StatusCode XAODToInDetClusterConversion::convertHgtdClusters(const EventContext& ctx) const {
    const InDetDD::HGTD_DetectorElementCollection *hgtdElements{};
    ATH_CHECK(SG::get(hgtdElements,m_HGTDDetEleCollKey, ctx));
    
    const xAOD::HGTDClusterContainer *inputHgtdClusters{};
    ATH_CHECK(SG::get(inputHgtdClusters, m_inputHgtdClusterContainerKey, ctx));

    using Link_t = ElementLink< ::HGTD_ClusterCollection >;
    SG::WriteDecorHandle<xAOD::HGTDClusterContainer, Link_t> dec_link{m_hgdtClusterLinkKey,ctx};
    
    SG::WriteHandle outputHgtdClusterContainer{m_outputHgtdClusterContainerKey, ctx};
    ATH_CHECK( outputHgtdClusterContainer.record (std::make_unique<::HGTD_ClusterContainer>(m_hgtdID->wafer_hash_max(), EventContainers::Mode::OfflineFast)) );
    ATH_MSG_DEBUG( "Container '" << m_outputHgtdClusterContainerKey.key() << "' initialised" );

    ContainerAccessor<xAOD::HGTDCluster, IdentifierHash, 1>
      hgtdAccessor ( *inputHgtdClusters,
         [] (const xAOD::HGTDCluster& cl) -> IdentifierHash { return cl.identifierHash(); },
         hgtdElements->size());

    const auto& allIdHashes = hgtdAccessor.allIdentifiers();
    for (const auto& hashId : allIdHashes) {
      const auto *element = InDetDD::HGTDDetEl::getDetectorElement(hashId,*hgtdElements);
      if ( element == nullptr ) {
        ATH_MSG_FATAL( "Invalid hgtd detector element for hash " << hashId);
        return StatusCode::FAILURE;
      }

      std::unique_ptr<::HGTD_ClusterCollection> collection = std::make_unique<::HGTD_ClusterCollection>(hashId);

      // Get the detector element and range for the idHash
      for (const auto& this_range : hgtdAccessor.rangesForIdentifierDirect(hashId)) {
        for (auto start = this_range.first; start != this_range.second; ++start) {
          const xAOD::HGTDCluster* in_cluster = *start;

          auto cluster = TrackingUtilities::convertXaodToInDetCluster(*in_cluster, *element);
          cluster->setHashAndIndex(hashId, collection->size());

          // Add to Collection
          collection->push_back(cluster.get());
          dec_link(*in_cluster) = Link_t{cluster.release(), *collection, ctx};
        }
      }

      ::HGTD_ClusterContainer::IDC_WriteHandle lock = outputHgtdClusterContainer->getWriteHandle(hashId);
      ATH_CHECK(lock.addOrDelete( std::move(collection) ));

    } // loop on hashIds

    ATH_CHECK( outputHgtdClusterContainer.setConst() );

    return StatusCode::SUCCESS;
  }


}


