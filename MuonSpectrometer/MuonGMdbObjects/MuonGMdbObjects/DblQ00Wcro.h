/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/********************************************************
 Class def for MuonGeoModel DblQ00/WCRO
 *******************************************************/

 //  author: S Spagnolo
 // entered: 07/28/04
 // comment: CRO SPACER

#ifndef DBLQ00_WCRO_H
#define DBLQ00_WCRO_H

#include <string>
#include <vector>

class IRDBAccessSvc;

namespace MuonGM {
class DblQ00Wcro {
public:
    DblQ00Wcro() = default;
    ~DblQ00Wcro() = default;
    DblQ00Wcro(IRDBAccessSvc *pAccessSvc, const std::string & GeoTag="", const std::string & GeoNode="");

    DblQ00Wcro & operator=(const DblQ00Wcro &right) = default;
    DblQ00Wcro(const DblQ00Wcro&)= default;

    
    // data members for DblQ00/WCRO fields
    struct WCRO {
        int version{0};         // VERSION
        int jsta{0};            // INDEX
        int num{0};             // NUMBER OF OBJECTS
        float heightness{0.f};  // HEIGHT
        float largeness{0.f};   // T-SHAPE LARGENESS
        float thickness{0.f};   // T-SHAPE THICKNESS
    };
    
    const WCRO* data() const { return m_d.data(); };
    unsigned int size() const { return m_nObj; };
    std::string getName() const { return "WCRO"; };
    std::string getDirName() const { return "DblQ00"; };
    std::string getObjName() const { return "WCRO"; };

private:
    std::vector<WCRO> m_d{};
    unsigned int m_nObj{0}; // > 1 if array; 0 if error in retrieve.
};
} // end of MuonGM namespace

#endif // DBLQ00_WCRO_H

