/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 DB data - Muon Station components
 -----------------------------------------
 ***************************************************************************/

#include "MuonGMdbObjects/DblQ00Alin.h"
#include "RDBAccessSvc/IRDBRecordset.h"
#include "RDBAccessSvc/IRDBAccessSvc.h"
#include "RDBAccessSvc/IRDBRecord.h"

#include <iostream>
#include <stdio.h>

namespace MuonGM
{

  DblQ00Alin::DblQ00Alin(IRDBAccessSvc *pAccessSvc, const std::string & GeoTag, const std::string & GeoNode) {

    IRDBRecordset_ptr alin = pAccessSvc->getRecordsetPtr(getName(),GeoTag, GeoNode);

    if(alin->size()>0) {
      m_nObj = alin->size();
      m_d.resize (m_nObj);
      if (m_nObj == 0) std::cerr<<"NO Alin banks in the MuonDD Database"<<std::endl;

      for(size_t i=0;i <alin->size(); ++i) {
          m_d[i].version      = (*alin)[i]->getInt("VERS");    
          m_d[i].dx           = (*alin)[i]->getFloat("DX");          
          m_d[i].dy           = (*alin)[i]->getFloat("DY");       
          m_d[i].i            = (*alin)[i]->getInt("I");   
          m_d[i].width_xs     = (*alin)[i]->getFloat("WIDTH_XS");
          m_d[i].width_xl     = (*alin)[i]->getFloat("WIDTH_XL");  
          m_d[i].length_y     = (*alin)[i]->getFloat("LENGTH_Y");  
          m_d[i].excent       = (*alin)[i]->getFloat("EXCENT");    
          m_d[i].dead1        = (*alin)[i]->getFloat("DEAD1");     
          m_d[i].jtyp         = (*alin)[i]->getInt("JTYP");      
          m_d[i].indx         = (*alin)[i]->getInt("INDX");     
          m_d[i].icut         = (*alin)[i]->getInt("ICUT");      
      }
  }
  else {
    std::cerr<<"NO Alin banks in the MuonDD Database"<<std::endl;
  }
}


} // end of namespace MuonGM
