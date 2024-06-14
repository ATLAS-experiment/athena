/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/********************************************************
 Class def for MuonGeoModel DblQ00/WMDT
 *******************************************************/

 //  author: S Spagnolo
 // entered: 07/28/04
 // comment: MDT

#ifndef DBLQ00_WMDT_H
#define DBLQ00_WMDT_H

#include <string>
#include <vector>
#include <array>

class IRDBAccessSvc;


namespace MuonGM {
class DblQ00Wmdt {
public:
    DblQ00Wmdt() = default;
    ~DblQ00Wmdt() = default;
    DblQ00Wmdt(IRDBAccessSvc *pAccessSvc, const std::string & GeoTag="", const std::string & GeoNode="");

    DblQ00Wmdt & operator=(const DblQ00Wmdt &right)=delete;
    DblQ00Wmdt(const DblQ00Wmdt&)=delete;

    // data members for DblQ00/WMDT fields
    struct WMDT {
        int version{0}; // VERSION
        std::string typ{}; // NAME
        int iw{0}; // INDEX
        float x0{0.f}; // X0
        int laymdt{0}; // MAXIMUM LAYER NUMBER
        float tubpit{0.f}; // PITCH BETWEEN TUBE
        float tubrad{0.f}; // RADIUS OF TUBE
        float tubsta{0.f}; // THICKNESS OF TUBE
        float tubdea{0.f}; // DEAD LENGTH IN TUBES
        std::array<float, 4> tubxco{}; // Y TUBE POSITION
        std::array<float, 4> tubyco{}; // X TUBE POSITION
        float tubwal{0.f}; // TUBE WALL THICKNESS
    };
    
    const WMDT* data() const { return m_d.data(); };
    unsigned int size() const { return m_nObj; };
    std::string getName() const { return "WMDT"; };
    std::string getDirName() const { return "DblQ00"; };
    std::string getObjName() const { return "WMDT"; };
    
private:
    std::vector<WMDT> m_d{};
    unsigned int m_nObj{0}; // > 1 if array; 0 if error in retrieve.
};
} // end of MuonGM namespace

#endif // DBLQ00_WMDT_H

