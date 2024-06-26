/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/********************************************************
 Class def for MuonGeoModel DblQ00/WCMI
 *******************************************************/

 //  author: S Spagnolo
 // entered: 07/28/04
 // comment: CMI SPACER

#ifndef DBLQ00_WCMI_H
#define DBLQ00_WCMI_H

#include <string>
#include <vector>

class IRDBAccessSvc;


namespace MuonGM {
class DblQ00Wcmi {
public:
    DblQ00Wcmi() = default;
    ~DblQ00Wcmi() = default;
    DblQ00Wcmi(IRDBAccessSvc *pAccessSvc, const std::string & GeoTag="", const std::string & GeoNode="");

    // data members for DblQ00/WCMI fields
    struct WCMI {
        int version{0}; // VERSION
        int jsta{0}; // INDEX
        int num{0}; // NUMBER OF OBJECTS
        float heightness{0.f}; // HEIGHT
        float largeness{0.f}; // T-SHAPE LARGENESS
        float thickness{0.f}; // T-SHAPE THICKNESS
    };
    
    const WCMI* data() const { return m_d.data(); };
    unsigned int size() const { return m_nObj; };
    std::string getName() const { return "WCMI"; };
    std::string getDirName() const { return "DblQ00"; };
    std::string getObjName() const { return "WCMI"; };

private:
    std::vector<WCMI> m_d{};
    unsigned int m_nObj{0}; // > 1 if array; 0 if error in retrieve.
};
} // end of MuonGM namespace
    

#endif // DBLQ00_WCMI_H

