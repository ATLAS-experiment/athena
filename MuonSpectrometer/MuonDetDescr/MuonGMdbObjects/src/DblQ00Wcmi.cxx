/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 DB data - Muon Station components
 -----------------------------------------
 ***************************************************************************/

#include "MuonGMdbObjects/DblQ00Wcmi.h"
#include "RDBAccessSvc/IRDBRecordset.h"
#include "RDBAccessSvc/IRDBAccessSvc.h"
#include "RDBAccessSvc/IRDBRecord.h"

#include <iostream>
#include <sstream>
#include <string> 
namespace MuonGM {

  DblQ00Wcmi::DblQ00Wcmi(IRDBAccessSvc *pAccessSvc, const std::string & GeoTag, const std::string & GeoNode){

    IRDBRecordset_ptr wcmi = pAccessSvc->getRecordsetPtr(getName(),GeoTag, GeoNode);

    if(wcmi->size()>0) {
      m_nObj = wcmi->size();
      m_d.resize (m_nObj);
      if (m_nObj == 0) std::cerr<<"NO Wcmi banks in the MuonDD Database"<<std::endl;

      for(size_t i=0;i<wcmi->size(); ++i) {
          m_d[i].version     = (*wcmi)[i]->getInt("VERS");    
          m_d[i].jsta        = (*wcmi)[i]->getInt("JSTA");
          m_d[i].num         = (*wcmi)[i]->getInt("NUM");
          m_d[i].heightness  = (*wcmi)[i]->getFloat("HEIGHTNESS");
          m_d[i].largeness   = (*wcmi)[i]->getFloat("LARGENESS");
          m_d[i].thickness   = (*wcmi)[i]->getFloat("THICKNESS");
      }
  } else {
    std::cerr<<"NO Wcmi banks in the MuonDD Database"<<std::endl;
  }
}

} // end of namespace MuonGM
