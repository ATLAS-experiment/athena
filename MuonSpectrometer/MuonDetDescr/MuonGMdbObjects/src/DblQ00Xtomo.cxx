/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 DB data - Muon Station components
 -----------------------------------------
 ***************************************************************************/

#include "MuonGMdbObjects/DblQ00Xtomo.h"
#include "RDBAccessSvc/IRDBRecordset.h"
#include "RDBAccessSvc/IRDBRecord.h"
#include "RDBAccessSvc/IRDBAccessSvc.h"

#include <iostream>

namespace MuonGM {
  DblQ00Xtomo::DblQ00Xtomo(IRDBAccessSvc *pAccessSvc, const std::string & GeoTag, const std::string & GeoNode){

    IRDBRecordset_ptr xtomo = pAccessSvc->getRecordsetPtr(getName(),GeoTag, GeoNode);
    if(xtomo->size()>0) {

        
      m_nObj = xtomo->size();
      m_d.resize(m_nObj);
      if (m_nObj == 0) std::cerr << "No XtomoData entries in the MuonDD Database" << std::endl;

      for (size_t i =0 ; i<xtomo->size(); ++i) {
          m_d[i].line = i;
          m_d[i].XTOMOCHBERNAME = (*xtomo)[i]->getString("XTOMOCHBERNAME"); 
          m_d[i].XTOMOSITE = (*xtomo)[i]->getString("XTOMOSITE"); 
          m_d[i].XTOMOSITEID = (*xtomo)[i]->getInt("XTOMOSITEID");
          m_d[i].XTOMOTIME = (*xtomo)[i]->getInt("XTOMOTIME"); 
          m_d[i].XTOMOPASSED = (*xtomo)[i]->getInt("XTOMOPASSED");
          m_d[i].XTOMOSIDE = (*xtomo)[i]->getString("XTOMOSIDE");
          m_d[i].XTOMONBERTUBE1 = (*xtomo)[i]->getInt("XTOMONBERTUBE1");
          m_d[i].XTOMONBERTUBE2 = (*xtomo)[i]->getInt("XTOMONBERTUBE2");
          m_d[i].XTOMONBERML = (*xtomo)[i]->getInt("XTOMONBERML");
          m_d[i].XTOMONBERLAYER = (*xtomo)[i]->getInt("XTOMONBERLAYER");
          try {
            m_d[i].XTOMOML1STAGG = (*xtomo)[i]->getInt("XTOMOML1STAGG");
          } catch (const std::exception&) {} // ignore exception for now: field missing in MuonSpectrometer-R.06.01-tomotest
          try {
            m_d[i].XTOMOML2STAGG = (*xtomo)[i]->getInt("XTOMOML2STAGG");
          } catch (const std::exception&) {} // ignore exception for now: field missing in MuonSpectrometer-R.06.01-tomotest
          try {
            m_d[i].XTOMOD1 = (*xtomo)[i]->getFloat("XTOMOD1");
          } catch (const std::exception&) {} // ignore exception for now: field missing in MuonSpectrometer-R.06.01-tomotest
          try {
            m_d[i].XTOMONMEZ = (*xtomo)[i]->getInt("XTOMONMEZ");
          } catch (const std::exception&) {} // ignore exception for now: field missing in MuonSpectrometer-R.06.01-tomotest
          m_d[i].XTOMOML1NYTUB = (*xtomo)[i]->getFloat("XTOMOML1NYTUB");
          m_d[i].XTOMOML1NZTUB = (*xtomo)[i]->getFloat("XTOMOML1NZTUB"); 
          m_d[i].XTOMOML1NDELA = (*xtomo)[i]->getFloat("XTOMOML1NDELA"); 
          m_d[i].XTOMOML1NYPIT = (*xtomo)[i]->getFloat("XTOMOML1NYPIT"); 
          m_d[i].XTOMOML1NZPIT = (*xtomo)[i]->getFloat("XTOMOML1NZPIT"); 
          m_d[i].XTOMOML1PYTUB = (*xtomo)[i]->getFloat("XTOMOML1PYTUB"); 
          m_d[i].XTOMOML1PZTUB = (*xtomo)[i]->getFloat("XTOMOML1PZTUB"); 
          m_d[i].XTOMOML1PDELA = (*xtomo)[i]->getFloat("XTOMOML1PDELA"); 
          m_d[i].XTOMOML1PYPIT = (*xtomo)[i]->getFloat("XTOMOML1PYPIT"); 
          m_d[i].XTOMOML1PZPIT = (*xtomo)[i]->getFloat("XTOMOML1PZPIT"); 
          m_d[i].XTOMOML2NYTUB = (*xtomo)[i]->getFloat("XTOMOML2NYTUB"); 
          m_d[i].XTOMOML2NZTUB = (*xtomo)[i]->getFloat("XTOMOML2NZTUB"); 
          m_d[i].XTOMOML2NDELA = (*xtomo)[i]->getFloat("XTOMOML2NDELA"); 
          m_d[i].XTOMOML2NYPIT = (*xtomo)[i]->getFloat("XTOMOML2NYPIT"); 
          m_d[i].XTOMOML2NZPIT = (*xtomo)[i]->getFloat("XTOMOML2NZPIT"); 
          m_d[i].XTOMOML2PYTUB = (*xtomo)[i]->getFloat("XTOMOML2PYTUB"); 
          m_d[i].XTOMOML2PZTUB = (*xtomo)[i]->getFloat("XTOMOML2PZTUB"); 
          m_d[i].XTOMOML2PDELA = (*xtomo)[i]->getFloat("XTOMOML2PDELA"); 
          m_d[i].XTOMOML2PYPIT = (*xtomo)[i]->getFloat("XTOMOML2PYPIT"); 
          m_d[i].XTOMOML2PZPIT = (*xtomo)[i]->getFloat("XTOMOML2PZPIT"); 
    }
  } 
}
} // end of namespace MuonGM
