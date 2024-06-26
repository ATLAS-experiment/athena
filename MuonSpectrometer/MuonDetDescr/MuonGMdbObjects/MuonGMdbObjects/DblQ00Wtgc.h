/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/********************************************************
 Class def for MuonGeoModel DblQ00/WTGC
 *******************************************************/

 //  author: S Spagnolo
 // entered: 07/28/04
 // comment: TGC GENERAL

#ifndef DBLQ00_WTGC_H
#define DBLQ00_WTGC_H

#include <string>
#include <vector>
#include <array>
class IRDBAccessSvc;


namespace MuonGM {
class DblQ00Wtgc {
public:
    DblQ00Wtgc()= default;
    ~DblQ00Wtgc() = default;
    DblQ00Wtgc & operator=(const DblQ00Wtgc &right) = delete;
    DblQ00Wtgc(const DblQ00Wtgc&) = delete;

    DblQ00Wtgc(IRDBAccessSvc *pAccessSvc, const std::string & GeoTag="", const std::string & GeoNode="");

    // data members for DblQ00/WTGC fields
    struct WTGC {
        int version{0}; // VERSION
        int jsta{0}; // JSTA INDEX
        int nbevol{0}; // NUMBER OF DETAILS
        float x0{0.f}; // X0
        float widchb{0.f}; // WIDTH OF THE CHBER ALONG Z
        float fwirch{0.f}; // FRAME WIDTH IN R
        float fwixch{0.f}; // FRAME WIDTH IN X
        std::array<std::string, 9> allname{}; // MATERIAL
    };
    
    const WTGC* data() const { return m_d.data(); };
    unsigned int size() const { return m_nObj; };
    std::string getName() const { return "WTGC"; };
    std::string getDirName() const { return "DblQ00"; };
    std::string getObjName() const { return "WTGC"; };

private:
    std::vector<WTGC> m_d{};
    unsigned int m_nObj{0}; // > 1 if array; 0 if error in retrieve.
};
} // end of MuonGM namespace

#endif // DBLQ00_WTGC_H

