/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 DB data - Muon Station components
 -----------------------------------------
 ***************************************************************************/

#include "MuonGMdbObjects/DblQ00Wspa.h"
#include "RDBAccessSvc/IRDBRecordset.h"
#include "RDBAccessSvc/IRDBAccessSvc.h"
#include "RDBAccessSvc/IRDBRecord.h"

#include <iostream>
#include <sstream>

namespace MuonGM{

  DblQ00Wspa::DblQ00Wspa(IRDBAccessSvc *pAccessSvc, const std::string & GeoTag, const std::string & GeoNode) {

    IRDBRecordset_ptr wspa = pAccessSvc->getRecordsetPtr(getName(),GeoTag, GeoNode);

    if(wspa->size()>0) {
      m_nObj = wspa->size();
      m_d.resize (m_nObj);
      if (m_nObj == 0) std::cerr<<"NO Wspa banks in the MuonDD Database"<<std::endl;   
      for(size_t i=0; i<wspa->size(); ++i) {
        m_d[i].version     = (*wspa)[i]->getInt("VERS");    
        m_d[i].jsta        = (*wspa)[i]->getInt("JSTA");
        m_d[i].nb        = (*wspa)[i]->getInt("NB");
        m_d[i].x0          = (*wspa)[i]->getFloat("X0");
        m_d[i].tckspa      = (*wspa)[i]->getFloat("TCKSPA");
      }
  } else {
    std::cerr<<"NO Wspa banks in the MuonDD Database"<<std::endl;
  }
}


} // end of namespace MuonGM
