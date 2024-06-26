/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/********************************************************
 Class def for MuonGeoModel DblQ00/ALIN
 *******************************************************/

#ifndef DBLQ00_ALIN_H
#define DBLQ00_ALIN_H
#include <string>
#include <vector>


class IRDBAccessSvc;

namespace MuonGM {
class DblQ00Alin {
public:
    DblQ00Alin() = default;
    ~DblQ00Alin() = default;
    DblQ00Alin(IRDBAccessSvc *pAccessSvc, 
               const std::string & GeoTag="", 
               const std::string & GeoNode="");

    DblQ00Alin & operator=(const DblQ00Alin &right) = default;
    DblQ00Alin(const DblQ00Alin&) = default;

    // data members for DblQ00/ALIN fields
    struct ALIN {
        int      version{0};             // VERSION
        float    dx{0.f};                   // X RELATIVE POSITION OF THE SUB-CUT
        float    dy{0.f};                 // Y RELATIVE POSITION OF THE SUB-CUT
        int      i{0};                 // SERIAL NB. OF THE OBJECT IN WHICH THE SU
        float    width_xs{0.f};         // S DIMENSIONS OF THE SUB-CUT
        float    width_xl{0.f};         // L DIMENSIONS OF THE SUB-CUT
        float    length_y{0.f};         // Y DIMENSIONS OF THE SUB-CUT
        float    excent{0.f};          // EXC DIMENSIONS OF THE SUB-CUT
        float    dead1{0.f};           // D1 DIMENSIONS OF THE SUB-CUT
        int      jtyp{0};                  // STATION TYPE
        int      indx{0};                // INDEX
        int      icut{0};                // CUT-OUT INDEX
    };
    
    const ALIN* data() const { return m_d.data(); };
    unsigned int size() const { return m_nObj; };
    std::string getName() const { return "ALIN"; };
    std::string getDirName() const { return "DblQ00"; };
    std::string getObjName() const { return "ALIN"; };

private:
    std::vector<ALIN> m_d;
    unsigned int m_nObj{0}; // > 1 if array; 0 if error in retrieve.
};
} // end of MuonGM namespace

#endif // DBLQ00_ALIN_H

