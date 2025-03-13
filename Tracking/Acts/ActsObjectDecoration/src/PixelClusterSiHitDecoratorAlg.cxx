/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "src/PixelClusterSiHitDecoratorAlg.h"
#include "StoreGate/WriteDecorHandle.h"
#include "xAODInDetMeasurement/ContainerAccessor.h"
#include "Identifier/IdentifierHash.h"
#include "ReadoutGeometryBase/SiCellId.h"

namespace ActsTrk {

  PixelClusterSiHitDecoratorAlg::PixelClusterSiHitDecoratorAlg(const std::string &name,
							       ISvcLocator *pSvcLocator)
    : AthReentrantAlgorithm(name, pSvcLocator)
  {}

  StatusCode PixelClusterSiHitDecoratorAlg::initialize()
  {
    ATH_MSG_DEBUG( "Initializing " << name() << " ..." );

    ATH_CHECK( m_inputMeasurementsKey.initialize() );
    ATH_CHECK( m_inputClustersKey.initialize() );
    ATH_CHECK( m_SDOcontainer_key.initialize() );
    ATH_CHECK( m_pixelDetEleCollKey.initialize() );
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


    ATH_MSG_DEBUG(m_inputMeasurementsKey);
    ATH_MSG_DEBUG(m_inputClustersKey);
    ATH_MSG_DEBUG(m_SDOcontainer_key);
    ATH_MSG_DEBUG(m_pixelDetEleCollKey);
    ATH_MSG_DEBUG(m_siHitsKey);
    
    ATH_MSG_DEBUG(m_sdo_words);
    ATH_MSG_DEBUG(m_sdo_depositsBarcode);
    ATH_MSG_DEBUG(m_sdo_depositsEnergy);
  
    ATH_MSG_DEBUG(m_sihit_energyDeposit_decor_key);
    ATH_MSG_DEBUG(m_sihit_meanTime_decor_key);
    ATH_MSG_DEBUG(m_sihit_barcode_decor_key);
    ATH_MSG_DEBUG(m_sihit_pdgid_decor_key);

    ATH_MSG_DEBUG(m_sihit_startPosX_decor_key);
    ATH_MSG_DEBUG(m_sihit_startPosY_decor_key);
    ATH_MSG_DEBUG(m_sihit_startPosZ_decor_key);

    ATH_MSG_DEBUG(m_sihit_endPosX_decor_key);
    ATH_MSG_DEBUG(m_sihit_endPosY_decor_key);
    ATH_MSG_DEBUG(m_sihit_endPosZ_decor_key);

    ATH_MSG_DEBUG(m_useSiHitsGeometryMatching);

    ATH_CHECK( detStore()->retrieve(m_PixelHelper, "PixelID") );
    
    return StatusCode::SUCCESS;
  }

