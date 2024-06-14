/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 DB data - Muon Station components
 -----------------------------------------
 ***************************************************************************/

#include "MuonGMdbObjects/DblQ00Acut.h"
#include "RDBAccessSvc/IRDBRecordset.h"
#include "RDBAccessSvc/IRDBAccessSvc.h"
#include "RDBAccessSvc/IRDBRecord.h"
#include <iostream>
#include <stdexcept>

namespace MuonGM
{

  DblQ00Acut::DblQ00Acut(IRDBAccessSvc *pAccessSvc, const std::string & GeoTag, const std::string & GeoNode):
    m_nObj(0) {

    IRDBRecordset_ptr acut = pAccessSvc->getRecordsetPtr(getName(),GeoTag, GeoNode);

    if(acut->size()>0) {
    m_nObj = acut->size();
    m_d.resize (m_nObj);
    if (m_nObj == 0) std::cerr<<"NO Acut banks in the MuonDD Database"<<std::endl;

    for(size_t i=0; i <acut->size(); ++i) {
      m_d[i].version = (*acut)[i]->getInt("VERS");
      m_d[i].i       = (*acut)[i]->getInt("I");
      m_d[i].icut    = (*acut)[i]->getInt("ICUT");
      m_d[i].n       = (*acut)[i]->getInt("N");
    }
  }
  else {
    std::cerr<<"NO Acut banks in the MuonDD Database"<<std::endl;
  }
}

} // end of namespace MuonGM
