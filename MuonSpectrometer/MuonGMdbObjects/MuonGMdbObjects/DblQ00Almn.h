/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/********************************************************
 Class def for MuonGeoModel DblQ00/ALMN
 *******************************************************/

 //  author: S Spagnolo
 // entered: 07/28/04
 // comment: MUON STATION ELEMENT

#ifndef DBLQ00_ALMN_H
#define DBLQ00_ALMN_H
#include <string>
#include <vector>
#include <array>

class IRDBAccessSvc;


namespace MuonGM {
class DblQ00Almn {
public:
    DblQ00Almn() = default;
    ~DblQ00Almn() = default;
    DblQ00Almn(IRDBAccessSvc *pAccessSvc, const std::string & GeoTag="", const std::string & GeoNode="");

    DblQ00Almn & operator=(const DblQ00Almn &right) = default;
    DblQ00Almn(const DblQ00Almn&) = default;

    
    // data members for DblQ00/ALMN fields
    struct ALMN {
        int version{0}; // VERSION
        int i{0}; // ELEMENT NUMBER
        float dx{0.f}; // X RELATIVE POSITION OF THE OBJECT
        float dy{0.f}; // Y RELATIVE POSITION OF THE OBJECT
        float dz{0.f}; // Z RELATIVE POSITION OF THE OBJECT
        int job{0}; // OBJECT SERIAL NUMBER IN THE STATION
        std::string tec{}; // TYPE OF TECHNOLOGY USED FOR THE OBJECT
        int iw{0}; // INNER STRUCTURE TYPE INDEX, ISTA
        int isplit_x{0}; // NUMBER OF SECTIONS IN X
        int isplit_y{0}; // NUMBER OF SECTIONS IN Y
        int ishape{0}; // TYPE OF GEOMETRICAL SHAPE 0-TRAPEZOIDAL,
        float width_xs{0.f}; // S WIDTH, WS
        float width_xl{0.f}; // L WIDTH, WL
        float length_y{0.f}; // Y LENGTH, LE
        float excent{0.f}; // ADDITIONAL INFORMATION, EX
        float dead1{0.f}; // FIRST DEAD MATERIAL, D1
        float dead2{0.f}; // SECOND DEAD MATERIAL, D2
        float dead3{0.f}; // STEPS, D3
        int jtyp{0}; // STATION TYPE
        int indx{0}; // ELEMENT NUMBER
    };

    const ALMN* data() const { return m_d.data(); };
    unsigned int size() const { return m_nObj; };
    std::string getName() const { return "ALMN"; };
    std::string getDirName() const { return "DblQ00"; };
    std::string getObjName() const { return "ALMN"; };

private:
    std::vector<ALMN> m_d{};
    unsigned int m_nObj{0}; // > 1 if array; 0 if error in retrieve.
};


} // end of MuonGM namespace

#endif // DBLQ00_ALMN_H

