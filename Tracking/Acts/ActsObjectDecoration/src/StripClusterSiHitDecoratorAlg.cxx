/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "src/StripClusterSiHitDecoratorAlg.h"
#include "StoreGate/WriteDecorHandle.h"
#include "xAODInDetMeasurement/ContainerAccessor.h"
#include "Identifier/IdentifierHash.h"
#include "ReadoutGeometryBase/SiCellId.h"
#include "src/detail/Utilities.h"

namespace ActsTrk {

  StripClusterSiHitDecoratorAlg::StripClusterSiHitDecoratorAlg(const std::string &name,
							       ISvcLocator *pSvcLocator)
    : AthReentrantAlgorithm(name, pSvcLocator)
  {}

  StatusCode StripClusterSiHitDecoratorAlg::initialize()
  {
    ATH_MSG_DEBUG( "Initializing " << name() << " ..." );

    ATH_CHECK( m_inputMeasurementsKey.initialize() );
    ATH_CHECK( m_inputClustersKey.initialize() );
    ATH_CHECK( m_SDOcontainer_key.initialize() );
    ATH_CHECK( m_stripDetEleCollKey.initialize() );
    ATH_CHECK( m_siHitsKey.initialize() );
    
    // SDO decorations
    m_sdo_words = m_inputMeasurementsKey.key() + "." + m_sdo_words.key();
    m_sdo_depositsBarcode = m_inputMeasurementsKey.key() + "." + m_sdo_depositsBarcode.key();
    m_sdo_depositsEnergy = m_inputMeasurementsKey.key() + "." + m_sdo_depositsEnergy.key();

    ATH_CHECK( m_sdo_words.initialize() );
    ATH_CHECK( m_sdo_depositsBarcode.initialize() );
    ATH_CHECK( m_sdo_depositsEnergy.initialize() );

    // SiHit decorations
    m_sihit_energyDeposit_decor_key = m_inputMeasurementsKey.key() + "." + m_sihit_energyDeposit_decor_key.key();
    m_sihit_meanTime_decor_key = m_inputMeasurementsKey.key() + "." + m_sihit_meanTime_decor_key.key();
    m_sihit_barcode_decor_key = m_inputMeasurementsKey.key() + "." + m_sihit_barcode_decor_key.key();
    m_sihit_pdgid_decor_key = m_inputMeasurementsKey.key() + "." + m_sihit_pdgid_decor_key.key();

    m_sihit_startPosX_decor_key = m_inputMeasurementsKey.key() + "." + m_sihit_startPosX_decor_key.key();
    m_sihit_startPosY_decor_key = m_inputMeasurementsKey.key() + "." + m_sihit_startPosY_decor_key.key();
    m_sihit_startPosZ_decor_key = m_inputMeasurementsKey.key() + "." + m_sihit_startPosZ_decor_key.key();

    m_sihit_endPosX_decor_key = m_inputMeasurementsKey.key() + "." + m_sihit_endPosX_decor_key.key();
    m_sihit_endPosY_decor_key = m_inputMeasurementsKey.key() + "." + m_sihit_endPosY_decor_key.key();
    m_sihit_endPosZ_decor_key = m_inputMeasurementsKey.key() + "." + m_sihit_endPosZ_decor_key.key();

    ATH_CHECK( m_sihit_energyDeposit_decor_key.initialize() );
    ATH_CHECK( m_sihit_meanTime_decor_key.initialize() );
    ATH_CHECK( m_sihit_barcode_decor_key.initialize() );
    ATH_CHECK( m_sihit_pdgid_decor_key.initialize() );

    ATH_CHECK( m_sihit_startPosX_decor_key.initialize() );
    ATH_CHECK( m_sihit_startPosY_decor_key.initialize() );
    ATH_CHECK( m_sihit_startPosZ_decor_key.initialize() );

    ATH_CHECK( m_sihit_endPosX_decor_key.initialize() );
    ATH_CHECK( m_sihit_endPosY_decor_key.initialize() );
    ATH_CHECK( m_sihit_endPosZ_decor_key.initialize() );

    ATH_CHECK( detStore()->retrieve(m_StripHelper, "SCT_ID") );
    
    return StatusCode::SUCCESS;
  }

