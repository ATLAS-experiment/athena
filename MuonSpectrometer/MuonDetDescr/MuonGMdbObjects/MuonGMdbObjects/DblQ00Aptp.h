/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/********************************************************
 Class def for MuonGeoModel DblQ00/APTP
 *******************************************************/

 //  author: S Spagnolo
 // entered: 07/28/04
 // comment: POSITION OF STATION

#ifndef DBLQ00_APTP_H
#define DBLQ00_APTP_H

#include <string> 
#include <vector> 
#include <array>
class IRDBAccessSvc;


namespace MuonGM {
class DblQ00Aptp {
public:
    DblQ00Aptp() = default;
    ~DblQ00Aptp() = default;
    DblQ00Aptp(IRDBAccessSvc *pAccessSvc, const std::string & GeoTag="", const std::string & GeoNode="");

    DblQ00Aptp & operator=(const DblQ00Aptp &right) = delete;
    DblQ00Aptp(const DblQ00Aptp&) = delete;

    // data members for DblQ00/APTP fields
    struct APTP {
        int   version{0};           // VERSION
        int   line{0};              // LINE NUMBER
        std::string type{};         // STATION TYPE
        int   i{0};                 // STATION AMDB INDEX
        int   icut{0};              // CUT-OUT INDEX,ZERO IF MISSING
        std::string iphi{};         // PHI INDICATES OF OCTANTS
        int   iz{0};                // Z (FOR BARREL) OR R (FOR END-CAPS) POS.
        float dphi{0.f};            // RELATIVE PHI POSITION OF THE STATION IN
        float z{0.f};               // Z POSITION OF THE LOWEST Z EDGE OF THE S
        float r{0.f};               // RADIAL POSITION OF ITS INNERMOST EDGE
        float s{0.f};               // ORTHO-RADIAL POSITION OF THE CENTER OF T
        float alfa{0.f};            // ALFA ANGLE DEFINING THE DEVIATION [GRAD]
        float beta{0.f};            // BETA ANGLE DEFINING THE DEVIATION
        float gamma{0.f};           // GAMMA ANGLE DEFINING THE DEVIATION
    };

    const APTP* data() const { return m_d.data(); };
    unsigned int size() const { return m_nObj; };
    std::string getName() const { return "APTP"; };
    std::string getDirName() const { return "DblQ00"; };
    std::string getObjName() const { return "APTP"; };

private:
    std::vector<APTP> m_d{};
    unsigned int m_nObj{0}; // > 1 if array; 0 if error in retrieve.
};
}// end of MuonGM namespace

#endif // DBLQ00_APTP_H

