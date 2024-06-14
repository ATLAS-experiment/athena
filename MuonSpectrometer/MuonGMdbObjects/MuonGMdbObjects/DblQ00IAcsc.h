/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/********************************************************
 Class def for MuonGeoModel DblQ00/IACSC
 *******************************************************/

 //  author: S Spagnolo
 // entered: 07/28/04
 // comment: CSC internal alignment parameters - class to read from DB

#ifndef DBLQ00_IACSC_H
#define DBLQ00_IACSC_H



class IRDBAccessSvc;
#include <string>
#include <vector>


namespace MuonGM {
class DblQ00IAcsc {
public:
    DblQ00IAcsc() = default;
    ~DblQ00IAcsc() = default;
    DblQ00IAcsc(IRDBAccessSvc *pAccessSvc, const std::string & GeoTag="", const std::string & GeoNode="");
    DblQ00IAcsc(const std::string& asciiFileName);

    DblQ00IAcsc & operator=(const DblQ00IAcsc &right) = delete;
    DblQ00IAcsc(const DblQ00IAcsc&) = delete;

    
    void WriteIAcscToAsciiFile(const std::string& filename);

    // data members for DblQ00/IACSC fields
    struct IACSC {
        int version{0}; // VERSION
        int line{0}; // LINE NUMBER
        std::string type{}; // STATION TYPE
        int jff{0}; // PHI POSITION
        int jzz{0}; // Z POSITION
        int job{0}; // JOB POSITION
        int wireLayer{0}; // JOB POSITION
        float tras{0.f}; // S TRANSLATION [MM]
        float traz{0.f}; // Z TRANSLATION
        float trat{0.f}; // T TRANSLATION
        float rots{0.f}; // S ROTATION [RAD]
        float rotz{0.f}; // Z ROTATION
        float rott{0.f}; // T ROTATION
        int i{0}; // STATION AMDB INDEX
    };

    const IACSC* data() const { return m_d.data(); };
    unsigned int size() const { return m_nObj; };
    std::string getName() const { return "IACSC"; };
    std::string getDirName() const { return "DblQ00"; };
    std::string getObjName() const { return "IACSC"; };

private:
    std::vector<IACSC> m_d{};
    unsigned int m_nObj{0}; // > 1 if array; 0 if error in retrieve.
};


} // end of MuonGM namespace

#endif // DBLQ00_ASZT_H

