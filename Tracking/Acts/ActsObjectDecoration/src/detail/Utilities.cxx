/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "src/detail/Utilities.h"

namespace ActsTrk::detail {
  
  std::tuple<std::vector<int>,
	     std::vector< std::vector< int > >,
	     std::vector< std::vector< float > >
	     >
  getSDOInformation( const std::vector<Identifier>& rdoList,
		     const InDetSimDataCollection& sdoCollection )
  {
    std::vector<int> sdo_word {};
    std::vector< std::vector< int > > sdo_depositsBarcode {};
    std::vector< std::vector< float > > sdo_depositsEnergy {};

    for (const Identifier hitIdentifier : rdoList) {
      auto pos = sdoCollection.find(hitIdentifier);
      if( pos == sdoCollection.end() ) continue;

      sdo_word.push_back( pos->second.word() ) ;

      std::vector<int> sdoDepBC(pos->second.getdeposits().size(), HepMC::INVALID_PARTICLE_ID);
      std::vector<float> sdoDepEnergy(pos->second.getdeposits().size());

      unsigned int nDepos {0};
      for (const auto& deposit: pos->second.getdeposits()) {
	if (deposit.first) sdoDepBC[nDepos] = HepMC::barcode(deposit.first); // FIXME barcode-based
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
  
  
  std::tuple<std::vector<float>,
	     std::vector<float>,
	     std::vector<int>,
	     std::vector<int>,
	     std::vector<float>,
	     std::vector<float>,
	     std::vector<float>,
	     std::vector<float>,
	     std::vector<float>,
	     std::vector<float>
	     >
  getSiHitInformation( const InDetDD::SiDetectorElement& element,
                       const std::vector<SiHit>& matchingHits )
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
      sihit_barcode[hitNumber] = HepMC::barcode(HMPL); // FIXME barcode-based
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
  
}

