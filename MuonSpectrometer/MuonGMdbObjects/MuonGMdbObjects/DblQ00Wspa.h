/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/********************************************************
 Class def for MuonGeoModel DblQ00/WSPA
 *******************************************************/

 //  author: S Spagnolo
 // entered: 07/28/04
 // comment: RPC SPACER

#ifndef DBLQ00_WSPA_H
#define DBLQ00_WSPA_H
#include <string> 
#include <vector> 

class IRDBAccessSvc;


namespace MuonGM {
class DblQ00Wspa {
public:
    DblQ00Wspa() = default;
    ~DblQ00Wspa() = default;
    DblQ00Wspa(IRDBAccessSvc *pAccessSvc, const std::string & GeoTag="", const std::string & GeoNode="");
    DblQ00Wspa & operator=(const DblQ00Wspa &right) = delete;
    DblQ00Wspa(const DblQ00Wspa&)= delete;

    // data members for DblQ00/WSPA fields
    struct WSPA {
        int version{0}; // VERSION
        int jsta{0}; // JSTA INDEX
        int nb{0}; // NUMBER OF DETAILS
        float x0{0.f}; // X0
        float tckspa{0.f}; // THICKNESS OF SPACER
    };
    
    const WSPA* data() const { return m_d.data(); };
    unsigned int size() const { return m_nObj; };
    std::string getName() const { return "WSPA"; };
    std::string getDirName() const { return "DblQ00"; };
    std::string getObjName() const { return "WSPA"; };

private:
  std::vector<WSPA> m_d{};
  unsigned int m_nObj{0}; // > 1 if array; 0 if error in retrieve.

};
} // end of MuonGM namespace

#endif // DBLQ00_WSPA_H

