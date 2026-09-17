/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONR4_MUONFASTRECOEVENT_FASTRECOUTILS__H
#define MUONR4_MUONFASTRECOEVENT_FASTRECOUTILS__H

#include "MuonFastRecoEvent/GlobalPattern.h"
#include "xAODMuon/MuonSegment.h"
#include "xAODMuon/Muon.h"

namespace MuonR4::FastReco {

    /** @brief Retrieve the parent global pattern of the segment */
    const GlobalPattern* getParentPattern(const xAOD::MuonSegment& segment);
    /** @brief Retrieve the parent global pattern of the muon */
    const GlobalPattern* getParentPattern(const xAOD::Muon& muon);
    /** @brief Retrieve the Q/P covariance of the muon */
    double getQOverPCov(const xAOD::Muon& muon);
}

#endif