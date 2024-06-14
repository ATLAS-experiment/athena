/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/********************************************************
 Class def for MuonGeoModel DblQ00/ASMP
 *******************************************************/

 //  author: S Spagnolo
 // entered: 07/28/04
 // comment: SINGLE MUON STATION

#ifndef DBLQ00_ASMP_H
#define DBLQ00_ASMP_H

#include <string>
#include <vector>
class IRDBAccessSvc;


namespace MuonGM {
class DblQ00Asmp {
public:
    DblQ00Asmp() = default;
    ~DblQ00Asmp() = default;
    DblQ00Asmp(IRDBAccessSvc *pAccessSvc, const std::string & GeoTag="", const std::string & GeoNode="");
    
    DblQ00Asmp & operator=(const DblQ00Asmp &right) = default;
    DblQ00Asmp(const DblQ00Asmp&) = default;


    // data members for DblQ00/ASMP fields
    struct ASMP {
        int version{0}; // VERSION
        int indx{0}; // STATION NUMBER (INSIDE TYPE)
        int n{0}; // NUMBER OF ELEMENTS
        int jtyp{0}; // AMDB STATION TYPE
    };
    
    const ASMP* data() const { return m_d.data(); };
    unsigned int size() const { return m_nObj; };
    std::string getName() const { return "ASMP"; };
    std::string getDirName() const { return "DblQ00"; };
    std::string getObjName() const { return "ASMP"; };

private:
  std::vector<ASMP> m_d{};
  unsigned int m_nObj{0}; // > 1 if array; 0 if error in retrieve.
};
} // end of MuonGM namespace

#endif // DBLQ00_ASMP_H

