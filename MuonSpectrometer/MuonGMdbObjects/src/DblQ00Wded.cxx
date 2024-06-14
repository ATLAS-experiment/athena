/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 DB data - Muon Station components
 -----------------------------------------
 ***************************************************************************/

#include "MuonGMdbObjects/DblQ00Wded.h"
#include "RDBAccessSvc/IRDBRecordset.h"
#include "RDBAccessSvc/IRDBAccessSvc.h"
#include "RDBAccessSvc/IRDBRecord.h"

#include <iostream>
#include <sstream>

namespace MuonGM
{

  DblQ00Wded::DblQ00Wded(IRDBAccessSvc *pAccessSvc, const std::string & GeoTag, const std::string & GeoNode) {

    IRDBRecordset_ptr wded = pAccessSvc->getRecordsetPtr(getName(),GeoTag, GeoNode);

    if(wded->size()>0) {
      m_nObj = wded->size();
      m_d.resize (m_nObj);
      if (m_nObj == 0) std::cerr<<"NO Wded banks in the MuonDD Database"<<std::endl;

      for(size_t i =0 ;i<wded->size(); ++i) {
        m_d[i].version     = (*wded)[i]->getInt("VERS");    
        m_d[i].jsta        = (*wded)[i]->getInt("JSTA");
        m_d[i].nb        = (*wded)[i]->getInt("NB");
        m_d[i].x0          = (*wded)[i]->getFloat("X0");
        m_d[i].auphcb      = (*wded)[i]->getFloat("AUPHCB");
        m_d[i].tckded      = (*wded)[i]->getFloat("TCKDED");
      }
  }
  else {
    std::cerr<<"NO Wded banks in the MuonDD Database"<<std::endl;
  }
}


} // end of namespace MuonGM
