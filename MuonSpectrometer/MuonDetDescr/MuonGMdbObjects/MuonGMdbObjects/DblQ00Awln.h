/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/********************************************************
 Class def for MuonGeoModel DblQ00/AWLN
 *******************************************************/

 //  author: S Spagnolo
 // entered: 07/28/04
 // comment: RPC DETAILS

#ifndef DBLQ00_AWLN_H
#define DBLQ00_AWLN_H
#include <string>
#include <vector>


class IRDBAccessSvc;

namespace MuonGM {
class DblQ00Awln {
public:
    DblQ00Awln() = default;
    ~DblQ00Awln() = default;
    DblQ00Awln(IRDBAccessSvc *pAccessSvc, const std::string & GeoTag="", const std::string & GeoNode="");
    
    // data members for DblQ00/AWLN fields
    struct AWLN {
        int version{0};     // VERSION
        int jsta{0};        // JSTA TYPE NUMBER
        float spitch{0.f};  // S-STRIPS PITCH
        float zpitch{0.f};  // Z-STRIPS PITCH
        float dedstr{0.f};  // DEAD REAGION BETWEEN STRIP
        int nsrest{0};      // NBER OF S STRIPS READOUTS
        int nzrest{0};      // NBER OF S GAS GAPS
        float sfirst{0};    // S-PHI STRIP OFFSET
        float zfirst{0};    // Z-ETA STRIP OFFSET
        float dedsep{0.f};  // DEAD SEPARATION
        int nsrost{0};      // NUMBER OF S-PHI READOUT STRIPS
        int nzrost{0};      // NUMBER OF Z-ETA READOUT STRIPS
    };
    
    const AWLN* data() const { return m_d.data(); };
    unsigned int size() const { return m_nObj; };
    std::string getName() const { return "AWLN"; };
    std::string getDirName() const { return "DblQ00"; };
    std::string getObjName() const { return "AWLN"; };

private:
    std::vector<AWLN> m_d;
    unsigned int m_nObj{}; // > 1 if array; 0 if error in retrieve.
    DblQ00Awln & operator=(const DblQ00Awln &right);
    DblQ00Awln(const DblQ00Awln&);
};
} // end of MuonGM namespace

#endif // DBLQ00_AWLN_H

