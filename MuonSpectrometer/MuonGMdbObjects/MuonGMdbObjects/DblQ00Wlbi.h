/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/********************************************************
 Class def for MuonGeoModel DblQ00/WLBI
 *******************************************************/

 //  author: S Spagnolo
 // entered: 07/28/04
 // comment: LONG BEAM

#ifndef DBLQ00_WLBI_H
#define DBLQ00_WLBI_H

#include <string>
#include <vector>

class IRDBAccessSvc;


namespace MuonGM {
class DblQ00Wlbi {
public:
    DblQ00Wlbi() = default;
    ~DblQ00Wlbi() = default;
    DblQ00Wlbi(IRDBAccessSvc *pAccessSvc, const std::string & GeoTag="", const std::string & GeoNode="");

    DblQ00Wlbi & operator=(const DblQ00Wlbi &right) = default;
    DblQ00Wlbi(const DblQ00Wlbi&) = default;


    struct WLBI {
        int version{0};       // VERSION
        int jsta{0};          // INDEX
        int num{0};           // NUMBER OF OBJECTS
        float height{0.f};    // HEIGHT
        float thickness{0.f}; // WALL THICKNESS
        float lowerThickness{0.f}; // 
        float yShift{0.f}; // 
    };

    const WLBI* data() const { return m_d.data(); };
    unsigned int size() const { return m_nObj; };
    std::string getName() const { return "WLBI"; };
    std::string getDirName() const { return "DblQ00"; };
    std::string getObjName() const { return "WLBI"; };

private:
    std::vector<WLBI> m_d{};
    unsigned int m_nObj{0}; // > 1 if array; 0 if error in retrieve.
};
} // end of MuonGM namespace

#endif // DBLQ00_WLBI_H

