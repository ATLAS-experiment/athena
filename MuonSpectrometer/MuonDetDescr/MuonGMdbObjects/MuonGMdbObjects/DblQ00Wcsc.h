/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/********************************************************
 Class def for MuonGeoModel DblQ00/WCSC
 *******************************************************/

 //  author: S Spagnolo
 // entered: 07/28/04
 // comment: CSC

#ifndef DBLQ00_WCSC_H
#define DBLQ00_WCSC_H

#include <string>
#include <vector>

class IRDBAccessSvc;

namespace MuonGM {
class DblQ00Wcsc {
public:
    DblQ00Wcsc() = default;
    ~DblQ00Wcsc() = default;
    DblQ00Wcsc(IRDBAccessSvc *pAccessSvc, const std::string & GeoTag="", const std::string & GeoNode="");
    DblQ00Wcsc & operator=(const DblQ00Wcsc &right) = delete;
    DblQ00Wcsc(const DblQ00Wcsc&)= delete;

    // data members for DblQ00/WCSC fields
    struct WCSC {
        int version{0}; // VERSION
        int jsta{0}; // JSTA TYPE NUMBER
        int laycsc{0}; // NBER OF CSCS LAYERS
        float ttotal{0.f}; // TOTAL THICKNESS
        float tnomex{0.f}; // NOMEX HONEYCOMB THICKNESS
        float tlag10{0.f}; // G10 LAMINATES THICKNESS
        float wispa{0.f}; // WIRE SPACING
        float dancat{0.f}; // ANODE-CATHODE DISTANCE
        float pcatre{0.f}; // CATHODE READOUT PITCH
        float gstrip{0.f}; // GAP BETWEEN CATHODE STRIPS
        float wrestr{0.f}; // WIDTH OF READOUT STRIPS
        float wflstr{0.f}; // WIDTH OF FLOATING STRIPS
        float trrwas{0.f}; // RIGIT RECTANGULAR WASHER THICKNES
        float wroxa{0.f}; // ROXACELL WIDTH
        float groxwi{0.f}; // ROXACELL AND WIRE BAR GAP
        float wgasba{0.f}; // FULL GAS GAP BAR WIDTH
        float tgasba{0.f}; // FULL GAS GAP BAR THICK.
        float wgascu{0.f}; // CUTS GAS GAP BAR WIDTH
        float tgascu{0.f}; // CUTS GAS GAP BAR THICK.
        float wfixwi{0.f}; // FULL WIRE FIX. BAR WID.
        float tfixwi{0.f}; // FULL WIRE FIX. BAR THICK.
        float pba1wi{0.f}; // WIRE BAR POSITION
        float pba2wi{0.f}; // WIRE BAR POSITION
        float pba3wi{0.f}; // WIRE BAR POSITION
        float psndco{0.f}; // 2ND COORDINATE PITCH
        float azcat{0.f};  // this actually is the 2ND COORDINATE PITCH
    };
    
    const WCSC* data() const { return m_d.data(); };
    unsigned int size() const { return m_nObj; };
    std::string getName() const { return "WCSC"; };
    std::string getDirName() const { return "DblQ00"; };
    std::string getObjName() const { return "WCSC"; };

private:
    std::vector<WCSC> m_d{};
    unsigned int m_nObj{0}; // > 1 if array; 0 if error in retrieve.
};
} // end of MuonGM namespace

#endif // DBLQ00_WCSC_H

