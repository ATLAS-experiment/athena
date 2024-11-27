/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "CaloCalibHitRec/CaloCalibClusterTruthAttributerTool.h"

CaloCalibClusterTruthAttributerTool::CaloCalibClusterTruthAttributerTool(const std::string& type, const std::string& name,  const IInterface* parent) : base_class(type,name,parent) {
}

CaloCalibClusterTruthAttributerTool::~CaloCalibClusterTruthAttributerTool()= default;

StatusCode CaloCalibClusterTruthAttributerTool::calculateTruthEnergies(const xAOD::CaloCluster& theCaloCluster, unsigned int numTruthParticles, const std::map<Identifier,std::vector<const CaloCalibrationHit*> >& identifierToCaloHitMap, std::vector<std::pair<unsigned int, double > >& truthIDTrueCalHitEnergy) const{

  ATH_MSG_DEBUG("In calculateTruthEnergies");

  const CaloClusterCellLink* theCellLinks = theCaloCluster.getCellLinks();

  if (!theCellLinks) {
    ATH_MSG_ERROR("A CaloCluster has no CaloClusterCellLinks");
    return StatusCode::FAILURE;
  }  
  
  std::map<unsigned int, double> truthIDTruePtMap;

  //Loop on calorimeter cells to sum up the truth energies of the truth particles.    
  for (const auto *thisCaloCell : *theCellLinks){
    
    if (!thisCaloCell){
      ATH_MSG_WARNING("Have invalid pointer to CaloCell");
      continue;
    }

    //get the unique calorimeter cell identifier
    Identifier cellID = thisCaloCell->ID();
    
    //look up the calibration hit that corresponds to this calorimeter cell - we use find because not all calorimeter cells will have calibration hits
    std::map<Identifier,std::vector<const CaloCalibrationHit*> >::const_iterator identifierToCaloHitMapIterator = identifierToCaloHitMap.find(cellID);
    if (identifierToCaloHitMap.end() == identifierToCaloHitMapIterator) continue;
    std::vector<const CaloCalibrationHit*> theseCalibrationHits = (*identifierToCaloHitMapIterator).second;

    for (const auto *thisCalibrationHit : theseCalibrationHits){
      const int truthID = HepMC::uniqueID(thisCalibrationHit);
      double thisCalHitTruthEnergy = thisCalibrationHit->energyEM() + thisCalibrationHit->energyNonEM();
      if (true == m_fullTruthEnergy) thisCalHitTruthEnergy += (thisCalibrationHit->energyEscaped() + thisCalibrationHit->energyInvisible());

      auto iterator = truthIDTruePtMap.find(truthID);
      if (iterator != truthIDTruePtMap.end()) truthIDTruePtMap[truthID] += thisCalHitTruthEnergy;
      else truthIDTruePtMap[truthID] = thisCalHitTruthEnergy;
      
    }//calibration hit loop
    
  }//loop on calorimeter cells to sum up truth energies

  //now create a vector with the same information as the map, which we can then sort
  std::vector<std::pair<unsigned int, double > > truthIDTruePtPairs;

  truthIDTruePtPairs.reserve(truthIDTruePtMap.size());
  for (const auto& thisEntry : truthIDTruePtMap) truthIDTruePtPairs.emplace_back(thisEntry);

  //sort vector by calibration hit truth energy
  std::sort(truthIDTruePtPairs.begin(),truthIDTruePtPairs.end(),[]( std::pair<unsigned int, double> a, std::pair<unsigned int, double> b) -> bool {return a.second > b.second;} );

  //store the truthID and truth energy of the top numTruthParticles truth particles
  if (numTruthParticles > truthIDTruePtPairs.size()) numTruthParticles = truthIDTruePtPairs.size();
  for ( unsigned int counter = 0; counter < numTruthParticles; counter++) truthIDTrueCalHitEnergy.push_back(truthIDTruePtPairs[counter]);

  for (const auto& thisPair : truthIDTrueCalHitEnergy) ATH_MSG_DEBUG("Truncated loop 2: truthID and true energy are " << thisPair.first << " and " << thisPair.second << " for cluster with e, eta of " << theCaloCluster.e() << " and " << theCaloCluster.eta() );

  return StatusCode::SUCCESS;
  
}
