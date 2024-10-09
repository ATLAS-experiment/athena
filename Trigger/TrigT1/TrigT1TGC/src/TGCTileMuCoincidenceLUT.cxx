/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigT1TGC/TGCTileMuCoincidenceLUT.h"

#include "AthenaKernel/getMessageSvc.h"
#include "PathResolver/PathResolver.h"

#include <fstream>

namespace LVL1TGC {

TGCTileMuCoincidenceLUT::TGCTileMuCoincidenceLUT(LVL1TGCTrigger::TGCArguments* tgcargs,
   		  	                         const SG::ReadCondHandleKey<TGCTriggerData>& readCondKey,
                                                 const std::string& version)
 : m_verName(version),
   m_tgcArgs(tgcargs),
   m_readCondKey(readCondKey)
{
  if (!tgcArgs()->TILE_MU()) return;
  if (tgcArgs()->USE_CONDDB()) return;

  MsgStream log(Athena::getMessageSvc(), "LVL1TGC::TGCTileMuCoincidenceLUT");

  // read Inner Coincidence Map 
  if (this->readMap()) {
    log << MSG::INFO 
      << " TGC TileMu CW version of " << m_verName << " is selected " << endmsg;
  } else {
    log << MSG::INFO  
	<< " NOT use TileMu " << endmsg;
    m_flagpt.clear();
    m_flagroi.clear();
    m_trigbit.clear();
  }
}


TGCTileMuCoincidenceLUT::~TGCTileMuCoincidenceLUT()
{
}

bool TGCTileMuCoincidenceLUT::readMap() 
{
  MsgStream log(Athena::getMessageSvc(), "LVL1TGC::TGCTileMuCoincidenceLUT");

  // select right database according to a set of thresholds
  std::string dbname="";
  dbname = "TileMuCoincidenceMap." + m_verName + "._12.db";

  //----- 
  
  std::string fullName;
  fullName = PathResolver::FindCalibDirectory("dev")+"/TrigT1TGC/TILE/"+dbname;
  bool isFound =( fullName.length() > 0 );
  if( !isFound) {
    log << MSG::WARNING 
	<< " Could not found " << dbname << endmsg;
    return false ;  
  } 

  std::ifstream file(fullName.c_str(),std::ios::in);    

  static constexpr unsigned int BufferSize = 256;
  char buf[BufferSize];

  while (file.getline(buf, BufferSize)) {

    int16_t sideId = -1;
    int16_t sectorId = -1;
    int16_t sscId    = -1;

    std::istringstream header(buf);
    std::string tag;
    header >> tag;
    if(tag == "#") {
      header >> sideId >> sectorId >> sscId;
    }

    // check Id
    if(sideId < 0   || sideId>=TGCTriggerData::N_SIDE ||
       sectorId < 0 || sectorId>=TGCTriggerData::N_ENDCAP_SECTOR ||
       sscId < 0    || sscId>=TGCTriggerData::N_ENDCAP_SSC ) {
      log << MSG::WARNING
          << " illegal parameter in database header : " << header.str()
          << " in file " << dbname
          << endmsg;
      file.close();
      return false;
    }

    uint16_t addr = this->getAddr(sideId, sectorId, sscId);

    uint8_t flagpt = 0;
    for (size_t pt = 0; pt < TGCTriggerData::N_PT_THRESH; pt++){
      uint8_t use;
      header >> use;
      flagpt |= (use&0x1)<<pt;
    }
    m_flagpt[addr] = flagpt;

    uint8_t roi = 0;
    for (size_t pos=0; pos< TGCTriggerData::N_ROI_IN_SSC; pos++){
      uint8_t use;
      header >> use;
      roi |= (use&0x1)<<pos;
    }
    m_flagroi[addr] = roi;

    // get trigger word
    file.getline(buf,BufferSize);
    std::istringstream cont(buf);
    uint16_t trigbit = 0x0;
    for(size_t pos=0; pos < TGCTriggerData::N_TILE_INPUT; pos++){
        uint16_t word;
        cont >> word;
        trigbit |= (word & 0xf)<<(pos*4);
    }
    m_trigbit[addr] = trigbit;

  }
  file.close();	  

  return true;
}


int TGCTileMuCoincidenceLUT::getTrigMask(const int module,
                                         const int16_t ssc,
                                         const int16_t sec,
                                         const int16_t side) const
{
  if ((module<0)||(module>=TGCTriggerData::N_TILE_INPUT)) return TM_NA;

  uint16_t mask = 0x0;
  if  (tgcArgs()->USE_CONDDB()) {
    SG::ReadCondHandle<TGCTriggerData> readHandle{m_readCondKey};
    const TGCTriggerData* readCdo{*readHandle};
    mask = readCdo->getTrigMaskTile(ssc, sec, side);
  } else {
    uint16_t addr = this->getAddr(side, sec, ssc);
    std::unordered_map<uint16_t, uint16_t>::const_iterator it = m_trigbit.find(addr);
    if(it != m_trigbit.end()) mask = it->second;
  }

  return mask>>(module*4) & 0x7;
}

int TGCTileMuCoincidenceLUT::getFlagPT(const int pt,
                                       const int16_t ssc,
                                       const int16_t sec,
                                       const int16_t side) const
{
  if ((pt<=0)||(pt>TGCTriggerData::N_PT_THRESH)) return -1;

  uint8_t ptmask = 0x0;
  if  (tgcArgs()->USE_CONDDB()) {
    SG::ReadCondHandle<TGCTriggerData> readHandle{m_readCondKey};
    const TGCTriggerData* readCdo{*readHandle};
    ptmask = readCdo->getFlagPtTile(ssc, sec, side);
  } else {
    uint16_t addr = this->getAddr(side, sec, ssc);
    std::unordered_map<uint16_t, uint8_t>::const_iterator it = m_flagpt.find(addr);
    if(it != m_flagpt.end()) ptmask = it->second;
  }
  return (ptmask>>(pt-1)) & 0x1;  /* only 1st bit needed (0x1) */
}

int TGCTileMuCoincidenceLUT::getFlagROI(const int roi,
                                        const int16_t ssc,
                                        const int16_t sec,
                                        const int16_t side) const
{
  if ((roi<0)||(roi>=TGCTriggerData::N_ROI_IN_SSC)) return -1;

  uint8_t roimask = 0x0;
  if  (tgcArgs()->USE_CONDDB()) {
    SG::ReadCondHandle<TGCTriggerData> readHandle{m_readCondKey};
    const TGCTriggerData* readCdo{*readHandle};
    roimask = readCdo->getFlagRoiTile(ssc, sec, side);
  } else {
    uint16_t addr = this->getAddr(side, sec, ssc);
    std::unordered_map<uint16_t, uint8_t>::const_iterator it = m_flagroi.find(addr);
    if(it != m_flagroi.end()) roimask = it->second;
  }
  return (roimask >> roi) & 0x1;  /* only 1st bit needed (0x1) */
}

uint16_t TGCTileMuCoincidenceLUT::getAddr(int16_t side, int16_t sec, int16_t ssc) const
{
  return ((side & TGCTriggerData::SIDE_MASK)<<TGCTriggerData::ADDR_SIDE_SHIFT) +
         ((sec & TGCTriggerData::SECTOR_MASK)<<TGCTriggerData::ADDR_SECTOR_SHIFT) +
         (ssc & TGCTriggerData::SSC_MASK);
}


} //end of namespace bracket
