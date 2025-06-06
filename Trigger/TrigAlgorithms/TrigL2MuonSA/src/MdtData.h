/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef  TRIGL2MUONSA_MDTDATA_H
#define  TRIGL2MUONSA_MDTDATA_H

#include <array>
#include "Identifier/Identifier.h"
#include "MuonReadoutGeometry/MdtReadoutElement.h"

namespace TrigL2MuonSA {

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

  struct MdtHitData
  {
    unsigned int name{0};
    int StationEta{0};
    int StationPhi{0};
    int Multilayer{0};
    int Layer{0};
    int TubeLayer{0};
    int Tube{0};
    int Chamber{0};
    //
    double cYmid{0.};
    double cXmid{0.};
    double cAmid{0.};
    double cPhip{0.};
    double cInCo{0.};
    double cPhi0{0.};
    std::array<char,4>   cType{};
    //
    double Z{0.};
    double R{0.};
    double DriftTime{0.};
    double DriftSpace{0.};
    double DriftSigma{0.};
    uint32_t OnlineId{0};
    uint16_t Adc{0};
    uint16_t LeadingCoarseTime{0};
    uint16_t LeadingFineTime{0};
    uint16_t TrailingCoarseTime{0};
    uint16_t TrailingFineTime{0};
    //
    double Residual{0.};
    int    isOutlier{0};
    Identifier Id{};
    const MuonGM::MdtReadoutElement* readEle{nullptr};
};
  
// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

typedef std::vector<MdtHitData>  MdtHits;

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

}

#endif  // TRIGL2MUONSA_MDTDATA_H
