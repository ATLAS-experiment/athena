/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 DB data - Muon Station components
 -----------------------------------------
 ***************************************************************************/

#include "MuonGMdbObjects/DblQ00Almn.h"
#include "RDBAccessSvc/IRDBRecordset.h"
#include "RDBAccessSvc/IRDBAccessSvc.h"
#include "RDBAccessSvc/IRDBRecord.h"

#include <iostream>
#include <stdio.h>

namespace MuonGM
{

  DblQ00Almn::DblQ00Almn(IRDBAccessSvc *pAccessSvc, const std::string & GeoTag, const std::string & GeoNode){

    IRDBRecordset_ptr almn = pAccessSvc->getRecordsetPtr(getName(),GeoTag, GeoNode);

    if(almn->size()>0) {
      m_nObj = almn->size();
      m_d.resize (m_nObj);
      if (m_nObj == 0) std::cerr<<"NO Almn banks in the MuonDD Database"<<std::endl;

      for(size_t i = 0; i<almn->size(); ++i) {
          m_d[i].version =           (*almn)[i]->getInt("VERS");
          m_d[i].i =                 (*almn)[i]->getInt("I");
          m_d[i].dx =                (*almn)[i]->getFloat("DX");
          m_d[i].dy =                (*almn)[i]->getFloat("DY");
          m_d[i].dz =                (*almn)[i]->getFloat("DZ");
          m_d[i].job =               (*almn)[i]->getInt("JOB");
          m_d[i].tec =               (*almn)[i]->getString("TEC");
          m_d[i].iw =                (*almn)[i]->getInt("IW");
          m_d[i].isplit_x =          (*almn)[i]->getInt("ISPLIT_X");
          m_d[i].isplit_y =          (*almn)[i]->getInt("ISPLIT_Y");
          m_d[i].ishape =            (*almn)[i]->getInt("ISHAPE");
          m_d[i].width_xs =          (*almn)[i]->getFloat("WIDTH_XS");
          m_d[i].width_xl =          (*almn)[i]->getFloat("WIDTH_XL");
          m_d[i].length_y =          (*almn)[i]->getFloat("LENGTH_Y");
          m_d[i].excent =            (*almn)[i]->getFloat("EXCENT");
          m_d[i].dead1 =             (*almn)[i]->getFloat("DEAD1");
          m_d[i].dead2 =             (*almn)[i]->getFloat("DEAD2");
          m_d[i].dead3 =             (*almn)[i]->getFloat("DEAD3");
          m_d[i].jtyp =              (*almn)[i]->getInt("JTYP");
          m_d[i].indx =              (*almn)[i]->getInt("INDX");
      }
  }
  else {
    std::cerr<<"NO Almn banks in the MuonDD Database"<<std::endl;
  }
}

} // end of namespace MuonGM