  StatusCode StripClusterSiHitDecoratorAlg::execute(const EventContext& ctx) const
  {
    ATH_MSG_DEBUG( "Executing " << name() << " ..." );

    ATH_MSG_DEBUG( "Retrieving TrackMeasurementValidationContainer with key: " << m_inputMeasurementsKey.key() );
    SG::ReadHandle< xAOD::TrackMeasurementValidationContainer > measurementHandle = SG::makeHandle( m_inputMeasurementsKey, ctx );
    ATH_CHECK( measurementHandle.isValid() );
    const xAOD::TrackMeasurementValidationContainer* measurements = measurementHandle.cptr();

    ATH_MSG_DEBUG( "Retrieving StripClusterContainer with key: " << m_inputClustersKey.key() );
    SG::ReadHandle< xAOD::StripClusterContainer > clusterHandle = SG::makeHandle( m_inputClustersKey, ctx );
    ATH_CHECK( clusterHandle.isValid() );
    const xAOD::StripClusterContainer* clusters = clusterHandle.cptr();

    ATH_MSG_DEBUG( "Retrieving InDetSimDataCollection with key: " << m_SDOcontainer_key.key() );
    SG::ReadHandle< InDetSimDataCollection > sdoHandle = SG::makeHandle( m_SDOcontainer_key, ctx );
    ATH_CHECK( sdoHandle.isValid() );
    const InDetSimDataCollection* sdos = sdoHandle.cptr();

    ATH_MSG_DEBUG("Retrieving SiHitCollection with key: " << m_siHitsKey.key());
    SG::ReadHandle< SiHitCollection > siHitsHandle = SG::makeHandle( m_siHitsKey, ctx );
    ATH_CHECK(siHitsHandle.isValid());
    const SiHitCollection* siHits = siHitsHandle.cptr();

    SG::ReadCondHandle< InDetDD::SiDetectorElementCollection > stripDetEleHandle = SG::makeHandle( m_stripDetEleCollKey, ctx );
    ATH_CHECK(stripDetEleHandle.isValid());
    const InDetDD::SiDetectorElementCollection* stripElements = stripDetEleHandle.cptr();

    // SDO decorators
    SG::WriteDecorHandle< xAOD::TrackMeasurementValidationContainer, std::vector<int> > decor_sdo_words( m_sdo_words, ctx );
    SG::WriteDecorHandle< xAOD::TrackMeasurementValidationContainer, std::vector< std::vector<int> > > decor_sdo_depositsBarcode( m_sdo_depositsBarcode, ctx );
    SG::WriteDecorHandle< xAOD::TrackMeasurementValidationContainer, std::vector< std::vector<float> > > decor_sdo_depositsEnergy( m_sdo_depositsEnergy, ctx );
    
    // SiHit decorators
    SG::WriteDecorHandle< xAOD::TrackMeasurementValidationContainer, std::vector<float> > decor_sihit_energyDeposit( m_sihit_energyDeposit_decor_key, ctx );
    SG::WriteDecorHandle< xAOD::TrackMeasurementValidationContainer, std::vector<float> > decor_sihit_meanTime( m_sihit_meanTime_decor_key, ctx );
    SG::WriteDecorHandle< xAOD::TrackMeasurementValidationContainer, std::vector<int> > decor_sihit_barcode( m_sihit_barcode_decor_key, ctx );
    SG::WriteDecorHandle< xAOD::TrackMeasurementValidationContainer, std::vector<int> > decor_sihit_pdgid( m_sihit_pdgid_decor_key, ctx );

    SG::WriteDecorHandle< xAOD::TrackMeasurementValidationContainer, std::vector<float> > decor_sihit_startPosX( m_sihit_startPosX_decor_key, ctx );
    SG::WriteDecorHandle< xAOD::TrackMeasurementValidationContainer, std::vector<float> > decor_sihit_startPosY( m_sihit_startPosY_decor_key, ctx );
    SG::WriteDecorHandle< xAOD::TrackMeasurementValidationContainer, std::vector<float> > decor_sihit_startPosZ( m_sihit_startPosZ_decor_key, ctx );
    
    SG::WriteDecorHandle< xAOD::TrackMeasurementValidationContainer, std::vector<float> > decor_sihit_endPosX( m_sihit_endPosX_decor_key, ctx );
    SG::WriteDecorHandle< xAOD::TrackMeasurementValidationContainer, std::vector<float> > decor_sihit_endPosY( m_sihit_endPosY_decor_key, ctx );
    SG::WriteDecorHandle< xAOD::TrackMeasurementValidationContainer, std::vector<float> > decor_sihit_endPosZ( m_sihit_endPosZ_decor_key, ctx );

    // measurements and clusters have the same size
    // measurement n corresponds to cluster n
    ATH_CHECK( measurements->size() == clusters->size() );

    // organize the si hits in such a way we group them together by idhash
    std::vector< std::vector< const SiHit* > > siHitsCollections(m_StripHelper->wafer_hash_max());
    for (const SiHit& siHit: *siHits) {
      if ( not siHit.isSCT() ) {
	ATH_MSG_ERROR("Si Hit in Strip collection is not Strip!!!");
	return StatusCode::FAILURE;
      }
      
      Identifier wafer_id(m_StripHelper->wafer_id(siHit.getBarrelEndcap(),
						  siHit.getLayerDisk(),
						  siHit.getPhiModule(),
						  siHit.getEtaModule(),
						  siHit.getSide()));
      IdentifierHash wafer_hash(m_StripHelper->wafer_hash(wafer_id));
      
      if (wafer_hash >= m_StripHelper->wafer_hash_max()) {
	ATH_MSG_ERROR("There is a problem with Si Hit collection.");
	ATH_MSG_ERROR("Wafer hash is too big");
	return StatusCode::FAILURE;
      }
      siHitsCollections[wafer_hash].push_back(&siHit);
    } // loop on si hits
    
    
    ContainerAccessor<xAOD::StripCluster, IdentifierHash, 1>
      stripAccessor ( *clusters,
		      [] (const xAOD::StripCluster& cl) -> IdentifierHash { return cl.identifierHash(); },
		      stripElements->size());

    // run on id hashes
    const auto& allIdHashes = stripAccessor.allIdentifiers();
    for (const auto& hashId : allIdHashes) {
      const InDetDD::SiDetectorElement *element = stripElements->getDetectorElement(hashId);
      if ( not element ) {
        ATH_MSG_FATAL( "Invalid strip detector element for hash " << hashId);
        return StatusCode::FAILURE;
      }
      
      auto [startRange, stopRange] = stripAccessor.rangesForIdentifierDirect(hashId).front();
      const std::vector< const SiHit* >& siHitsWithCurrentHash = siHitsCollections.at(hashId);

      for (auto itr = startRange; itr != stopRange; ++itr) {
	const xAOD::StripCluster* cluster = *startRange;
	const xAOD::TrackMeasurementValidation* measurement = measurements->at(cluster->index());
	ATH_CHECK(measurement->identifier() == Identifier(static_cast<int>(cluster->identifier())).get_compact() );

	auto [word, depositsBarcode, depositsEnergy] = ActsTrk::detail::getSDOInformation(cluster->rdoList(), *sdos);
	std::vector<SiHit> compatibleSiHits = findAllHitsCompatibleWithCluster(*cluster, *element, siHitsWithCurrentHash);

	auto [energyDeposit, meanTime, barcode, pdgid,
	      startPosX, startPosY, startPosZ,
	      endPosX, endPosY, endPosZ] = ActsTrk::detail::getSiHitInformation(*element, compatibleSiHits);
	
	// attach SDO decorations
	decor_sdo_words(*measurement) = std::move(word);
	decor_sdo_depositsBarcode(*measurement) = std::move(depositsBarcode);
	decor_sdo_depositsEnergy(*measurement) = std::move(depositsEnergy);

	// attach SiHit decorations
	decor_sihit_energyDeposit(*measurement) = std::move(energyDeposit);
	decor_sihit_meanTime(*measurement) = std::move(meanTime);
	decor_sihit_barcode(*measurement) = std::move(barcode);
	decor_sihit_pdgid(*measurement) = std::move(pdgid);
	
	decor_sihit_startPosX(*measurement) = std::move(startPosX);
	decor_sihit_startPosY(*measurement) = std::move(startPosY);
	decor_sihit_startPosZ(*measurement) = std::move(startPosZ);
	
	decor_sihit_endPosX(*measurement) = std::move(endPosX);
	decor_sihit_endPosY(*measurement) = std::move(endPosY);
	decor_sihit_endPosZ(*measurement) = std::move(endPosZ);
      } // loop on clusters
    } // loop on hash ids

    return StatusCode::SUCCESS;
  }

    
  std::vector<SiHit> StripClusterSiHitDecoratorAlg::findAllHitsCompatibleWithCluster( const xAOD::StripCluster& cluster,
										      const InDetDD::SiDetectorElement& element,
										      const std::vector<const SiHit*>& siHits) const
  {
    std::vector<SiHit> matchingHits {};
    std::vector<const SiHit*> multiMatchingHits {};

    for (const SiHit* siHit: siHits) {
      // Now we have all hits in the module that match lets check to see if they match the cluster
      // Must be within +/- 1 hits of any hit in the cluster to be included      
      HepGeom::Point3D<double> averagePosition = 0.5 * (siHit->localStartPosition() + siHit->localEndPosition());
      Amg::Vector2D pos = element.hitLocalToLocal(averagePosition.z(), averagePosition.y());
      InDetDD::SiCellId diode = element.cellIdOfPosition(pos);
      
      for (const auto& hitIdentifier: cluster.rdoList()) {
	ATH_MSG_DEBUG("Truth Strip " <<  diode.phiIndex() << " Cluster Strip " << m_StripHelper->strip(hitIdentifier));
	
	if (std::abs( static_cast<int>(diode.phiIndex()) - m_StripHelper->strip(hitIdentifier) ) <= 1) {
	  multiMatchingHits.push_back(siHit);
	  break;
	}
      }
    } // loop on si hits 
    
    // Now we will now make 1 SiHit for each true particle if the SiHits "touch" other
    std::vector<const SiHit* >::iterator siHitIter  = multiMatchingHits.begin();
    std::vector<const SiHit* >::iterator siHitIter2 = multiMatchingHits.begin();
    for ( ; siHitIter != multiMatchingHits.end(); ++siHitIter) {
      const SiHit* lowestXPos  = *siHitIter;
      const SiHit* highestXPos = *siHitIter;
      
      // We will merge these hits
      std::vector<const SiHit* > ajoiningHits;
      ajoiningHits.push_back( *siHitIter );
      
      siHitIter2 = siHitIter + 1;
      while ( siHitIter2 != multiMatchingHits.end() ) {
	// Need to come from the same truth particle
	if ( not HepMC::is_same_particle((*siHitIter)->particleLink(),
					 (*siHitIter2)->particleLink()) ) {
	  ++siHitIter2;
	  continue;
	}
	
	// Check to see if the SiHits are compatible with each other.
	if (std::abs((highestXPos->localEndPosition().x()-(*siHitIter2)->localStartPosition().x()))<0.00005 &&
	    std::abs((highestXPos->localEndPosition().y()-(*siHitIter2)->localStartPosition().y()))<0.00005 &&
	    std::abs((highestXPos->localEndPosition().z()-(*siHitIter2)->localStartPosition().z()))<0.00005 ) {
	  highestXPos = *siHitIter2;
	  ajoiningHits.push_back( *siHitIter2 );
	  // Dont use hit  more than once
	  // @TODO could invalidate siHitIter
	  siHitIter2 = multiMatchingHits.erase( siHitIter2 );
	} else if (std::abs((lowestXPos->localStartPosition().x()-(*siHitIter2)->localEndPosition().x()))<0.00005 &&
		   std::abs((lowestXPos->localStartPosition().y()-(*siHitIter2)->localEndPosition().y()))<0.00005 &&
		   std::abs((lowestXPos->localStartPosition().z()-(*siHitIter2)->localEndPosition().z()))<0.00005) {
	  lowestXPos = *siHitIter2;
	  ajoiningHits.push_back( *siHitIter2 );
	  // Dont use hit  more than once
	  // @TODO could invalidate siHitIter
	  siHitIter2 = multiMatchingHits.erase( siHitIter2 );
	} else {
	  ++siHitIter2;
	}	
      } // while loop
      
      if ( ajoiningHits.empty() ) {
	ATH_MSG_WARNING("This should really never happen");
	continue;
      } else if ( ajoiningHits.size() == 1 ) {
	// Copy Si Hit ready to return
	matchingHits.push_back( *ajoiningHits[0] );
	continue;
      } else {
	//  Build new SiHit and merge information together.
	ATH_MSG_DEBUG("Merging " << ajoiningHits.size() << " SiHits together." );
	
	float energyDep {0.f};
	float time {0.f};
	for( const auto& siHit : ajoiningHits ){
	  energyDep += siHit->energyLoss();
	  time += siHit->meanTime();    
	}
	time /= ajoiningHits.size();
	
	matchingHits.emplace_back(lowestXPos->localStartPosition(), 
				  highestXPos->localEndPosition(),
				  energyDep,
				  time,
				  HepMC::barcode((*siHitIter)->particleLink()),
				  1, // 0 for pixel 1 for strip
				  (*siHitIter)->getBarrelEndcap(),
				  (*siHitIter)->getLayerDisk(),
				  (*siHitIter)->getEtaModule(),
				  (*siHitIter)->getPhiModule(),
				  (*siHitIter)->getSide() );
	
	ATH_MSG_DEBUG("Finished Merging " << ajoiningHits.size() << " SiHits together." );
      }
    } // loop on multi matching hits
    
    return matchingHits;
  }

  
}

