/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/********************************************************
 Class def for MuonGeoModel DblQ00/ASZT
 *******************************************************/

 //  author: S Spagnolo
 // entered: 07/28/04
 // comment: MUON STATION ELEMENT

#ifndef DBLQ00_ASZT_H
#define DBLQ00_ASZT_H



class IRDBAccessSvc;
#include <string>
#include <vector>
#include <array>

namespace MuonGM {
class DblQ00Aszt {
public:
    DblQ00Aszt() = default;
    ~DblQ00Aszt() = default;
    DblQ00Aszt(IRDBAccessSvc *pAccessSvc, const std::string & GeoTag="", const std::string & GeoNode="");
    DblQ00Aszt & operator=(const DblQ00Aszt &right) = delete;
    DblQ00Aszt(const DblQ00Aszt&) = delete;

    DblQ00Aszt(const std::string& asciiFileName);
    
    void WriteAsztToAsciiFile(const std::string& filename);

    // data members for DblQ00/ASZT fields
    struct ASZT {
        int version{0}; // VERSION
        int line{0}; // LINE NUMBER
        std::string type{}; // STATION TYPE
        int jff{0}; // PHI POSITION
        int jzz{0}; // Z POSITION
        int job{0}; // JOB POSITION
        float tras{0.f}; // S TRANSLATION [MM]
        float traz{0.f}; // Z TRANSLATION
        float trat{0.f}; // T TRANSLATION
        float rots{0.f}; // S ROTATION [RAD]
        float rotz{0.f}; // Z ROTATION
        float rott{0.f}; // T ROTATION
        int i{0}; // STATION AMDB INDEX
    };

    const ASZT* data() const { return m_d.data(); };
    unsigned int size() const { return m_nObj; };
    std::string getName() const { return "ASZT"; };
    std::string getDirName() const { return "DblQ00"; };
    std::string getObjName() const { return "ASZT"; };

private:
    std::vector<ASZT> m_d{};
    unsigned int m_nObj{0}; // > 1 if array; 0 if error in retrieve.
};


} // end of MuonGM namespace

#endif // DBLQ00_ASZT_H

