/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 DB data - Muon Station components
 -----------------------------------------
 ***************************************************************************/

#include "MuonGMdbObjects/DblQ00Wcro.h"
#include "RDBAccessSvc/IRDBRecordset.h"
#include "RDBAccessSvc/IRDBAccessSvc.h"
#include "RDBAccessSvc/IRDBRecord.h"

#include <iostream>
#include <sstream>

namespace MuonGM
{

  DblQ00Wcro::DblQ00Wcro(IRDBAccessSvc *pAccessSvc, const std::string & GeoTag, const std::string & GeoNode) {

    IRDBRecordset_ptr wcro = pAccessSvc->getRecordsetPtr(getName(),GeoTag, GeoNode);

    if(wcro->size()>0) {
      m_nObj = wcro->size();
      m_d.resize (m_nObj);
      if (m_nObj == 0) std::cerr<<"NO Wcro banks in the MuonDD Database"<<std::endl;

      for(size_t i =0; i<wcro->size(); ++i) {
         m_d[i].version     = (*wcro)[i]->getInt("VERS");    
         m_d[i].jsta        = (*wcro)[i]->getInt("JSTA");
         m_d[i].num         = (*wcro)[i]->getInt("NUM");
         m_d[i].heightness     = (*wcro)[i]->getFloat("HEIGHTNESS");
         m_d[i].largeness      = (*wcro)[i]->getFloat("LARGENESS");
         m_d[i].thickness      = (*wcro)[i]->getFloat("THICKNESS");
	    }
  } else {
    std::cerr<<"NO Wcro banks in the MuonDD Database"<<std::endl;
  }
}

} // end of namespace MuonGM
