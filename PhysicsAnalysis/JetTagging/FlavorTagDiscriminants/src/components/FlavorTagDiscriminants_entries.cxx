/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "FlavorTagDiscriminants/VRJetOverlapDecoratorTool.h"
#include "FlavorTagDiscriminants/HbbTagTool.h"
#include "FlavorTagDiscriminants/DL2Tool.h"
#include "FlavorTagDiscriminants/PoorMansIpAugmenterAlg.h"
#include "FlavorTagDiscriminants/TrackLeptonDecoratorAlg.h"
#include "FlavorTagDiscriminants/TruthParticleDecoratorAlg.h"
#include "FlavorTagDiscriminants/TrackTruthDecoratorAlg.h"
#include "FlavorTagDiscriminants/SoftElectronDecoratorAlg.h"
#include "FlavorTagDiscriminants/SoftElectronTruthDecoratorAlg.h"
#include "FlavorTagDiscriminants/GNNAuxTaskDecoratorAlg.h"
#include "FlavorTagDiscriminants/TrackClassifier.h"
#include "FlavorTagDiscriminants/FTagGhostLeptonAssociationAlg.h"
#include "FlavorTagDiscriminants/HitDecoratorAlg.h"
#include "FlavorTagDiscriminants/JetHitAssociationAlg.h"
#include "FlavorTagDiscriminants/JetLeptonDecayLabelAlg.h"
#include "FlavorTagDiscriminants/CaloChargedFlowDecoratorAlg.h"
#include "FlavorTagDiscriminants/JetCalibratedPtDecoratorAlg.h"
#include "FlavorTagDiscriminants/TruthTauDecoratorAlg.h"

#include "src/CountIParticleAlg.h"
#include "src/CountTrackParticleAlg.h"


DECLARE_COMPONENT(FlavorTagDiscriminants::VRJetOverlapDecoratorTool)
DECLARE_COMPONENT(FlavorTagDiscriminants::HbbTagTool)
DECLARE_COMPONENT(FlavorTagDiscriminants::DL2Tool)
DECLARE_COMPONENT(FlavorTagDiscriminants::PoorMansIpAugmenterAlg)
DECLARE_COMPONENT(FlavorTagDiscriminants::TrackLeptonDecoratorAlg)
DECLARE_COMPONENT(FlavorTagDiscriminants::TruthParticleDecoratorAlg)
DECLARE_COMPONENT(FlavorTagDiscriminants::TrackTruthDecoratorAlg)
DECLARE_COMPONENT(FlavorTagDiscriminants::SoftElectronDecoratorAlg)
DECLARE_COMPONENT(FlavorTagDiscriminants::SoftElectronTruthDecoratorAlg)
DECLARE_COMPONENT(FlavorTagDiscriminants::TrackClassifier)


DECLARE_COMPONENT(FlavorTagDiscriminants::GNNAuxTaskDecoratorAlg)
DECLARE_COMPONENT(FlavorTagDiscriminants::CountIParticleAlg)
DECLARE_COMPONENT(FlavorTagDiscriminants::CountTrackParticleAlg)
DECLARE_COMPONENT(FlavorTagDiscriminants::FTagGhostElectronAssociationAlg)
DECLARE_COMPONENT(FlavorTagDiscriminants::FTagGhostMuonAssociationAlg)
DECLARE_COMPONENT(FlavorTagDiscriminants::HitDecoratorAlg)
DECLARE_COMPONENT(FlavorTagDiscriminants::JetHitAssociationAlg)
DECLARE_COMPONENT(FlavorTagDiscriminants::JetLeptonDecayLabelAlg)
DECLARE_COMPONENT(FlavorTagDiscriminants::CaloChargedFlowDecoratorAlg)
DECLARE_COMPONENT(FlavorTagDiscriminants::JetCalibratedPtDecoratorAlg)
DECLARE_COMPONENT(FlavorTagDiscriminants::TruthTauDecoratorAlg)

#ifndef XAOD_ANALYSIS
#include "FlavorTagDiscriminants/HitBeamSpotDataDecoratorAlg.h"
DECLARE_COMPONENT(FlavorTagDiscriminants::HitBeamSpotDataDecoratorAlg)
#endif
