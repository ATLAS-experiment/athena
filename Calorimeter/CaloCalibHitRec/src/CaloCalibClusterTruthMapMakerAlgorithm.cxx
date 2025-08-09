/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "CaloCalibClusterTruthMapMakerAlgorithm.h"

StatusCode CaloCalibClusterTruthMapMakerAlgorithm::initialize(){

  ATH_CHECK(m_tileActiveCaloCalibrationHitReadHandleKey.initialize());
  ATH_CHECK(m_tileInactiveCaloCalibrationHitReadHandleKey.initialize());
  ATH_CHECK(m_tileDMCaloCalibrationHitReadHandleKey.initialize());

  ATH_CHECK(m_lArActiveCaloCalibrationHitReadHandleKey.initialize());
  ATH_CHECK(m_lArInactiveCaloCalibrationHitReadHandleKey.initialize());
  ATH_CHECK(m_lArDMCaloCalibrationHitReadHandleKey.initialize());

  ATH_CHECK(m_truthParticleReadHandleKey.initialize());

  ATH_CHECK(m_mapIdentifierToCalibHitsWriteHandleKey.initialize());

  return StatusCode::SUCCESS;

}

StatusCode CaloCalibClusterTruthMapMakerAlgorithm::execute(const EventContext& ctx) const{

  SG::WriteHandle<std::map<Identifier,std::vector<const CaloCalibrationHit*> > > mapIdentifierToCalibHitsWriteHandle(m_mapIdentifierToCalibHitsWriteHandleKey,ctx);
  ATH_CHECK(mapIdentifierToCalibHitsWriteHandle.record(std::make_unique<std::map<Identifier,std::vector<const CaloCalibrationHit*> > >()));
  fillIdentifierToCaloHitMap(*mapIdentifierToCalibHitsWriteHandle, ctx);
  
  return StatusCode::SUCCESS;
}


StatusCode CaloCalibClusterTruthMapMakerAlgorithm::finalize(){return StatusCode::SUCCESS;}

void CaloCalibClusterTruthMapMakerAlgorithm::fillIdentifierToCaloHitMap(std::map<Identifier,std::vector<const CaloCalibrationHit*> >& identifierToCaloHitMap, const EventContext& ctx) const{

  //get calibration hit containers and add them to a vector

  std::vector<SG::ReadHandle<CaloCalibrationHitContainer> > calibrationHitReadHandles;

  {
    SG::ReadHandle<CaloCalibrationHitContainer> tileActiveCaloCalibrationHitReadHandle(m_tileActiveCaloCalibrationHitReadHandleKey,ctx);
    if (!tileActiveCaloCalibrationHitReadHandle.isValid()) ATH_MSG_WARNING("Could not retrieve CaloCalibrationHitContainer with key " << tileActiveCaloCalibrationHitReadHandle.key());
    else calibrationHitReadHandles.push_back(std::move(tileActiveCaloCalibrationHitReadHandle));
  }

  {
    SG::ReadHandle<CaloCalibrationHitContainer> tileInactiveCaloCalibrationHitReadHandle(m_tileInactiveCaloCalibrationHitReadHandleKey,ctx);
    if (!tileInactiveCaloCalibrationHitReadHandle.isValid()) ATH_MSG_WARNING("Could not retrieve CaloCalibrationHitContainer with key " << tileInactiveCaloCalibrationHitReadHandle.key());
    else calibrationHitReadHandles.push_back(std::move(tileInactiveCaloCalibrationHitReadHandle));
  }

  {
    SG::ReadHandle<CaloCalibrationHitContainer> tileDMCaloCalibrationHitReadHandle(m_tileDMCaloCalibrationHitReadHandleKey,ctx);
    if (!tileDMCaloCalibrationHitReadHandle.isValid()) ATH_MSG_WARNING("Could not retrieve CaloCalibrationHitContainer with key " << tileDMCaloCalibrationHitReadHandle.key());

    else calibrationHitReadHandles.push_back(std::move(tileDMCaloCalibrationHitReadHandle));
  }

  {
    SG::ReadHandle<CaloCalibrationHitContainer> lArActiveCaloCalibrationHitReadHandle(m_lArActiveCaloCalibrationHitReadHandleKey,ctx);
    if (!lArActiveCaloCalibrationHitReadHandle.isValid()) ATH_MSG_WARNING("Could not retrieve CaloCalibrationHitContainer with key " << lArActiveCaloCalibrationHitReadHandle.key());
    else calibrationHitReadHandles.push_back(std::move(lArActiveCaloCalibrationHitReadHandle));
  }

  {
    SG::ReadHandle<CaloCalibrationHitContainer> lArInactiveCaloCalibrationHitReadHandle(m_lArInactiveCaloCalibrationHitReadHandleKey,ctx);
    if (!lArInactiveCaloCalibrationHitReadHandle.isValid()) ATH_MSG_WARNING("Could not retrieve CaloCalibrationHitContainer with key " << lArInactiveCaloCalibrationHitReadHandle.key());
    else calibrationHitReadHandles.push_back(std::move(lArInactiveCaloCalibrationHitReadHandle));
  }

  {
    SG::ReadHandle<CaloCalibrationHitContainer> lArDMCaloCalibrationHitReadHandle(m_lArDMCaloCalibrationHitReadHandleKey,ctx);
    if (!lArDMCaloCalibrationHitReadHandle.isValid()) ATH_MSG_WARNING("Could not retrieve CaloCalibrationHitContainer with key " << lArDMCaloCalibrationHitReadHandle.key());
    else calibrationHitReadHandles.push_back(std::move(lArDMCaloCalibrationHitReadHandle));
  }
  
  for (auto& thisCalibrationHitReadHandle : calibrationHitReadHandles){
    for (const auto *thisCalibrationHit : *thisCalibrationHitReadHandle){

      if (!thisCalibrationHit) {
        ATH_MSG_WARNING("Got invalid pointer to CaloCalibrationHit in container with key :" << thisCalibrationHitReadHandle.key());
        continue;
      }

      //fill the map
      Identifier thisIdentifier = thisCalibrationHit->cellID();
      //count returns 1 if the key exists, otherwise it returns 0
      int count = identifierToCaloHitMap.count(thisIdentifier);
      if (0 == count) identifierToCaloHitMap[thisIdentifier] = std::vector<const CaloCalibrationHit*>{thisCalibrationHit};
      else identifierToCaloHitMap[thisIdentifier].push_back(thisCalibrationHit);
      
    }//loop on calibration hits in a container
  }//loop over calibration hit containers
  
}
