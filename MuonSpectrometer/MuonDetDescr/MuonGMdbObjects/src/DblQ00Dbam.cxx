/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 DB data - Muon Station components
 -----------------------------------------
 ***************************************************************************/

#include "MuonGMdbObjects/DblQ00Dbam.h"
#include "RDBAccessSvc/IRDBRecordset.h"
#include "RDBAccessSvc/IRDBAccessSvc.h"
#include "RDBAccessSvc/IRDBRecord.h"
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdexcept>

namespace MuonGM
{

  DblQ00Dbam::DblQ00Dbam(IRDBAccessSvc *pAccessSvc, const std::string & GeoTag, const std::string & GeoNode) {

    IRDBRecordset_ptr dbam = pAccessSvc->getRecordsetPtr(getName(),GeoTag, GeoNode);

    if(dbam->size()>0) {
      m_nObj = dbam->size();
      m_d.resize (m_nObj);
      if (m_nObj == 0) std::cerr<<"NO Dbam banks in the MuonDD Database"<<std::endl;

      for(size_t i =0; i<dbam->size(); ++i) {
        m_d[i].version     = (*dbam)[i]->getInt("VERS");    
        m_d[i].nvrs        = (*dbam)[i]->getInt("NVRS");
        m_d[i].mtyp        = (*dbam)[i]->getInt("MTYP");
        m_d[i].numbox      = (*dbam)[i]->getInt("NUMBOX");
        m_d[i].amdb         =(*dbam)[i]->getString("AMDB"); 
      
        for (unsigned int j=0; j<53; j++) {
            try {
                m_d[i].name[j] = (*dbam)[i]->getString("NAME_"+std::to_string(j));
            } catch (const std::runtime_error&) {
                break;
            }
        }
    }
  }
  else {
    std::cerr<<"NO Dbam banks in the MuonDD Database"<<std::endl;
  }
}


} // end of namespace MuonGM
