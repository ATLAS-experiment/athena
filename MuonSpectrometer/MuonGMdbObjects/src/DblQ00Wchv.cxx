/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 DB data - Muon Station components
 -----------------------------------------
 ***************************************************************************/

#include "MuonGMdbObjects/DblQ00Wchv.h"
#include "RDBAccessSvc/IRDBRecordset.h"
#include "RDBAccessSvc/IRDBAccessSvc.h"
#include "RDBAccessSvc/IRDBRecord.h"

#include <iostream>
#include <sstream>

namespace MuonGM
{

  DblQ00Wchv::DblQ00Wchv(IRDBAccessSvc *pAccessSvc, const std::string & GeoTag, const std::string & GeoNode){

    IRDBRecordset_ptr wchv = pAccessSvc->getRecordsetPtr(getName(),GeoTag, GeoNode);

    if(wchv->size()>0) {
      m_nObj = wchv->size();
      m_d.resize (m_nObj);
      if (m_nObj == 0) std::cerr<<"NO Wchv banks in the MuonDD Database"<<std::endl;

      for (size_t i =0;i<wchv->size(); ++i) {
          m_d[i].version     = (*wchv)[i]->getInt("VERS");    
          m_d[i].jsta        = (*wchv)[i]->getInt("JSTA");
          m_d[i].num         = (*wchv)[i]->getInt("NUM");
          m_d[i].heightness  = (*wchv)[i]->getFloat("HEIGHTNESS");
          m_d[i].largeness   = (*wchv)[i]->getFloat("LARGENESS");
          m_d[i].thickness   = (*wchv)[i]->getFloat("THICKNESS");
      }
  }
  else {
    std::cerr<<"NO Wchv banks in the MuonDD Database"<<std::endl;
  }
}

} // end of namespace MuonGM
