/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina

#include <TruthParticleLevelAnalysisAlgorithms/ParticleLevelJetsAlg.h>
#include <TruthParticleLevelAnalysisAlgorithms/ParticleLevelIsolationAlg.h>
#include <TruthParticleLevelAnalysisAlgorithms/ParticleLevelChargeDecoratorAlg.h>
#include <TruthParticleLevelAnalysisAlgorithms/ParticleLevelPtEtaPhiDecoratorAlg.h>
#include <TruthParticleLevelAnalysisAlgorithms/ParticleLevelMissingETAlg.h>
#include <TruthParticleLevelAnalysisAlgorithms/ParticleLevelOverlapRemovalAlg.h>

DECLARE_COMPONENT (CP::ParticleLevelJetsAlg)
DECLARE_COMPONENT (CP::ParticleLevelIsolationAlg)
DECLARE_COMPONENT (CP::ParticleLevelChargeDecoratorAlg)
DECLARE_COMPONENT (CP::ParticleLevelPtEtaPhiDecoratorAlg)
DECLARE_COMPONENT (CP::ParticleLevelMissingETAlg)
DECLARE_COMPONENT (CP::ParticleLevelOverlapRemovalAlg)
