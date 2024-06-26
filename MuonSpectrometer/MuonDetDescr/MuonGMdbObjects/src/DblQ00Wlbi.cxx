/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 DB data - Muon Station components
 -----------------------------------------
 ***************************************************************************/

#include "MuonGMdbObjects/DblQ00Wlbi.h"
#include "RelationalAccess/ICursor.h"
#include "CoralBase/AttributeList.h"
#include "CoralBase/Attribute.h"
#include "RDBAccessSvc/IRDBRecordset.h"
#include "RDBAccessSvc/IRDBAccessSvc.h"
#include "RDBAccessSvc/IRDBRecord.h"


#include <iostream>
#include <sstream>
#include <stdexcept>
#include <limits>

namespace MuonGM
{

  DblQ00Wlbi::DblQ00Wlbi(IRDBAccessSvc *pAccessSvc, const std::string & GeoTag, const std::string & GeoNode) {

    IRDBRecordset_ptr wlbi = pAccessSvc->getRecordsetPtr(getName(),GeoTag, GeoNode);

    if(wlbi->size()>0) {
      m_nObj = wlbi->size();
      m_d.resize (m_nObj);
      if (m_nObj == 0) std::cerr<<"NO Wlbi banks in the MuonDD Database"<<std::endl;

    for(size_t i =0;i<wlbi->size(); ++i) {
        m_d[i].version     = (*wlbi)[i]->getInt("VERS");    
        m_d[i].jsta        = (*wlbi)[i]->getInt("JSTA");
        m_d[i].num         = (*wlbi)[i]->getInt("NUM");
        m_d[i].height      = (*wlbi)[i]->getFloat("HEIGHT");
        m_d[i].thickness   = (*wlbi)[i]->getFloat("THICKNESS");
	      try{
	         m_d[i].lowerThickness   = (*wlbi)[i]->getFloat("LOWERTHICK");
	      } catch (const std::runtime_error &) {
	        m_d[i].lowerThickness   = m_d[i].thickness ;
	      }
        try {
	        m_d[i].yShift   = (*wlbi)[i]->getFloat("SHIFTYSTATION");
	      } catch(const std::runtime_error &) {
	        m_d[i].yShift   = 0.;
	      }
    }
  }
  else {
    std::cerr<<"NO Wlbi banks in the MuonDD Database"<<std::endl;
  }
}

} // end of namespace MuonGM
