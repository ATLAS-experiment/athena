/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "FlavorTagDiscriminants/VRJetOverlapDecoratorTool.h"
#include "FlavorTagDiscriminants/HbbTagTool.h"
#include "FlavorTagDiscriminants/DL2Tool.h"
#include "FlavorTagDiscriminants/BTagAugmenterTool.h"
#include "FlavorTagDiscriminants/BTagMuonAugmenterTool.h"
#include "FlavorTagDiscriminants/BTagToJetLinkerAlg.h"
#include "FlavorTagDiscriminants/JetToBTagLinkerAlg.h"
#include "FlavorTagDiscriminants/BTagTrackLinkCopyAlg.h"
#include "FlavorTagDiscriminants/BTaggingBuilderAlg.h"
#include "FlavorTagDiscriminants/PoorMansIpAugmenterAlg.h"
#include "FlavorTagDiscriminants/TrackLeptonDecoratorAlg.h"
#include "FlavorTagDiscriminants/TruthParticleDecoratorAlg.h"
#include "FlavorTagDiscriminants/TrackTruthDecoratorAlg.h"
#include "FlavorTagDiscriminants/SoftElectronDecoratorAlg.h"
#include "FlavorTagDiscriminants/SoftElectronTruthDecoratorAlg.h"
#include "FlavorTagDiscriminants/GNNAuxTaskDecoratorAlg.h"
#include "FlavorTagDiscriminants/TrackClassifier.h"
#include "FlavorTagDiscriminants/FTagGhostElectronAssociationAlg.h"
#include "FlavorTagDiscriminants/HitDecoratorAlg.h"
#include "FlavorTagDiscriminants/JetHitAssociationAlg.h"

#include "src/CountIParticleAlg.h"
#include "src/CountTrackParticleAlg.h"


using namespace FlavorTagDiscriminants;

DECLARE_COMPONENT(VRJetOverlapDecoratorTool)
DECLARE_COMPONENT(HbbTagTool)
DECLARE_COMPONENT(DL2Tool)
DECLARE_COMPONENT(BTagAugmenterTool)
DECLARE_COMPONENT(BTagMuonAugmenterTool)
DECLARE_COMPONENT(BTagToJetLinkerAlg)
DECLARE_COMPONENT(JetToBTagLinkerAlg)
DECLARE_COMPONENT(BTagTrackLinkCopyAlg)
DECLARE_COMPONENT(BTaggingBuilderAlg)
DECLARE_COMPONENT(PoorMansIpAugmenterAlg)
DECLARE_COMPONENT(TrackLeptonDecoratorAlg)
DECLARE_COMPONENT(TruthParticleDecoratorAlg)
DECLARE_COMPONENT(TrackTruthDecoratorAlg)
DECLARE_COMPONENT(SoftElectronDecoratorAlg)
DECLARE_COMPONENT(SoftElectronTruthDecoratorAlg)
DECLARE_COMPONENT(TrackClassifier)


DECLARE_COMPONENT(GNNAuxTaskDecoratorAlg)
DECLARE_COMPONENT(CountIParticleAlg)
DECLARE_COMPONENT(CountTrackParticleAlg)
DECLARE_COMPONENT(FTagGhostElectronAssociationAlg)
DECLARE_COMPONENT(HitDecoratorAlg)
DECLARE_COMPONENT(JetHitAssociationAlg)
