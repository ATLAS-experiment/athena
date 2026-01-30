/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonTGC_Cabling/TGCCableHPBToSL.h"

#include "MuonTGC_Cabling/TGCDatabasePPToSL.h" 
#include "MuonTGC_Cabling/TGCModuleHPB.h"
#include "MuonTGC_Cabling/TGCModuleSL.h"

namespace MuonTGC_Cabling {

TGCCableHPBToSL::TGCCableHPBToSL(const std::string& filename)
  : TGCCable(TGCCable::HPBToSL),
    m_database{{{nullptr}}}
{
  m_database[TGCId::Endcap][TGCId::Wire]   = std::make_unique<TGCDatabasePPToSL>(filename,"HPB EW");
  m_database[TGCId::Endcap][TGCId::Strip]  = std::make_unique<TGCDatabasePPToSL>(filename,"HPB ES");
  m_database[TGCId::Forward][TGCId::Wire]  = std::make_unique<TGCDatabasePPToSL>(filename,"HPB FW");
  m_database[TGCId::Forward][TGCId::Strip] = std::make_unique<TGCDatabasePPToSL>(filename,"HPB FS");
}

TGCCableHPBToSL::~TGCCableHPBToSL() = default;

TGCModuleMap* TGCCableHPBToSL::getModule(const TGCModuleId* moduleId) const {
  if(moduleId){
    if(moduleId->getModuleIdType()==TGCModuleId::HPB)
      return getModuleOut(moduleId);
    if(moduleId->getModuleIdType()==TGCModuleId::SL)
      return getModuleIn(moduleId);
  }
  return nullptr;
}

TGCModuleMap* TGCCableHPBToSL::getModuleIn(const TGCModuleId* sl) const {
  if(sl->isValid()==false) return nullptr;  

  TGCDatabase* wireP = m_database[sl->getRegionType()][TGCId::Wire].get();
  TGCDatabase* stripP = m_database[sl->getRegionType()][TGCId::Strip].get();

  TGCModuleMap* mapId = nullptr;
  const int wireMaxEntry = wireP->getMaxEntry();
  for(int i=0; i<wireMaxEntry; i++){
    int id = wireP->getEntry(i,0);
    int block = wireP->getEntry(i,1);
    TGCModuleHPB* hpb = new TGCModuleHPB(sl->getSideType(),
					 TGCId::Wire,
					 sl->getRegionType(),
					 sl->getSector(),
					 id);
    if(mapId==nullptr) mapId = new TGCModuleMap();
    mapId->insert(block,hpb);
  } 

  const int stripMaxEntry = stripP->getMaxEntry();
  for(int i=0; i<stripMaxEntry; i++){
    int id = stripP->getEntry(i,0);
    int block = stripP->getEntry(i,1);
    TGCModuleHPB* hpb = new TGCModuleHPB(sl->getSideType(),
					 TGCId::Strip,
					 sl->getRegionType(),
					 sl->getSector(),
					 id);
    if(mapId==nullptr) mapId = new TGCModuleMap();
    mapId->insert(block,hpb);	
  }

  return mapId;
}

TGCModuleMap* TGCCableHPBToSL::getModuleOut(const TGCModuleId* hpb) const {
  if(hpb->isValid()==false) return nullptr;  

  const int hpbId = hpb->getId();

  TGCDatabase* databaseP = m_database[hpb->getRegionType()][hpb->getSignalType()].get();

  TGCModuleMap* mapId = nullptr;
  const int MaxEntry = databaseP->getMaxEntry();
  for(int i=0; i<MaxEntry; i++){
    if(databaseP->getEntry(i,0)==hpbId)
      {
	int block = databaseP->getEntry(i,1);
	TGCModuleSL* sl = new TGCModuleSL(hpb->getSideType(),
					  hpb->getRegionType(),
					  hpb->getSector());

	mapId = new TGCModuleMap();
	mapId->insert(block,sl);
	break;
      } 
  }
 
  return mapId;
}
  
} //end of namespace
