/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 DB data - Muon Station components
 -----------------------------------------
 ***************************************************************************/

#include "MuonGMdbObjects/DblQ00Asmp.h"
#include "RDBAccessSvc/IRDBRecordset.h"
#include "RDBAccessSvc/IRDBAccessSvc.h"
#include "RDBAccessSvc/IRDBRecord.h"

#include <iostream>

namespace MuonGM
{

  DblQ00Asmp::DblQ00Asmp(IRDBAccessSvc *pAccessSvc, const std::string & GeoTag, const std::string & GeoNode){

    IRDBRecordset_ptr asmp = pAccessSvc->getRecordsetPtr(getName(),GeoTag, GeoNode);

    if(asmp->size()>0) {
      m_nObj = asmp->size();
      m_d.resize (m_nObj);
      if (m_nObj == 0) std::cerr<<"NO Asmp banks in the MuonDD Database"<<std::endl;

      for (size_t i = 0 ; i<asmp->size(); ++i) {
          m_d[i].version     = (*asmp)[i]->getInt("VERS");    
          m_d[i].indx        = (*asmp)[i]->getInt("INDX");
          m_d[i].n           = (*asmp)[i]->getInt("N");          
          m_d[i].jtyp        = (*asmp)[i]->getInt("JTYP");       
      }
  } else {
    std::cerr<<"NO Asmp banks in the MuonDD Database"<<std::endl;
  }
}
} // end of namespace MuonGM
