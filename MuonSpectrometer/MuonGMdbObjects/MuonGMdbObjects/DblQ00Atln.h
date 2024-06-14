/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/********************************************************
 Class def for MuonGeoModel DblQ00/ATLN
 *******************************************************/

 //  author: S Spagnolo
 // entered: 07/28/04
 // comment: MATERIAL OF TGC

#ifndef DBLQ00_ATLN_H
#define DBLQ00_ATLN_H

#include <string>
#include <vector>
#include <array>
class IRDBAccessSvc;


namespace MuonGM {
class DblQ00Atln {
public:
    DblQ00Atln() = default;
    ~DblQ00Atln() = default;
    DblQ00Atln(IRDBAccessSvc *pAccessSvc, const std::string & GeoTag="", const std::string & GeoNode="");
    DblQ00Atln & operator=(const DblQ00Atln &right) = delete;
    DblQ00Atln(const DblQ00Atln&) = delete;

    // data members for DblQ00/ATLN fields
    struct ATLN {
        int version{0};     // VERSION
        int i{0};           // INDEX
        int icovol{0};      // MATERIAL CODE
        float zpovol{0.f}; // RELATIVE Z POSITION
        float widvol{0.f}; // Z WIDTH
        std::string namvol{}; // MATERIAL NAME
        int jsta{0}; // 
    };

    const ATLN* data() const { return m_d.data(); };
    unsigned int size() const { return m_nObj; };
    std::string getName() const { return "ATLN"; };
    std::string getDirName() const { return "DblQ00"; };
    std::string getObjName() const { return "ATLN"; };

private:
    std::vector<ATLN> m_d{};
    unsigned int m_nObj{0}; // > 1 if array; 0 if error in retrieve.
};
} // end of MuonGM namespace

#endif // DBLQ00_ATLN_H

