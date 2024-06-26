/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 DB data - Muon Station components
 -----------------------------------------
 ***************************************************************************/

#include "MuonGMdbObjects/DblQ00Wcsc.h"
#include "RDBAccessSvc/IRDBRecordset.h"
#include "RDBAccessSvc/IRDBAccessSvc.h"
#include "RDBAccessSvc/IRDBRecord.h"

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <limits>

namespace MuonGM
{

  DblQ00Wcsc::DblQ00Wcsc(IRDBAccessSvc *pAccessSvc, const std::string & GeoTag, const std::string & GeoNode){

    IRDBRecordset_ptr wcsc = pAccessSvc->getRecordsetPtr(getName(),GeoTag, GeoNode);

    if(wcsc->size()>0) {
      m_nObj = wcsc->size();
      m_d.resize (m_nObj);
      if (m_nObj == 0) std::cerr<<"NO Wcsc banks in the MuonDD Database"<<std::endl;

      for(size_t i = 0; i<wcsc->size(); ++i) {
          m_d[i].version     = (*wcsc)[i]->getInt("VERS");    
          m_d[i].jsta        = (*wcsc)[i]->getInt("JSTA");    
          m_d[i].laycsc      = (*wcsc)[i]->getInt("LAYCSC");
          m_d[i].ttotal      = (*wcsc)[i]->getFloat("TTOTAL");
          m_d[i].tnomex      = (*wcsc)[i]->getFloat("TNOMEX"); 
          m_d[i].tlag10      = (*wcsc)[i]->getFloat("TLAG10"); 
          m_d[i].wispa       = (*wcsc)[i]->getFloat("WISPA"); 
          m_d[i].dancat      = (*wcsc)[i]->getFloat("DANCAT"); 
          m_d[i].pcatre      = (*wcsc)[i]->getFloat("PCATRE"); 
          m_d[i].gstrip      = (*wcsc)[i]->getFloat("GSTRIP"); 
          m_d[i].wrestr      = (*wcsc)[i]->getFloat("WRESTR"); 
          m_d[i].wflstr      = (*wcsc)[i]->getFloat("WFLSTR"); 
          m_d[i].trrwas      = (*wcsc)[i]->getFloat("TRRWAS"); 
          m_d[i].wroxa       = (*wcsc)[i]->getFloat("WROXA"); 
          m_d[i].groxwi      = (*wcsc)[i]->getFloat("GROXWI"); 
          m_d[i].wgasba      = (*wcsc)[i]->getFloat("WGASBA"); 
          m_d[i].tgasba      = (*wcsc)[i]->getFloat("TGASBA"); 
          m_d[i].wgascu      = (*wcsc)[i]->getFloat("WGASCU"); 
          m_d[i].tgascu      = (*wcsc)[i]->getFloat("TGASCU"); 
          m_d[i].wfixwi      = (*wcsc)[i]->getFloat("WFIXWI"); 
          m_d[i].tfixwi      = (*wcsc)[i]->getFloat("TFIXWI"); 
          m_d[i].pba1wi      = (*wcsc)[i]->getFloat("PBA1WI"); 
          m_d[i].pba2wi      = (*wcsc)[i]->getFloat("PBA2WI"); 
          m_d[i].pba3wi      = (*wcsc)[i]->getFloat("PBA3WI"); 
          m_d[i].psndco      = (*wcsc)[i]->getFloat("PSNDCO");
          m_d[i].azcat = 0.;
          float  azcat = 0.;
          try {
              azcat = (*wcsc)[i]->getFloat("AZCAT");
              m_d[i].azcat =   azcat;
          }
          catch (const std::runtime_error&) {
              std::cerr<<" azcat field does not exists !"<<std::endl;
              m_d[i].azcat =   0.;
          }
      }
  }
  else {
    std::cerr<<"NO Wcsc banks in the MuonDD Database"<<std::endl;
  }
}


} // end of namespace MuonGM
