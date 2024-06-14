/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/********************************************************
 Class def for MuonGeoModel DblQ00/DBAM
 *******************************************************/

 //  author: S Spagnolo
 // entered: 07/28/04
 // comment: MUON GEOMETRY FROM AMDB

#ifndef DBLQ00_DBAM_H
#define DBLQ00_DBAM_H

#include <string>
#include <vector>
#include <array>

class IRDBAccessSvc;


namespace MuonGM {
class DblQ00Dbam {
public:
    DblQ00Dbam() = default;
    ~DblQ00Dbam() = default;
    DblQ00Dbam(IRDBAccessSvc *pAccessSvc, const std::string & GeoTag="", const std::string & GeoNode="");
    DblQ00Dbam & operator=(const DblQ00Dbam &right) = default;
    DblQ00Dbam(const DblQ00Dbam&)  = default;


    // data members for DblQ00/DBAM fields
    struct DBAM {
        int version{0};         // VERSION
        int nvrs{0};            // VERSION OF READING
        std::string amdb{};     // AMDB NAME
        int mtyp{0};            // MAXIMUM STATION NUMBER
        std::array<std::string, 53> name{}; // STATION NAME
        int numbox{0};          // FILE INDEX
    };
    
    const DBAM* data() const { return m_d.data(); };
    unsigned int size() const { return m_nObj; };
    std::string getName() const { return "DBAM"; };
    std::string getDirName() const { return "DblQ00"; };
    std::string getObjName() const { return "DBAM"; };
    
private:
    std::vector<DBAM> m_d{};
    unsigned int m_nObj{0}; // > 1 if array; 0 if error in retrieve.
};
} // end of MuonGM namespace

#endif // DBLQ00_DBAM_H

