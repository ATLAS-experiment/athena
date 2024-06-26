/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/********************************************************
 Class def for MuonGeoModel DblQ00/ATYP
 *******************************************************/

 //  author: S Spagnolo
 // entered: 07/28/04
 // comment: STATION TYPE

#ifndef DBLQ00_ATYP_H
#define DBLQ00_ATYP_H
#include <string>
#include <vector>
#include <array>

class IRDBAccessSvc;

namespace MuonGM {

class DblQ00Atyp {
public:
    DblQ00Atyp() = default;
    ~DblQ00Atyp() = default;
    DblQ00Atyp(IRDBAccessSvc *pAccessSvc,const std::string & GeoTag="", const std::string & GeoNode="" );
    
    DblQ00Atyp & operator=(const DblQ00Atyp &right) = delete;
    DblQ00Atyp(const DblQ00Atyp&) = delete;

    // data members for DblQ00/ATYP fields
    struct ATYP {
        int version{0}; // VERSION
        int jtyp{0}; // AMDB STATION TYPE
        std::string type{}; // AMDB STATION NAME
        int nsta{0}; // NUMBER OF STATIONS OF THIS TYPE
    };
    
    const ATYP* data() const { return m_d.data(); };
    unsigned int size() const { return m_nObj; };
    unsigned int getNObj() const { return m_nObj; };

    std::string getName() const { return "ATYP"; };
    std::string getDirName() const { return "DblQ00"; };
    std::string getObjName() const { return "ATYP"; };

private:
    std::vector<ATYP> m_d{0};
    unsigned int m_nObj{0}; // > 1 if array; 0 if error in retrieve.
};
    
} // end of MuonGM namespace

#endif // DBLQ00_ATYP_H

