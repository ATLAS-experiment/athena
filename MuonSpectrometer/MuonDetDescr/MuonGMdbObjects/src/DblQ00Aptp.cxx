/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 DB data - Muon Station components
 -----------------------------------------
 ***************************************************************************/

#include "MuonGMdbObjects/DblQ00Aptp.h"
#include "RDBAccessSvc/IRDBRecordset.h"
#include "RDBAccessSvc/IRDBAccessSvc.h"
#include "RDBAccessSvc/IRDBRecord.h"

#include <iostream>
#include <sstream>
#include <stdio.h>

namespace MuonGM
{

  DblQ00Aptp::DblQ00Aptp(IRDBAccessSvc *pAccessSvc, const std::string & GeoTag, const std::string & GeoNode){

    IRDBRecordset_ptr aptp = pAccessSvc->getRecordsetPtr(getName(),GeoTag, GeoNode);

    if(aptp->size()>0) {
      m_nObj = aptp->size();
      m_d.resize (m_nObj);
      if (m_nObj == 0) std::cerr<<"NO Aptp banks in the MuonDD Database"<<std::endl;

      for(size_t i=0;i<aptp->size(); ++i) {
        m_d[i].version     = (*aptp)[i]->getInt("VERS");    
        m_d[i].line        = (*aptp)[i]->getInt("LINE");
        m_d[i].type             = (*aptp)[i]->getString("TYP"); 
        m_d[i].i           = (*aptp)[i]->getInt("I");
        m_d[i].icut        = (*aptp)[i]->getInt("ICUT");
        m_d[i].iphi.resize(8);
        m_d[i].iphi[0]     = (*aptp)[i]->getInt("IPHI_0");        
        m_d[i].iphi[1]     = (*aptp)[i]->getInt("IPHI_1");        
        m_d[i].iphi[2]     = (*aptp)[i]->getInt("IPHI_2");        
        m_d[i].iphi[3]     = (*aptp)[i]->getInt("IPHI_3");        
        m_d[i].iphi[4]     = (*aptp)[i]->getInt("IPHI_4");        
        m_d[i].iphi[5]     = (*aptp)[i]->getInt("IPHI_5");        
        m_d[i].iphi[6]     = (*aptp)[i]->getInt("IPHI_6");        
        m_d[i].iphi[7]     = (*aptp)[i]->getInt("IPHI_7");        
        m_d[i].iz          = (*aptp)[i]->getInt("IZ");
        m_d[i].dphi        = (*aptp)[i]->getFloat("DPHI");  
        m_d[i].z           = (*aptp)[i]->getFloat("Z");  
        m_d[i].r           = (*aptp)[i]->getFloat("R");    
        m_d[i].s           = (*aptp)[i]->getFloat("S");     
        m_d[i].alfa        = (*aptp)[i]->getFloat("ALFA");      
        m_d[i].beta        = (*aptp)[i]->getFloat("BETA");     
        m_d[i].gamma       = (*aptp)[i]->getFloat("GAMMA");
      }
  }
  else {
    std::cerr<<"NO Aptp banks in the MuonDD Database"<<std::endl;
  }
}


} // end of namespace MuonGM
