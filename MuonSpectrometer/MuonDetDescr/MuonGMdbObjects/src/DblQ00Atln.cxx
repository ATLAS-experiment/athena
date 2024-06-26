/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 DB data - Muon Station components
 -----------------------------------------
 ***************************************************************************/

#include "MuonGMdbObjects/DblQ00Atln.h"
#include "RDBAccessSvc/IRDBRecordset.h"
#include "RDBAccessSvc/IRDBAccessSvc.h"
#include "RDBAccessSvc/IRDBRecord.h"

#include <string> 
#include <iostream>
#include <stdio.h>

namespace MuonGM
{

  DblQ00Atln::DblQ00Atln(IRDBAccessSvc *pAccessSvc, const std::string & GeoTag, const std::string & GeoNode) {

    IRDBRecordset_ptr atln = pAccessSvc->getRecordsetPtr(getName(),GeoTag, GeoNode);

    if(atln->size()>0) {
      m_nObj = atln->size();
      m_d.resize (m_nObj);
      if (m_nObj == 0) std::cerr<<"NO Atln banks in the MuonDD Database"<<std::endl;

      for (size_t i=0; i<atln->size(); ++i) {
          m_d[i].version     = (*atln)[i]->getInt("VERS");    
          m_d[i].i           = (*atln)[i]->getInt("I");          
          m_d[i].icovol      = (*atln)[i]->getInt("ICOVOL");
          m_d[i].zpovol      = (*atln)[i]->getFloat("ZPOVOL");
          m_d[i].widvol      = (*atln)[i]->getFloat("WIDVOL");
          m_d[i].namvol      = (*atln)[i]->getString("NAMVOL");
          m_d[i].jsta        = (*atln)[i]->getInt("JSTA");          
      }
  }
  else {
    std::cerr<<"NO Atln banks in the MuonDD Database"<<std::endl;
  }
}

} // end of namespace MuonGM
