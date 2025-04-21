
/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "../DetailedMuonPatternTruthBuilder.h"
#include "../MuonDecayTruthTrajectoryBuilder.h"
#include "../MuonSegmentTruthAssociationAlg.h"




#include "../MuonTruthSegmentCreationAlg.h"
#include "MuonTruthAlgs/MuonDetailedTrackTruthMaker.h"
#include "MuonTruthAlgs/MuonPRD_MultiTruthMaker.h"
#include "MuonTruthAlgs/MuonPatternCombinationDetailedTrackTruthMaker.h"
#include "MuonTruthAlgs/MuonTrackTruthTool.h"


#include "../TruthMuonMakerAlg.h"
#include "../TruthHitSummaryAlg.h"
#include "../TruthTrackRecordsAlg.h"
#include "../RecoToTruthAssociationAlg.h"
using namespace Muon;
using namespace Trk;

DECLARE_COMPONENT(MuonPRD_MultiTruthMaker)
DECLARE_COMPONENT(MuonDetailedTrackTruthMaker)
DECLARE_COMPONENT(MuonPatternCombinationDetailedTrackTruthMaker)

DECLARE_COMPONENT(MuonTruthSegmentCreationAlg)

DECLARE_COMPONENT(MuonSegmentTruthAssociationAlg)

DECLARE_COMPONENT(MuonTrackTruthTool)
DECLARE_COMPONENT(MuonDecayTruthTrajectoryBuilder)
DECLARE_COMPONENT(DetailedMuonPatternTruthBuilder)


DECLARE_COMPONENT(Muon::TruthMuonMakerAlg)
DECLARE_COMPONENT(Muon::TruthHitSummaryAlg)
DECLARE_COMPONENT(Muon::TruthTrackRecordsAlg)
DECLARE_COMPONENT(Muon::RecoToTruthAssociationAlg)