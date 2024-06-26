/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/********************************************************
 Class def for MuonGeoModel DblQ00/WSUP
 *******************************************************/

 //  author: S Spagnolo
 // entered: 07/28/04
 // comment: RPC SUPPORT

#ifndef DBLQ00_WSUP_H
#define DBLQ00_WSUP_H
#include <string>
#include <vector>
#include <array>


class IRDBAccessSvc;


namespace MuonGM {
class DblQ00Wsup {
public:
    DblQ00Wsup() = default;
    ~DblQ00Wsup() = default;
    DblQ00Wsup(IRDBAccessSvc *pAcccessSvc, const std::string & GeoTag="", const std::string & GeoNode="");
    DblQ00Wsup & operator=(const DblQ00Wsup &right) = delete;
    DblQ00Wsup(const DblQ00Wsup&)= delete;

    // data members for DblQ00/WSUP fields
    struct WSUP {
        int version{0}; // VERSION
        int jsta{0}; // INDEX
        int nxxsup{0}; // MAX NB. FOR X FRAGMENTS
        int nzzsup{0}; // MAX NB. FOR Z FRAGMENTS
        float x0{0.f}; // X0
        float thickn{0.f}; // THICKNESS
        std::array<float,4> xxsup{}; // X DIMENSION
        std::array<float,4> zzsup{}; // Z DIMENSION
    };
    
    const WSUP* data() const { return m_d.data(); };
    unsigned int size() const { return m_nObj; };
    std::string getName() const { return "WSUP"; };
    std::string getDirName() const { return "DblQ00"; };
    std::string getObjName() const { return "WSUP"; };

private:
    std::vector<WSUP> m_d{};
    unsigned int m_nObj{0}; // > 1 if array; 0 if error in retrieve.

};
} // end of MuonGM namespace

#endif // DBLQ00_WSUP_H

