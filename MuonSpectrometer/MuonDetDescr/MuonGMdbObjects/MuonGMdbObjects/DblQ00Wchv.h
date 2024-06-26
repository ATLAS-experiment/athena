/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/********************************************************
 Class def for MuonGeoModel DblQ00/WCHV
 *******************************************************/

 //  author: S Spagnolo
 // entered: 07/28/04
 // comment: CHV SPACER

#ifndef DBLQ00_WCHV_H
#define DBLQ00_WCHV_H

#include <string>
#include <vector>

class IRDBAccessSvc;


namespace MuonGM {
class DblQ00Wchv {
public:
    DblQ00Wchv() = default;
    ~DblQ00Wchv() = default;
    DblQ00Wchv(IRDBAccessSvc *pAccessSvc, const std::string & GeoTag="", const std::string & GeoNode="");

    DblQ00Wchv & operator=(const DblQ00Wchv &right) = default;
    DblQ00Wchv(const DblQ00Wchv&) = default;

    
    // data members for DblQ00/WCHV fields
    struct WCHV {
        int version{0}; // VERSION
        int jsta{0}; // INDEX
        int num{0}; // NUMBER OF OBJECTS
        float heightness{0.f}; // HEIGHT
        float largeness{0.f}; // T-SHAPE LARGENESS
        float thickness{0.f}; // T-SHAPE THICKNESS
    };
    
    const WCHV* data() const { return m_d.data(); };
    unsigned int size() const { return m_nObj; };
    std::string getName() const { return "WCHV"; };
    std::string getDirName() const { return "DblQ00"; };
    std::string getObjName() const { return "WCHV"; };

private:
    std::vector<WCHV> m_d{};
    unsigned int m_nObj{0}; // > 1 if array; 0 if error in retrieve.
};
} // end of MuonGM namespace

#endif // DBLQ00_WCHV_H

