/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "../AnalysisMuonThinningAlg.h"
#include "../IDTrackCaloDepositsDecoratorAlg.h"
#include "../TrackIsolationDecorAlg.h"
#include "../CaloIsolationDecorAlg.h"
#include "../PflowIsolationDecorAlg.h"
#include "../MuonJetDrTool.h"
#include "../MuonTPExtrapolationAlg.h"
#include "../MuonTruthClassifierFallback.h"
#include "../MuonTruthIsolationDecorAlg.h"
#include "../DiMuonTaggingAlg.h"

DECLARE_COMPONENT(DerivationFramework::MuonTruthClassifierFallback)
DECLARE_COMPONENT(DerivationFramework::MuonTruthIsolationDecorAlg)
DECLARE_COMPONENT(DerivationFramework::MuonJetDrTool)
DECLARE_COMPONENT(DerivationFramework::DiMuonTaggingAlg)
DECLARE_COMPONENT(DerivationFramework::AnalysisMuonThinningAlg)
DECLARE_COMPONENT(DerivationFramework::IDTrackCaloDepositsDecoratorAlg)
DECLARE_COMPONENT(DerivationFramework::TrackIsolationDecorAlg)
DECLARE_COMPONENT(DerivationFramework::CaloIsolationDecorAlg)
DECLARE_COMPONENT(DerivationFramework::PflowIsolationDecorAlg)
DECLARE_COMPONENT(DerivationFramework::IDTrackCaloDepositsDecoratorAlg)
DECLARE_COMPONENT(DerivationFramework::MuonTPExtrapolationAlg)
