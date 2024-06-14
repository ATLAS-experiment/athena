/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 DB data - Muon Station components
 -----------------------------------------
 ***************************************************************************/

#include "MuonGMdbObjects/DblQ00Wsup.h"
#include "RDBAccessSvc/IRDBAccessSvc.h"
#include "RDBAccessSvc/IRDBRecordset.h"
#include "RDBAccessSvc/IRDBRecord.h"
#include <iostream>
#include <sstream>

namespace MuonGM
{

  DblQ00Wsup::DblQ00Wsup(IRDBAccessSvc *pAccessSvc, const std::string & GeoTag, const std::string & GeoNode) {

    IRDBRecordset_ptr wsup = pAccessSvc->getRecordsetPtr(getName(),GeoTag, GeoNode);

    if(wsup->size()>0) {
      m_nObj = wsup->size();
      m_d.resize (m_nObj);
      if (m_nObj == 0) std::cerr<<"NO Wsup banks in the MuonDD Database"<<std::endl;

      for(size_t i=0 ; i<wsup->size(); ++i) {
        m_d[i].version     = (*wsup)[i]->getInt("VERS");    
        m_d[i].jsta        = (*wsup)[i]->getInt("JSTA");
        m_d[i].nxxsup      = (*wsup)[i]->getInt("NXXSUP");
        m_d[i].nzzsup      = (*wsup)[i]->getInt("NZZSUP");
        m_d[i].x0          = (*wsup)[i]->getFloat("X0");
        m_d[i].thickn      = (*wsup)[i]->getFloat("THICKN");
        for (unsigned int j=0; j<4; ++j){
            m_d[i].xxsup[j]     = (*wsup)[i]->getFloat("XXSUP_"+std::to_string(j));        
            m_d[i].zzsup[j]     = (*wsup)[i]->getFloat("ZZSUP_"+std::to_string(j));        
        }
    }
  }
  else {
    std::cerr<<"NO Wsup banks in the MuonDD Database"<<std::endl;
  }
}

} // end of namespace MuonGM
