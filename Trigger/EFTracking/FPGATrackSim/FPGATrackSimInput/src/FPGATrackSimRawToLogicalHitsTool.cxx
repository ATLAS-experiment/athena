/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "FPGATrackSimInput/FPGATrackSimRawToLogicalHitsTool.h"
#include "FPGATrackSimSGInput/FPGATrackSimInputUtils.h"

#include "FPGATrackSimObjects/FPGATrackSimEventInputHeader.h"
#include "FPGATrackSimObjects/FPGATrackSimLogicalEventInputHeader.h"
#include "FPGATrackSimObjects/FPGATrackSimTowerInputHeader.h"
#include "FPGATrackSimObjects/FPGATrackSimEventInfo.h"
#include "FPGATrackSimObjects/FPGATrackSimOptionalEventInfo.h"
#include "FPGATrackSimObjects/FPGATrackSimOfflineTrack.h"
#include "FPGATrackSimObjects/FPGATrackSimOfflineHit.h"
#include "FPGATrackSimObjects/FPGATrackSimTruthTrack.h"

#include "FPGATrackSimMaps/FPGATrackSimRegionMap.h"
#include "FPGATrackSimMaps/FPGATrackSimPlaneMap.h"
#include "FPGATrackSimMaps/IFPGATrackSimMappingSvc.h"



FPGATrackSimRawToLogicalHitsTool::FPGATrackSimRawToLogicalHitsTool(const std::string& algname, const std::string &name, const IInterface *ifc) :
  AthAlgTool(algname,name,ifc){}


StatusCode FPGATrackSimRawToLogicalHitsTool::initialize(){
  ATH_CHECK(m_FPGATrackSimMapping.retrieve());
  ATH_CHECK(m_EvtSel.retrieve());

  ATH_MSG_INFO("SaveOptional="<<m_saveOptional);
  // remove deferensed m_towersToMap.value() in newer releases!
  if( m_towersToMap.value().empty() ){
    int maxNtowers = m_FPGATrackSimMapping->RegionMap_2nd()->getNRegions();
    for (int ireg=0;ireg!=maxNtowers;++ireg) m_towers.push_back(ireg);
  }else{
    m_towers = m_towersToMap.value();
  }

  ATH_MSG_DEBUG ("Configured to process "<<m_towers.size() << " towers");
  std::stringstream listOfTowers;
  for (int ireg: m_towers){
    listOfTowers<<", "<<ireg;
  }
  ATH_MSG_DEBUG ("List of Towers: "<<listOfTowers.str());
  return StatusCode::SUCCESS;
}


StatusCode FPGATrackSimRawToLogicalHitsTool::convert(unsigned stage, const FPGATrackSimEventInputHeader& eventHeader, FPGATrackSimLogicalEventInputHeader& logicEventHeader){

  ATH_MSG_DEBUG ("Mapping " << eventHeader.nHits() << " hits using stage " << stage);
  FPGATrackSimEventInfo eventinfo = eventHeader.event();
  ATH_MSG_DEBUG ("Getting Event " << eventinfo);
  logicEventHeader.newEvent(eventinfo);//this also reset all varaibles

  if(stage!=1 && stage!=2)
  {
      ATH_MSG_FATAL("convert() must have stage == 1 or 2");
  }

  const FPGATrackSimPlaneMap*  pmap = m_FPGATrackSimMapping->PlaneMap_2nd(0);
  
  logicEventHeader.reserveTowers(m_towers.size());
  for (int ireg: m_towers){
    FPGATrackSimTowerInputHeader tower = FPGATrackSimTowerInputHeader(ireg);//default header, can eventually set eta/phi/deta/dphi
    logicEventHeader.addTower( tower);
  }
  for (FPGATrackSimHit hit: eventHeader.hits()) { // hit loop
      // In the ITk geometry, some of the plane IDs are -1 if the layers are not yet being used.
      // This causes the code in this hit loop to crash. As a workaround for the moment, we currently
      // skip over hits in layers that are not included in the FPGATrackSim geometry, with plane = -1

      pmap->doRemap(hit);

    for (unsigned int ireg = 0; ireg != m_towers.size(); ++ireg) {
      logicEventHeader.getTower(ireg)->addHit(hit);
      ATH_MSG_VERBOSE("Hit mapped (" << hit.isMapped() << ") to tower=" << ireg << ", nHits now=" << logicEventHeader.getTower(ireg)->nHits());
    }
  } // end hit loop
  
  if (stage == 1) {
    FPGATrackSimOptionalEventInfo op = eventHeader.optional();
    if (m_saveOptional == 2) {
      logicEventHeader.setOptional(op);
    }
    else if (m_saveOptional == 1) {
      // save offline tracks  
      FPGATrackSimOptionalEventInfo newop;
      newop.reserveOfflineTracks(op.nOfflineTracks());
      for (FPGATrackSimOfflineTrack const & offline_t : op.getOfflineTracks()) newop.addOfflineTrack(offline_t);
      // save truth in region
      for (FPGATrackSimTruthTrack const & truth_t : op.getTruthTracks()) {
        if (m_EvtSel->passMatching(truth_t)) newop.addTruthTrack(truth_t);
      }
      ATH_MSG_DEBUG("Selected " << newop.nTruthTracks() << " truth tracks");
      logicEventHeader.setOptional(newop);
    }
  }

  return StatusCode::SUCCESS;
}

const FPGATrackSimPlaneMap* FPGATrackSimRawToLogicalHitsTool::getPlaneMap_1st(int sliceNum) {
  return m_FPGATrackSimMapping->PlaneMap_1st(sliceNum);
}

