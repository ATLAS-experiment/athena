/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/********************************************************
 Class def for MuonGeoModel DblQ00/WRPC
 *******************************************************/

 //  author: S Spagnolo
 // entered: 07/28/04
 // comment: RPC GENERAL

#ifndef DBLQ00_WRPC_H
#define DBLQ00_WRPC_H
#include <string>
#include <vector>
#include <array>
class IRDBAccessSvc;


namespace MuonGM {
class DblQ00Wrpc {
public:
    DblQ00Wrpc() = default;
    ~DblQ00Wrpc() = default;
    DblQ00Wrpc(IRDBAccessSvc *pAccessSvc,const std::string & GeoTag="", const std::string & GeoNode="");
    DblQ00Wrpc & operator=(const DblQ00Wrpc &right) = delete;
    DblQ00Wrpc(const DblQ00Wrpc&) = delete;
    // data members for DblQ00/WRPC fields
    struct WRPC {
        int version{0}; // VERSION
        int nvrs{0}; // VERSION OF RPC TECHNOLOGY
        int layrpc{0}; // LAYERS NUMBER
        float tckrla{0.f}; // THICK. OF AN RPC LAYER
        float tottck{0.f}; // TOTAL THICKNESS
        float tckfsp{0.f}; // THICK. OF FOAM SPACER
        float ackfsp{0.f}; // THICK. OF AL PLATE OF FOAM SPACER
        float tlohcb{0.f}; // THICK. OF LOWER HONEYCOMB
        float alohcb{0.f}; // THICK. OF AL PLATE OF LOWER HONEYCOMB
        float tckbak{0.f}; // THICK. OF BAKELITE
        float tckgas{0.f}; // THICK. OF GAS GAP
        float tckssu{0.f}; // THICK. OF STRIPS SUPPORT
        float tckstr{0.f}; // THICK. OF STRIPS
        float sdedmi{0.f}; // S INTERNAL MID-CHBER DEAD REGION
        float zdedmi{0.f}; // Z INTERNAL MID-CHBER DEAD REGION
        float spdiam{0.f}; // SPACER DIAMETER
        float sppitc{0.f}; // SPACER PITCH
        std::array<float, 3> stroff{}; // STRIP OFFSET S, FIRST Z, SECOND Z
    };
    
    const WRPC* data() const { return m_d.data(); };
    unsigned int size() const { return m_nObj; };
    std::string getName() const { return "WRPC"; };
    std::string getDirName() const { return "DblQ00"; };
    std::string getObjName() const { return "WRPC"; };

private:
    std::vector<WRPC> m_d{};
    unsigned int m_nObj{0}; // > 1 if array; 0 if error in retrieve.

};
} // end of MuonGM namespace

#endif // DBLQ00_WRPC_H