  StatusCode PixelClusterSiHitDecoratorAlg::execute(const EventContext& ctx) const
  {
    ATH_MSG_DEBUG( "Executing " << name() << " ..." );

    ATH_MSG_DEBUG( "Retrieving TrackMeasurementValidationContainer with key: " << m_inputMeasurementsKey.key() );
    SG::ReadHandle< xAOD::TrackMeasurementValidationContainer > measurementHandle = SG::makeHandle( m_inputMeasurementsKey, ctx );
    ATH_CHECK( measurementHandle.isValid() );
    const xAOD::TrackMeasurementValidationContainer* measurements = measurementHandle.cptr();

    ATH_MSG_DEBUG( "Retrieving PixelClusterContainer with key: " << m_inputClustersKey.key() );
    SG::ReadHandle< xAOD::PixelClusterContainer > clusterHandle = SG::makeHandle( m_inputClustersKey, ctx );
    ATH_CHECK( clusterHandle.isValid() );
    const xAOD::PixelClusterContainer* clusters = clusterHandle.cptr();

    ATH_MSG_DEBUG( "Retrieving InDetSimDataCollection with key: " << m_SDOcontainer_key.key() );
    SG::ReadHandle< InDetSimDataCollection > sdoHandle = SG::makeHandle( m_SDOcontainer_key, ctx );
    ATH_CHECK( sdoHandle.isValid() );
    const InDetSimDataCollection* sdos = sdoHandle.cptr();

    ATH_MSG_DEBUG("Retrieving SiHitCollection with key: " << m_siHitsKey.key());
    SG::ReadHandle< SiHitCollection > siHitsHandle = SG::makeHandle( m_siHitsKey, ctx );
    ATH_CHECK(siHitsHandle.isValid());
    const SiHitCollection* siHits = siHitsHandle.cptr();

    SG::ReadCondHandle< InDetDD::SiDetectorElementCollection > pixelDetEleHandle = SG::makeHandle( m_pixelDetEleCollKey, ctx );
    ATH_CHECK(pixelDetEleHandle.isValid());
    const InDetDD::SiDetectorElementCollection* pixElements = pixelDetEleHandle.cptr();

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
    std::vector< std::vector< const SiHit* > > siHitsCollections(m_PixelHelper->wafer_hash_max());
    for (const SiHit& siHit: *siHits) {
      if (!siHit.isPixel()) {
	ATH_MSG_ERROR("Si Hit in Pixel collection is not Pixel!!!");
	return StatusCode::FAILURE;
      }
      
      Identifier wafer_id(m_PixelHelper->wafer_id(siHit.getBarrelEndcap(),
						  siHit.getLayerDisk(),
						  siHit.getPhiModule(),
						  siHit.getEtaModule()));
      IdentifierHash wafer_hash(m_PixelHelper->wafer_hash(wafer_id));
      
      if (wafer_hash >= m_PixelHelper->wafer_hash_max()) {
	ATH_MSG_ERROR("There is a problem with Si Hit collection.");
	ATH_MSG_ERROR("Wafer hash is too big");
	return StatusCode::FAILURE;
      }
      siHitsCollections[wafer_hash].push_back(&siHit);
    } // loop on si hits
    
    
    ContainerAccessor<xAOD::PixelCluster, IdentifierHash, 1>
      pixelAccessor ( *clusters,
		      [] (const xAOD::PixelCluster& cl) -> IdentifierHash { return cl.identifierHash(); },
		      pixElements->size());

    // run on id hashes
    const auto& allIdHashes = pixelAccessor.allIdentifiers();
    for (const auto& hashId : allIdHashes) {
      const InDetDD::SiDetectorElement *element = pixElements->getDetectorElement(hashId);
      if ( not element ) {
        ATH_MSG_FATAL( "Invalid pixel detector element for hash " << hashId);
        return StatusCode::FAILURE;
      }
      
      auto [startRange, stopRange] = pixelAccessor.rangesForIdentifierDirect(hashId).front();
      const std::vector< const SiHit* >& siHitsWithCurrentHash = siHitsCollections.at(hashId);

      for (auto itr = startRange; itr != stopRange; ++itr) {
	const xAOD::PixelCluster* cluster = *startRange;
	const xAOD::TrackMeasurementValidation* measurement = measurements->at(cluster->index());
	ATH_CHECK(measurement->identifier() == Identifier(static_cast<int>(cluster->identifier())).get_compact() );

	auto [word, depositsBarcode, depositsEnergy] = getSDOInformation(*cluster, *sdos);
	std::vector<SiHit> compatibleSiHits = findAllHitsCompatibleWithCluster(*cluster, *element, siHitsWithCurrentHash, depositsBarcode);

	auto [energyDeposit, meanTime, barcode, pdgid,
	      startPosX, startPosY, startPosZ,
	      endPosX, endPosY, endPosZ] = getSiHitInformation(*element, compatibleSiHits);
	
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


  typename PixelClusterSiHitDecoratorAlg::sdo_info_t
  PixelClusterSiHitDecoratorAlg::getSDOInformation( const xAOD::PixelCluster& cluster,
						    const InDetSimDataCollection& sdoCollection ) const
  {
    std::vector<int> sdo_word {};
    std::vector< std::vector< int > > sdo_depositsBarcode {};
    std::vector< std::vector< float > > sdo_depositsEnergy {};

    const std::vector<Identifier>& rdoList = cluster.rdoList();
    for (const Identifier hitIdentifier : rdoList) {
      auto pos = sdoCollection.find(hitIdentifier);
      if( pos == sdoCollection.end() ) continue;

      sdo_word.push_back( pos->second.word() ) ;

      std::vector<int> sdoDepBC(pos->second.getdeposits().size(), HepMC::INVALID_PARTICLE_ID);
      std::vector<float> sdoDepEnergy(pos->second.getdeposits().size());

      unsigned int nDepos {0};
      for (auto& deposit: pos->second.getdeposits()) {
	if (deposit.first) sdoDepBC[nDepos] = HepMC::barcode(deposit.first);
	ATH_MSG_DEBUG(" SDO Energy Deposit " << deposit.second  ) ;
	sdoDepEnergy[nDepos] = deposit.second;
	++nDepos;
      }

      sdo_depositsBarcode.push_back( std::move(sdoDepBC) );
      sdo_depositsEnergy.push_back( std::move(sdoDepEnergy) );
    }
    
    return std::make_tuple(std::move(sdo_word),
                           std::move(sdo_depositsBarcode),
                           std::move(sdo_depositsEnergy));
  }

  typename PixelClusterSiHitDecoratorAlg::sihit_info_t
  PixelClusterSiHitDecoratorAlg::getSiHitInformation( const InDetDD::SiDetectorElement& element,
						      const std::vector<SiHit> & matchingHits ) const
  {
    int numHits = matchingHits.size();
    
    std::vector<float> sihit_energyDeposit(numHits, 0);
    std::vector<float> sihit_meanTime(numHits, 0);
    std::vector<int>   sihit_barcode(numHits, 0);
    std::vector<int>   sihit_pdgid(numHits, 0);
    
    std::vector<float> sihit_startPosX(numHits, 0);
    std::vector<float> sihit_startPosY(numHits, 0);
    std::vector<float> sihit_startPosZ(numHits, 0);
    
    std::vector<float> sihit_endPosX(numHits, 0);
    std::vector<float> sihit_endPosY(numHits, 0);
    std::vector<float> sihit_endPosZ(numHits, 0);

    int hitNumber {0};
    for ( const SiHit& sihit : matchingHits ) {          
      sihit_energyDeposit[hitNumber] =  sihit.energyLoss() ;
      sihit_meanTime[hitNumber] =  sihit.meanTime() ;
      
      const HepMcParticleLink& HMPL = sihit.particleLink();
      sihit_barcode[hitNumber] = HepMC::barcode(HMPL) ;
      if( HMPL.isValid() ){
        sihit_pdgid[hitNumber] = HMPL->pdg_id();
      }
      
      // Convert Simulation frame into reco frame
      const HepGeom::Point3D<double>& startPos=sihit.localStartPosition();
      
      Amg::Vector2D pos = element.hitLocalToLocal( startPos.z(), startPos.y() );
      sihit_startPosX[hitNumber] =  pos[0];
      sihit_startPosY[hitNumber] =  pos[1];
      sihit_startPosZ[hitNumber] =  startPos.x();

      const HepGeom::Point3D<double>& endPos=sihit.localEndPosition();
      pos = element.hitLocalToLocal( endPos.z(), endPos.y() );
      sihit_endPosX[hitNumber] =  pos[0];
      sihit_endPosY[hitNumber] =  pos[1];
      sihit_endPosZ[hitNumber] =  endPos.x();
      ++hitNumber;
    }

    return std::make_tuple(std::move(sihit_energyDeposit), std::move(sihit_meanTime), std::move(sihit_barcode), std::move(sihit_pdgid),
			   std::move(sihit_startPosX), std::move(sihit_startPosY), std::move(sihit_startPosZ),
			   std::move(sihit_endPosX), std::move(sihit_endPosY), std::move(sihit_endPosZ));
  }
  
  std::vector<SiHit> PixelClusterSiHitDecoratorAlg::findAllHitsCompatibleWithCluster( const xAOD::PixelCluster& cluster,
										      const InDetDD::SiDetectorElement& element,
										      const std::vector<const SiHit*>& sihits,
										      const std::vector< std::vector< int > >& sdoTracks) const
  {
    std::vector<SiHit> matchingHits {};
    std::vector<const SiHit*> multiMatchingHits {};

    for ( const SiHit* siHit : sihits) {
      // Now we have all hits in the module that match lets check to see if they match the cluster
      // Must be within +/- 1 hits of any hit in the cluster to be included
      if ( m_useSiHitsGeometryMatching ) {
	HepGeom::Point3D<double>  averagePosition =  0.5 * (siHit->localStartPosition() + siHit->localEndPosition());
	Amg::Vector2D pos = element.hitLocalToLocal( averagePosition.z(), averagePosition.y() );
	InDetDD::SiCellId diode = element.cellIdOfPosition(pos);
	
	for( const auto& hitIdentifier : cluster.rdoList() ){
	    ATH_MSG_DEBUG("Truth Phi " <<  diode.phiIndex() << " Cluster Phi " <<   m_PixelHelper->phi_index( hitIdentifier ) );
	    ATH_MSG_DEBUG("Truth Eta " <<  diode.etaIndex() << " Cluster Eta " <<   m_PixelHelper->eta_index( hitIdentifier ) );
	    if( std::abs( static_cast<int>(diode.etaIndex()) - m_PixelHelper->eta_index( hitIdentifier ) ) <= 1 and
		std::abs( static_cast<int>(diode.phiIndex()) - m_PixelHelper->phi_index( hitIdentifier ) ) <= 1 ) {
	      multiMatchingHits.push_back(siHit);
	      break;
	    }
	} // list on rdos
	
      } else { // not m_useSiHitsGeometryMatching
	auto siHitBarcode = HepMC::barcode(siHit->particleLink());       
	for ( const std::vector<int>& barcodeSDOColl : sdoTracks ) {
	  if (std::find(barcodeSDOColl.begin(), barcodeSDOColl.end(), siHitBarcode) == barcodeSDOColl.end()) continue;
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
      }else if ( ajoiningHits.size() == 1 ) {
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
				  0,
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

