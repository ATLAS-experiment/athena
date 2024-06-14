/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 DB data - Muon Station components
 -----------------------------------------
 ***************************************************************************/

#include "MuonGMdbObjects/DblQ00Wrpc.h"
#include "RDBAccessSvc/IRDBAccessSvc.h"
#include "RDBAccessSvc/IRDBRecordset.h"
#include "RDBAccessSvc/IRDBRecord.h"

#include <iostream>
#include <sstream>

namespace MuonGM {

  DblQ00Wrpc::DblQ00Wrpc(IRDBAccessSvc *pAccessSvc, const std::string & GeoTag, const std::string & GeoNode) {
  IRDBRecordset_ptr wrpc = pAccessSvc->getRecordsetPtr(getName(),GeoTag, GeoNode);
  if(wrpc->size()>0) {
    m_nObj = wrpc->size();
    m_d.resize (m_nObj);
    if (m_nObj == 0) std::cerr<<"NO Wrpc banks in the MuonDD Database"<<std::endl;

    for(size_t i =0; i<wrpc->size();++i) {
        m_d[i].version     = (*wrpc)[i]->getInt("VERS");    
        m_d[i].nvrs        = (*wrpc)[i]->getInt("NVRS");
        m_d[i].layrpc      = (*wrpc)[i]->getInt("LAYRPC");
        m_d[i].tckrla      = (*wrpc)[i]->getFloat("TCKRLA");
        m_d[i].tottck      = (*wrpc)[i]->getFloat("TOTTCK");
        m_d[i].tckfsp      = (*wrpc)[i]->getFloat("TCKFSP");
        m_d[i].ackfsp      = (*wrpc)[i]->getFloat("ACKFSP");
        m_d[i].tlohcb      = (*wrpc)[i]->getFloat("TLOHCB");
        m_d[i].alohcb      = (*wrpc)[i]->getFloat("ALOHCB");
        m_d[i].tckbak      = (*wrpc)[i]->getFloat("TCKBAK");
        m_d[i].tckgas      = (*wrpc)[i]->getFloat("TCKGAS");
        m_d[i].tckssu      = (*wrpc)[i]->getFloat("TCKSSU");
        m_d[i].tckstr      = (*wrpc)[i]->getFloat("TCKSTR");
        m_d[i].sdedmi      = (*wrpc)[i]->getFloat("SDEDMI");
        m_d[i].zdedmi      = (*wrpc)[i]->getFloat("ZDEDMI");
        m_d[i].spdiam      = (*wrpc)[i]->getFloat("SPDIAM");
        m_d[i].sppitc      = (*wrpc)[i]->getFloat("SPPITC");
        m_d[i].stroff[0]   = (*wrpc)[i]->getFloat("STROFF_0");
        m_d[i].stroff[1]   = (*wrpc)[i]->getFloat("STROFF_1");
        m_d[i].stroff[2]   = (*wrpc)[i]->getFloat("STROFF_2");
    }
  }
  else {
    std::cerr<<"NO Wrpc banks in the MuonDD Database"<<std::endl;
  }
}

} // end of namespace MuonGM
