/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration 
*/

#include "TrigT1TGC/TGCGoodMF.h"
#include "TrigT1TGC/TGCNumbering.h"
#include "PathResolver/PathResolver.h"

#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/IMessageSvc.h"

#include "TrigT1TGC/TGCArguments.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

namespace LVL1TGC {

TGCGoodMF::TGCGoodMF(LVL1TGCTrigger::TGCArguments* tgcargs, const std::string& version)
: AthMessaging("LVL1TGC::TGCGoodMF"),
  m_tgcArgs(tgcargs),
  m_verName(version)
{
  if (this->readBadMFList()) {
    ATH_MSG_INFO("LVL1TGC Hot-ROI mask version of " << m_verName << " is selected");
  } else {
    ATH_MSG_INFO("LVL1TGC Hot-ROI mask NOT defined");
  }

}

const TGCGoodMF& TGCGoodMF::operator=(const TGCGoodMF& right)
{
  if(this != &right){
    m_tgcArgs=right.m_tgcArgs;
    m_verName=right.m_verName;
    this->readBadMFList();
  }

  return *this;
}

bool TGCGoodMF::readBadMFList()
{
  //initialize
  std::string buf;
  std::string fullName, tag;    char delimiter = '\n';
  std::string fn = "/HotRoI/HotRoI."+m_verName+".db";

  fullName = PathResolver::FindCalibDirectory("dev")+"/TrigT1TGC"+fn;
  int mod,maxssc;
  std::ifstream file(fullName.c_str(),std::ios::in); 
  while(getline(file,buf,delimiter)) {
    std::istringstream header(buf);
    header>>tag;
    if(tag == "#"){ // read header part.
      header>>mod>>maxssc;

      // get BadMF list
      getline(file,buf,delimiter);
      std::istringstream cont(buf);
      std::map<int,int>& mapssc = m_mapisgoodMF[mod]; // mapssc = <SSCId,badMFId>, m_mapisgoodMF = <ModuleId,<~>>
      int badMFId;
      //coverity[TAINTED_SCALAR]
      for(int i=0; i<=maxssc; i++) {
        cont>>badMFId;
        if(badMFId < 0) continue;
        mapssc[i] = badMFId;
      }
    }
  }
  return true;
}

bool TGCGoodMF::test_GoodMF(int moduleId,int sscId,int RoI) const
{
  std::map<int, std::map<int, int> >::const_iterator itModule=m_mapisgoodMF.find(moduleId);
  if (itModule==m_mapisgoodMF.end()) return true;

  const std::map<int, int> &mapssc = itModule->second;
  std::map<int, int>::const_iterator itSSC=mapssc.find(sscId);
  if (itSSC==mapssc.end()) return true;

  int badMFId = itSSC->second;
  if (badMFId==0){ return true; }

  std::bitset<kNRoiInSSC> bs(badMFId);

  int RoIId;
  if(RoI % kNRoiInSSC >= 4){ RoIId = RoI % kNRoiInSSC - 4; }
  else{ RoIId = RoI % kNRoiInSSC + 4; }
  
  return !bs[RoIId];
}


} //end of namespace bracket
