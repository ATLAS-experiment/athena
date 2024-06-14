/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 DB data - Muon Station components
 -----------------------------------------
 ***************************************************************************/

#include "MuonGMdbObjects/DblQ00Awln.h"
#include "RDBAccessSvc/IRDBRecordset.h"
#include "RDBAccessSvc/IRDBAccessSvc.h"
#include "RDBAccessSvc/IRDBRecord.h"

#include <iostream>
#include <sstream>

namespace MuonGM
{

  DblQ00Awln::DblQ00Awln(IRDBAccessSvc *pAccessSvc, const std::string & GeoTag, const std::string & GeoNode){

    IRDBRecordset_ptr awln = pAccessSvc->getRecordsetPtr(getName(),GeoTag, GeoNode);

    if(awln->size()>0) {
      m_nObj = awln->size();
      m_d.resize (m_nObj);
      if (m_nObj == 0) std::cerr<<"NO Awln banks in the MuonDD Database"<<std::endl;

      for(size_t i =0; i<awln->size(); ++i) {
          m_d[i].version       = (*awln)[i]->getInt("VERS");    
          m_d[i].jsta          = (*awln)[i]->getInt("JSTA");    
          m_d[i].spitch        = (*awln)[i]->getFloat("SPITCH");  
          m_d[i].zpitch        = (*awln)[i]->getFloat("ZPITCH");  
          m_d[i].dedstr        = (*awln)[i]->getFloat("DEDSTR");  
          m_d[i].nsrest        = (*awln)[i]->getInt("NSREST");  
          m_d[i].nzrest        = (*awln)[i]->getInt("NZREST");
          m_d[i].sfirst        = (*awln)[i]->getFloat("SFIRST");
          m_d[i].zfirst        = (*awln)[i]->getFloat("ZFIRST");
          m_d[i].dedsep        = (*awln)[i]->getFloat("DEDSEP");
          m_d[i].nsrost        = (*awln)[i]->getInt("NSROST");  
          m_d[i].nzrost        = (*awln)[i]->getInt("NZROST");
      }
  }
  else {
    std::cerr<<"NO Awln banks in the MuonDD Database"<<std::endl;
  }
}


} // end of namespace MuonGM
