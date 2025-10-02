/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonTGC_Cabling/TGCCableInASD.h"

#include "MuonTGC_Cabling/TGCDatabaseASDToPP.h" 
#include "MuonTGC_Cabling/TGCChannelASDIn.h"
#include "MuonTGC_Cabling/TGCChannelASDOut.h"

namespace MuonTGC_Cabling {

// Constructor & Destructor
TGCCableInASD::TGCCableInASD(const std::string& filename)
  : TGCCable(TGCCable::InASD) {
  m_database[TGCId::Endcap][TGCId::WD] = std::make_unique<TGCDatabaseASDToPP>(filename,"EWD");
  m_database[TGCId::Endcap][TGCId::WT] = std::make_unique<TGCDatabaseASDToPP>(filename,"EWT");
  m_database[TGCId::Endcap][TGCId::SD] = std::make_unique<TGCDatabaseASDToPP>(filename,"ESD");
  m_database[TGCId::Endcap][TGCId::ST] = std::make_unique<TGCDatabaseASDToPP>(filename,"EST");
  m_database[TGCId::Endcap][TGCId::WI] = std::make_unique<TGCDatabaseASDToPP>(filename,"EWI");
  m_database[TGCId::Endcap][TGCId::SI] = std::make_unique<TGCDatabaseASDToPP>(filename,"ESI");
  m_database[TGCId::Forward][TGCId::WD] = std::make_unique<TGCDatabaseASDToPP>(filename,"FWD");
  m_database[TGCId::Forward][TGCId::WT] = std::make_unique<TGCDatabaseASDToPP>(filename,"FWT");
  m_database[TGCId::Forward][TGCId::SD] = std::make_unique<TGCDatabaseASDToPP>(filename,"FSD");
  m_database[TGCId::Forward][TGCId::ST] = std::make_unique<TGCDatabaseASDToPP>(filename,"FST");
  m_database[TGCId::Forward][TGCId::WI] = std::make_unique<TGCDatabaseASDToPP>(filename,"FWI");
  m_database[TGCId::Forward][TGCId::SI] = std::make_unique<TGCDatabaseASDToPP>(filename,"FSI");
}
  
TGCChannelId* TGCCableInASD::getChannel(const TGCChannelId* channelId,
					bool orChannel) const {
  if(channelId){
    if(channelId->getChannelIdType()==TGCChannelId::ChannelIdType::ASDIn)
      return getChannelOut(channelId,orChannel);
    if(channelId->getChannelIdType()==TGCChannelId::ChannelIdType::ASDOut)
      return getChannelIn(channelId,orChannel);
  }
  return nullptr;
}

TGCChannelId*TGCCableInASD::getChannelIn(const TGCChannelId* asdout,
					 bool orChannel) const {
  if(orChannel) return nullptr;
  if(asdout->isValid()==false) return nullptr;

  TGCDatabase* databaseP = m_database[asdout->getRegionType()][asdout->getModuleType()].get();
  
  TGCChannelASDIn* asdin = nullptr;

  // sector ASDIn [1..48, 1..24], ASDOut [0..47, 0..23]
  int sector;
  if(asdout->isEndcap() && !asdout->isInner()){
    sector = asdout->getSector()-1;
    if(sector<=0) sector += TGCId::NUM_ENDCAP_SECTOR;
  } else {
    sector = asdout->getSector();
    if(sector<=0) sector += TGCId::NUM_FORWARD_SECTOR;
  }

  // chamber ASDIn [1(F),1,2,3,4,5(E)], ASDOut [0(F),4,3,2,1,0(E)]
  int chamber;
  if(asdout->isEndcap() && !asdout->isInner()){
    chamber = 5-asdout->getChamber();
  } else {
    chamber = asdout->getChamber()+1;
  }
  
  int channel=-1;
  // channel ASDIn [1..32(S),1..n(W chamber)], ASDOut [0..31(S),n..0(W sector)]
  if(asdout->isWire()){
    // Endcap Triplet chamberId start from 1 in ASDOut 
    int dbChamber = asdout->getChamber(); 
    if(asdout->isEndcap() && asdout->isTriplet()) dbChamber = dbChamber-1; 
    int indexIn[TGCDatabaseASDToPP::NIndexIn] = 
      {asdout->getLayer(), dbChamber, asdout->getChannel()}; 
    int i = databaseP->getIndexDBIn(indexIn); 
    if(i<0) return nullptr; 
    channel = databaseP->getEntry(i,7)+1; 
  } else {
    if(( asdout->isBackward() && asdout->isAside()) ||
       (!asdout->isBackward() && asdout->isCside()))
      channel = 32-asdout->getChannel();
    else
      channel = asdout->getChannel()+1;
  }
  if(channel==-1) return nullptr;

  asdin = new TGCChannelASDIn(asdout->getSideType(),
			      asdout->getSignalType(),
			      asdout->getRegionType(),
			      sector,
			      asdout->getLayer(),
			      chamber,
			      channel);

  return asdin;
}

TGCChannelId* TGCCableInASD::getChannelOut(const TGCChannelId* asdin,
					   bool orChannel) const {
  if(orChannel) return nullptr;
  if(asdin->isValid()==false) return nullptr;

  const bool asdinisEndcap = asdin->isEndcap();
  const bool asdinisTriplet = asdin->isTriplet();
  const int asdinLayer = asdin->getLayer();
  const int asdinChannel = asdin->getChannel();

  TGCDatabase* databaseP = m_database[asdin->getRegionType()][asdin->getModuleType()].get();
  
  TGCChannelASDOut* asdout = nullptr;

  // sector ASDIn [1..48, 1..24], ASDOut [2..47.0.1, 1..23.0]
  int sector;
  if(asdin->isEndcap()) {
   if(!asdin->isInner()){
      // Endcap
      sector = (asdin->getSector()+1) % TGCId::NUM_ENDCAP_SECTOR;
    } else {
      // EI
       sector = (asdin->getSector()) % TGCId::NUM_INNER_SECTOR;
    } 
  } else {
   if(!asdin->isInner()){
      // Forward
      sector = (asdin->getSector()) % TGCId::NUM_FORWARD_SECTOR;
    } else {
      // FI 
      sector = (asdin->getSector()) % TGCId::NUM_INNER_SECTOR;
    }
  }
  
  // chamber ASDIn [1(F),1,2,3,4,5(E)], ASDOut [0(F),4,3,2,1,0(E)]
  int chamber;
  if(asdin->isEndcap()&&!asdin->isInner()){
    chamber = 5-asdin->getChamber();
  } else {
    chamber = asdin->getChamber()-1;
  }
  
  int channel=-1;
  // channel ASDIn [1..32(S),1..n(W chamber)], ASDOut [0..31(S),n..0(W sector)]
  if(asdin->isWire()){
    const int MaxEntry = databaseP->getMaxEntry();
    for(int i=0; i<MaxEntry; i++){
      // Endcap Triplet chamberId start from 1 in ASDOut
      int dbChamber = chamber;
      if(asdinisEndcap&&asdinisTriplet)
	dbChamber = dbChamber-1;
      
      int dbChannel = asdinChannel-1;
      if(databaseP->getEntry(i,7)==dbChannel &&
         databaseP->getEntry(i,1)==dbChamber &&
         databaseP->getEntry(i,0)==asdinLayer) {
        channel = databaseP->getEntry(i,6);
        break;
      }
    }
  } else {
    // asdin->isBackward() can not be used because this method rely on sector number for asdout
    bool is_Backward = false; 
    if(asdin->isEndcap()){
      if(!asdin->isInner()) { 
        if(asdin->isAside()) is_Backward = (sector%2==1);
        else                 is_Backward = (sector%2==0); 
      } else {
        // EI
        // Special case of EI11
        if(sector == 15) {
          if(asdin->isAside()) is_Backward = false;
          else                 is_Backward = true; 
        } else if(sector == 16) {
	  if(asdin->isAside()) is_Backward = true;
	  else                 is_Backward = false; 
        } else {
	  //  A-side phi0 F: phi1 F: phi2 B
	  //  C-side phi0 B: phi1 B: phi2 F
	  if(asdin->isAside())  is_Backward = (sector%3==2);
	  else                  is_Backward = (sector%3!=2);
        }
      }
    } else {
      if(asdin->isAside()) is_Backward = true; //All Backward for A-side
    }
    if(( is_Backward && asdin->isAside()) ||
       (!is_Backward && asdin->isCside()))
      channel = 32-asdin->getChannel();
    else
      channel = asdin->getChannel()-1;
  }
  if(channel==-1) return nullptr;

  asdout = new TGCChannelASDOut(asdin->getSideType(),
				asdin->getSignalType(),
				asdin->getRegionType(),
				sector,
				asdin->getLayer(),
				chamber,
				channel);

  return asdout;
}  


} //end of namespace
