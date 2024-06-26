/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

/********************************************************
 Class def for MuonGeoModel DblQ00/Xtomo
 *******************************************************/

 //  author: M Schreyer
 // entered: 2012-09-24
 // comment: Mdt AsBuilt parameters - class to read from DB

#ifndef DBLQ00_XTOMO_H
#define DBLQ00_XTOMO_H

class IRDBAccessSvc;

#include <string>
#include <vector>

namespace MuonGM {
class DblQ00Xtomo {
public:
    DblQ00Xtomo() = default;
    ~DblQ00Xtomo() = default;
    DblQ00Xtomo(IRDBAccessSvc *pAccessSvc,const std::string & GeoTag="", const std::string & GeoNode="");

    DblQ00Xtomo & operator=(const DblQ00Xtomo &right) = default;
    DblQ00Xtomo(const DblQ00Xtomo&) = default;


    // data members for DblQ00/XTOMO fields
    struct XTOMO {
          int line{0}; // LINE NUMBER
          std::string XTOMOCHBERNAME{}; 
          std::string XTOMOSITE{};     
          int XTOMOSITEID{0};     
          int XTOMOTIME{0}; 
          int XTOMOPASSED{0};     
          std::string XTOMOSIDE{}; 
          int XTOMONBERTUBE1{0};  
          int XTOMONBERTUBE2{0};  
          int XTOMONBERML{0};     
          int XTOMONBERLAYER{0};  
          int XTOMOML1STAGG{0};
          int XTOMOML2STAGG{0};
          float XTOMOD1{0.f};
          int XTOMONMEZ{0};
          float XTOMOML1NYTUB{0.f};   
          float XTOMOML1NZTUB{0.f};   
          float XTOMOML1NDELA{0.f};   
          float XTOMOML1NYPIT{0.f};   
          float XTOMOML1NZPIT{0.f};   
          float XTOMOML1PYTUB{0.f};   
          float XTOMOML1PZTUB{0.f};   
          float XTOMOML1PDELA{0.f};   
          float XTOMOML1PYPIT{0.f};   
          float XTOMOML1PZPIT{0.f};   
          float XTOMOML2NYTUB{0.f};   
          float XTOMOML2NZTUB{0.f};   
          float XTOMOML2NDELA{0.f};   
          float XTOMOML2NYPIT{0.f};   
          float XTOMOML2NZPIT{0.f};   
          float XTOMOML2PYTUB{0.f};   
          float XTOMOML2PZTUB{0.f};   
          float XTOMOML2PDELA{0.f};   
          float XTOMOML2PYPIT{0.f};   
          float XTOMOML2PZPIT{0.f};
    };    
    
    const XTOMO* data() const { return m_d.data(); };
    unsigned int size() const { return m_nObj; };
    std::string getName() const { return "XTOMO"; };
    std::string getDirName() const { return "DblQ00"; };
    std::string getObjName() const { return "XTOMO"; };

private:
    std::vector<XTOMO> m_d{};
    unsigned int m_nObj{0}; // > 1 if array; 0 if error in retrieve.
};


} // end of MuonGM namespace

#endif // DBLQ00_ASZT_H

