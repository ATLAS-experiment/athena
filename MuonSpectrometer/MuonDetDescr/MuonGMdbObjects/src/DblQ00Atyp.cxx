/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 DB data - Muon Station components
 -----------------------------------------
 ***************************************************************************/

#include "MuonGMdbObjects/DblQ00Atyp.h"
#include "RDBAccessSvc/IRDBRecordset.h"
#include "RDBAccessSvc/IRDBAccessSvc.h"
#include "RDBAccessSvc/IRDBRecord.h"

#include <iostream>
#include <stdio.h>

namespace MuonGM
{

  DblQ00Atyp::DblQ00Atyp(IRDBAccessSvc *pAccessSvc, const std::string & GeoTag, const std::string & GeoNode) {

  IRDBRecordset_ptr atyp = pAccessSvc->getRecordsetPtr(getName(),GeoTag, GeoNode);

  if(atyp->size()!=0) {
    m_nObj = atyp->size();
    m_d.resize (m_nObj);
    if (m_nObj == 0) std::cerr<<"NO Atyp banks in the MuonDD Database"<<std::endl;

    for (size_t i=0;i<atyp->size(); ++i) {
        m_d[i].version        = (*atyp)[i]->getInt("VERS");    
        m_d[i].jtyp           = (*atyp)[i]->getInt("JTYP");          
        m_d[i].nsta           = (*atyp)[i]->getInt("NSTA");          
        m_d[i].type           = (*atyp)[i]->getString("TYP");
    }
  }
  else {
    std::cerr<<"NO Atyp banks in the MuonDD Database"<<std::endl;
  }
}


} // end of namespace MuonGM
