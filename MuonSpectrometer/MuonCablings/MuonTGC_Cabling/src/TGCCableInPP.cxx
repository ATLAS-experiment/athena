/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonTGC_Cabling/TGCCableInPP.h"

#include "MuonTGC_Cabling/TGCDatabaseInPP.h" 
#include "MuonTGC_Cabling/TGCChannelPPIn.h"
#include "MuonTGC_Cabling/TGCChannelPPOut.h"
#include "MuonTGC_Cabling/TGCChannelSLBIn.h"

namespace MuonTGC_Cabling {

// Constructor & Destructor
TGCCableInPP::TGCCableInPP(const std::string& filename)
  : TGCCable(TGCCable::InPP) {
  m_database[TGCId::Endcap][TGCId::WD] = std::make_unique<TGCDatabaseInPP>(filename, "EWD");
  m_database[TGCId::Endcap][TGCId::WT] = std::make_unique<TGCDatabaseInPP>(filename, "EWT");
  m_database[TGCId::Endcap][TGCId::SD] = std::make_unique<TGCDatabaseInPP>(filename, "ESD");
  m_database[TGCId::Endcap][TGCId::ST] = std::make_unique<TGCDatabaseInPP>(filename, "EST");
  m_database[TGCId::Endcap][TGCId::WI] = std::make_unique<TGCDatabaseInPP>(filename, "EWI");
  m_database[TGCId::Endcap][TGCId::SI] = std::make_unique<TGCDatabaseInPP>(filename, "ESI");
  m_database[TGCId::Forward][TGCId::WD] = std::make_unique<TGCDatabaseInPP>(filename, "FWD");
  m_database[TGCId::Forward][TGCId::WT] = std::make_unique<TGCDatabaseInPP>(filename, "FWT");
  m_database[TGCId::Forward][TGCId::SD] = std::make_unique<TGCDatabaseInPP>(filename, "FSD");
  m_database[TGCId::Forward][TGCId::ST] = std::make_unique<TGCDatabaseInPP>(filename, "FST");
  m_database[TGCId::Forward][TGCId::WI] = std::make_unique<TGCDatabaseInPP>(filename, "FWI");
  m_database[TGCId::Forward][TGCId::SI] = std::make_unique<TGCDatabaseInPP>(filename, "FSI");
}
  

TGCChannelId* TGCCableInPP::getChannel(const TGCChannelId* channelId,
                                       const bool orChannel) const {
  if(channelId){
    if(channelId->getChannelIdType()==TGCChannelId::ChannelIdType::PPIn)
      return getChannelOut(channelId, orChannel);
    if(channelId->getChannelIdType()==TGCChannelId::ChannelIdType::PPOut)
      return getChannelIn(channelId, orChannel);
  }
  return nullptr;
}
  
TGCChannelId* TGCCableInPP::getChannelIn(const TGCChannelId* ppout,
                                         const bool orChannel) const {
  if(ppout->isValid()==false) return nullptr;

  TGCId::ModuleType moduleType = ppout->getModuleType();

  int ndatabaseP = 1;
  TGCDatabase* databaseP[2];
  databaseP[0] = m_database[ppout->getRegionType()][moduleType].get();
  // EI/FI
  //  wire(TGCId::WI) and strip(TGCId::SI) of a chamber
  //  use the same SLB chip
  //  The SLB chip is treated as TGCId::WI in TGCCableSLBToSSW.cxx
  if(moduleType==TGCId::WI) {
    databaseP[1] = m_database[ppout->getRegionType()][TGCId::SI].get();
    ndatabaseP = 2;
  }
  
  int id=-1, block=-1, channel=-1;
  bool found = false;
  
  for(int idatabaseP=0; idatabaseP<ndatabaseP; idatabaseP++) {
    // EI/FI
    //  wire(TGCId::WI) and strip(TGCId::SI) of a chamber
    //  use the same SLB chip
    //  The SLB chip is treated as TGCId::WI in TGCCableSLBToSSW.cxx
    if(idatabaseP==1) {
      moduleType = TGCId::SI;
    }

    int indexIn[TGCDatabaseInPP::NIndexIn] = 
      {ppout->getId(), ppout->getBlock(), ppout->getChannel()};
    int i = databaseP[idatabaseP]->getIndexDBIn(indexIn);
    if(i<0) continue;
    
    if(orChannel==false){
      // first channel
      id = databaseP[idatabaseP]->getEntry(i,3);
      block = databaseP[idatabaseP]->getEntry(i,4);
      channel = databaseP[idatabaseP]->getEntry(i,5);
      if(id==-1 && block==-1 && channel==-1) continue;
      found = true; 
      break;
    } else {
      // ored channel
      if(databaseP[idatabaseP]->getEntrySize(i)==9) {
        id = databaseP[idatabaseP]->getEntry(i,6);
        block = databaseP[idatabaseP]->getEntry(i,7);
        channel = databaseP[idatabaseP]->getEntry(i,8);
        found = true; 
      } 
    }
  } 
  
  if(!found) return nullptr;
  
  TGCChannelPPIn* ppin = 
    new TGCChannelPPIn(ppout->getSideType(),
		       moduleType,
		       ppout->getRegionType(),
		       ppout->getSector(),
		       id,
		       block,
		       channel);
  
  return ppin;
}

TGCChannelId* TGCCableInPP::getChannelOut(const TGCChannelId* ppin,
                                          const bool orChannel) const {
  if(ppin->isValid()==false) return nullptr;

  const int ppinChannel = ppin->getChannel();
  const int ppinBlock = ppin->getBlock();
  const int ppinId = ppin->getId();

  TGCDatabase* databaseP =
    m_database[ppin->getRegionType()][ppin->getModuleType()].get();

  TGCChannelPPOut* ppout = nullptr;
  const int MaxEntry = databaseP->getMaxEntry();
  for(int i=0; i<MaxEntry; i++){
    bool cond1 = (databaseP->getEntry(i,5)==ppinChannel)&& 
      (databaseP->getEntry(i,4)==ppinBlock)&&
      (databaseP->getEntry(i,3)==ppinId);
      
    bool cond2 = (databaseP->getEntrySize(i)==9)&&
      (databaseP->getEntry(i,8)==ppinChannel)&&
      (databaseP->getEntry(i,7)==ppinBlock)&&
      (databaseP->getEntry(i,6)==ppinId);
                 
    if(cond1 || cond2) {
      int id = databaseP->getEntry(i,0);
      int block = databaseP->getEntry(i,1);
      int channel = databaseP->getEntry(i,2);

      //TGCChannelSLBIn::CellType cellType = TGCChannelSLBIn::NoCellType;
      int channelInSLB = -1;
      bool adjacent = false;
      TGCId::ModuleType moduleType = ppin->getModuleType();
      if(block==0||block==2){//C,D
	int lengthOfC = TGCChannelSLBIn::getLengthOfSLB(moduleType,
							TGCChannelSLBIn::CellC);
	int lengthOfD = TGCChannelSLBIn::getLengthOfSLB(moduleType,
							TGCChannelSLBIn::CellD);
	if(channel<lengthOfD){
	  int adjacentOfD = TGCChannelSLBIn::getAdjacentOfSLB(moduleType,
							      TGCChannelSLBIn::CellD);
	  //cellType = TGCChannelSLBIn::CellD;
	  channelInSLB = channel;
	  if(channelInSLB<adjacentOfD||channelInSLB>=lengthOfD-adjacentOfD)
	    adjacent = true;
	} else {
	  int adjacentOfC = TGCChannelSLBIn::getAdjacentOfSLB(moduleType,
							      TGCChannelSLBIn::CellC);
	  //cellType = TGCChannelSLBIn::CellC;
	  channelInSLB = channel-lengthOfD;
	  if(channelInSLB<adjacentOfC||channelInSLB>=lengthOfC-adjacentOfC)
	    adjacent = true;
	}
      }
      if(block==1||block==3){//A,B
	int lengthOfA = TGCChannelSLBIn::getLengthOfSLB(moduleType,
							TGCChannelSLBIn::CellA);
	int lengthOfB = TGCChannelSLBIn::getLengthOfSLB(moduleType,
							TGCChannelSLBIn::CellB);
	if(channel<lengthOfB){
	  int adjacentOfB = TGCChannelSLBIn::getAdjacentOfSLB(moduleType,
							      TGCChannelSLBIn::CellB);
	  //cellType = TGCChannelSLBIn::CellB;
	  channelInSLB = channel;
	  if(channelInSLB<adjacentOfB||channelInSLB>=lengthOfB-adjacentOfB)
	    adjacent = true;
	} else {
	  int adjacentOfA = TGCChannelSLBIn::getAdjacentOfSLB(moduleType,
							      TGCChannelSLBIn::CellA);
	  //cellType = TGCChannelSLBIn::CellA;
	  channelInSLB = channel-lengthOfB;
	  if(channelInSLB<adjacentOfA||channelInSLB>=lengthOfA-adjacentOfA)
	    adjacent = true;
	}
      }
       
      if((moduleType == TGCId::SD)  &&
	 (ppin->getRegionType() == TGCId::Endcap)){
	// Strips of Middle doublets are ORed to the adjacent chamber  
	adjacent = cond2;
      } 

      if(adjacent==orChannel){
	ppout = new TGCChannelPPOut(ppin->getSideType(),
				    ppin->getModuleType(),
				    ppin->getRegionType(),
				    ppin->getSector(),
				    id,
				    block,
				    channel);
	break;
      }
    } 
  }
  
  return ppout;
}  

} //end of namespace
