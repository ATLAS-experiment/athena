/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/********************************************************
 Class def for MuonGeoModel DblQ00/WDED
 *******************************************************/

 //  author: S Spagnolo
 // entered: 07/28/04
 // comment: RPC PANEL

#ifndef DBLQ00_WDED_H
#define DBLQ00_WDED_H

#include <string>
#include <vector>

class IRDBAccessSvc;


namespace MuonGM {
class DblQ00Wded {
public:
    DblQ00Wded() = default;
    ~DblQ00Wded() = default;
    DblQ00Wded(IRDBAccessSvc *pAccessSvc, const std::string & GeoTag="", const std::string & GeoNode="");
    DblQ00Wded & operator=(const DblQ00Wded &right) = delete;
    DblQ00Wded(const DblQ00Wded&) = delete;


    // data members for DblQ00/WDED fields
    struct WDED {
        int version{0}; // VERSION
        int jsta{0}; // INDEX
        int nb{0}; // NUMBER OF DETAILS
        float x0{0.f}; // X0
        float auphcb{0.f}; // HONEYCOMB THICKNESS
        float tckded{0.f}; // ALUMINIUM THICKNESS
    };
    
    const WDED* data() const { return m_d.data(); };
    unsigned int size() const { return m_nObj; };
    std::string getName() const { return "WDED"; };
    std::string getDirName() const { return "DblQ00"; };
    std::string getObjName() const { return "WDED"; };

private:
    std::vector<WDED> m_d{};
    unsigned int m_nObj{0}; // > 1 if array; 0 if error in retrieve.
};
} // end of MuonGM namespace


#endif // DBLQ00_WDED_H

