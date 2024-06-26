/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 DB data - Muon Station components
 -----------------------------------------
 ***************************************************************************/

#include "MuonGMdbObjects/DblQ00Wtgc.h"
#include "RDBAccessSvc/IRDBRecordset.h"
#include "RDBAccessSvc/IRDBAccessSvc.h"
#include "RDBAccessSvc/IRDBRecord.h"


#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdexcept>

namespace MuonGM
{

  DblQ00Wtgc::DblQ00Wtgc(IRDBAccessSvc *pAccessSvc, const std::string & GeoTag, const std::string & GeoNode){
  

    IRDBRecordset_ptr wtgc = pAccessSvc->getRecordsetPtr(getName(),GeoTag, GeoNode);

    if(wtgc->size()>0) {
      m_nObj = wtgc->size();
      m_d.resize (m_nObj);
      if (m_nObj == 0) std::cerr<<"NO Wtgc banks in the MuonDD Database"<<std::endl;

      for(size_t i=0; i <wtgc->size(); ++i) {
        m_d[i].version     = (*wtgc)[i]->getInt("VERS");    
        m_d[i].jsta        = (*wtgc)[i]->getInt("JSTA");
        m_d[i].nbevol      = (*wtgc)[i]->getInt("NBEVOL");
        m_d[i].x0          = (*wtgc)[i]->getFloat("X0");
        m_d[i].widchb      = (*wtgc)[i]->getFloat("WIDCHB");
        m_d[i].fwirch      = (*wtgc)[i]->getFloat("FWIRCH");
        m_d[i].fwixch      = (*wtgc)[i]->getFloat("FWIXCH");
        for (unsigned int j=0; j<9; j++) {
           
          try {
              m_d[i].allname[j] = (*wtgc)[i]->getString("ALLNAME_"+std::to_string(j));
          } catch (const std::runtime_error&) {
                break;
          }
        }
    }
  }
  else {
    std::cerr<<"NO Wtgc banks in the MuonDD Database"<<std::endl;
  }
}



} // end of namespace MuonGM
