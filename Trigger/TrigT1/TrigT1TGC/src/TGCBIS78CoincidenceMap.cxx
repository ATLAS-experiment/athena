/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/


#include <fstream>
#include <sstream>
#include <string>

#include "TrigT1TGC/TGCBIS78CoincidenceMap.h"
#include "TrigT1TGC/BIS78TrigOut.h"
#include "TrigT1TGC/TGCDatabaseManager.h"
#include "PathResolver/PathResolver.h"

#include "TrigT1TGC/TGCArguments.h"

namespace LVL1TGC {

TGCBIS78CoincidenceMap::TGCBIS78CoincidenceMap(LVL1TGCTrigger::TGCArguments* tgcargs, const std::string& version)
: AthMessaging("TGCBIS78CoincidenceMap"),
  m_verName(version),
  m_tgcArgs(tgcargs)
{
  setLevel(tgcArgs()->MSGLEVEL());

  if(!tgcArgs()->USE_BIS78()){return;}

  // initialize map
  for (size_t sec=0; sec < kNEndcapTrigSector; sec++){
    for (size_t ssc=0; ssc < kNMaxSSC; ssc++){
      for (size_t pos=0; pos < kNRoiInSSC; pos++){
        m_flagROI[pos][ssc][sec] = 1;
      }
    }
  }

  for (size_t dr=0; dr!=N_DETA; dr++) {
    for (size_t dphi=0; dphi!=N_DPHI; dphi++) {
      m_CW[dr][dphi].resize(kNumberOfEndcapRoI);
      std::fill(m_CW[dr][dphi].begin(), m_CW[dr][dphi].end(), 0);
    }
  }

  //---------Read out CW data---------
  if (!this->readMap()) {
    // BIS78 trigger flag is set to false when the map reading failed.
    tgcArgs()->set_USE_BIS78(false);
    ATH_MSG_INFO("NOT using BIS78");
  } else {
    ATH_MSG_INFO("TGC BIS78 CW version of " << m_verName << " is selected");
  }
}


//TGC-BIS78 Eta-Phi Coincidence
int TGCBIS78CoincidenceMap::TGCBIS78_pt(const BIS78TrigOut *bis78Out,int /*roi*/) const
{
    std::vector<uint8_t> bis78Eta_vec=bis78Out->getBIS78eta();
    std::vector<uint8_t> bis78Phi_vec=bis78Out->getBIS78phi();
    std::vector<uint8_t> bis78Deta_vec=bis78Out->getBIS78Deta();
    std::vector<uint8_t> bis78Dphi_vec=bis78Out->getBIS78Dphi();
    std::vector<uint8_t> bis78flag3over3eta_vec=bis78Out->getBIS78flag3over3eta();
    std::vector<uint8_t> bis78flag3over3phi_vec=bis78Out->getBIS78flag3over3phi();
    
    int pt=0;
    // temporal algorithm
    for(unsigned int bis78hit_id=0; bis78hit_id!=bis78Eta_vec.size(); bis78hit_id++){
      // calculate dR, dPhi bin # from eta/phiIndex?
      // for now, if there is a hit at BIS78, it returns true always
      pt=1;
    }
    
    return pt;
}

int TGCBIS78CoincidenceMap::getFlagROI(const unsigned int roi,
                                       const unsigned int ssc,
                                       const unsigned int sec,
                                       const unsigned int side) const
{
  if (side != TGCSide::ASIDE) return 0;   // BIS78 only in A side

  if (roi >= kNRoiInSSC) return -1;
  if (ssc >= kNMaxSSC) return 0;
  if (sec >= kNEndcapTrigSector) return -1;

  // which inner LUT will be implemented
  return m_flagROI[roi][ssc][sec];
}

bool TGCBIS78CoincidenceMap::readMap() 
{
  // no LUT available for TGC-BIS78 coincidence
  // just return true for now
    
  return true;
} 
  
}  // end of namespace
