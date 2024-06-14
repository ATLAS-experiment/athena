/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 DB data - Muon Station components
 -----------------------------------------
 ***************************************************************************/

#include "MuonGMdbObjects/DblQ00Wmdt.h"
#include "RDBAccessSvc/IRDBRecordset.h"
#include "RDBAccessSvc/IRDBAccessSvc.h"
#include "RDBAccessSvc/IRDBRecord.h"


#include <iostream>
#include <sstream>
#include <stdio.h>

namespace MuonGM
{

  DblQ00Wmdt::DblQ00Wmdt(IRDBAccessSvc *pAccessSvc, const std::string & GeoTag, const std::string & GeoNode){

    IRDBRecordset_ptr wmdt = pAccessSvc->getRecordsetPtr(getName(),GeoTag, GeoNode);

    if(wmdt->size()>0) {
      m_nObj = wmdt->size();
      m_d.resize (m_nObj);
      if (m_nObj == 0) std::cerr<<"NO Wmdt banks in the MuonDD Database"<<std::endl;

      for(size_t i =0;i<wmdt->size(); ++i) {
          m_d[i].version     = (*wmdt)[i]->getInt("VERS");    
          m_d[i].iw          = (*wmdt)[i]->getInt("IW");
          m_d[i].laymdt      = (*wmdt)[i]->getInt("LAYMDT");
          m_d[i].typ         = (*wmdt)[i]->getString("TYP");
          m_d[i].x0          = (*wmdt)[i]->getFloat("X0");
          m_d[i].tubpit      = (*wmdt)[i]->getFloat("TUBPIT");
          m_d[i].tubrad      = (*wmdt)[i]->getFloat("TUBRAD");
          m_d[i].tubsta      = (*wmdt)[i]->getFloat("TUBSTA");
          m_d[i].tubdea      = (*wmdt)[i]->getFloat("TUBDEA");
          m_d[i].tubwal      = (*wmdt)[i]->getFloat("TUBWAL");
          for (unsigned int j=0; j<4; j++) {
            m_d[i].tubxco[j]     = (*wmdt)[i]->getFloat("TUBXCO_"+std::to_string(j));        
            m_d[i].tubyco[j]     = (*wmdt)[i]->getFloat("TUBYCO_"+std::to_string(j));        
          }
      }
  } else {
    std::cerr<<"NO Wmdt banks in the MuonDD Database"<<std::endl;
  }
}


} // end of namespace MuonGM
