/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MSVTXVALIDATIONALGUTILS_H
#define MSVTXVALIDATIONALGUTILS_H

#include <vector>
#include <cmath>
#include <unordered_set>
#include <algorithm>

#include "CxxUtils/fpcompare.h"
#include "GaudiKernel/SystemOfUnits.h"
#include "TruthUtils/HepMCHelpers.h"
#include "TruthUtils/MagicNumbers.h"
#include "FourMomUtils/xAODP4Helpers.h"
#include "GeoPrimitives/GeoPrimitives.h"

#include "xAODTruth/TruthParticleContainer.h"
#include "xAODTruth/TruthVertexContainer.h"
#include "xAODTracking/VertexContainer.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODJet/JetContainer.h"


namespace MSVtxValidationAlgUtils {

    // particle properties
    double getCTau(const xAOD::TruthVertex *decVtx);
    // takes the fraction of energy deposited in the ECal and computes the log10 of the ratio between the energy deposited in the ECal and HCal
    double getCalEnergyLogRatio(double EMF); 
    bool comparePt(const xAOD::TruthParticle* part1, const xAOD::TruthParticle* part2);

    // decay chain utils
    std::vector<const xAOD::TruthParticle*> getChildren(const xAOD::TruthParticle* mother);
    std::vector<const xAOD::TruthParticle*> getStableChildren(const xAOD::TruthParticle* particle, bool findOnlyGenStable);
    std::vector<const xAOD::TruthParticle*> getStableChildrenRecursive(const xAOD::TruthParticle* particle, bool findOnlyGenStable, std::unordered_set<const xAOD::TruthParticle*>& visited);

    // displaced vertex approximation within jets
    struct JetVtxApprox {
        const xAOD::TruthVertex* vtx{nullptr};
        size_t nChildren{0};
        size_t decayDepth{0};
    };

    JetVtxApprox getJetVtxApprox(const xAOD::Jet* jet, const xAOD::TruthParticleContainer& truthParticles);

    // vertex isolation
    struct VtxIso {
        double track_mindR{-1.};
        double track_pTsum{-1.};
        double jet_mindR{-1.};
    };  

    VtxIso getIso(const xAOD::Vertex *MSVtx, const xAOD::TrackParticleContainer& Tracks, const xAOD::JetContainer& Jets, 
                  double trackIso_pT, double softTrackIso_R, double jetIso_pT, double jetIso_LogRatio);
}

#endif // MSVTXVALIDATIONALGUTILS_H
