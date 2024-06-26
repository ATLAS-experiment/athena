/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "../InDetTrackPerfMonTool.h"
#include "../TrackAnalysisDefinitionSvc.h"
#include "../TrackQualitySelectionTool.h"
#include "../TruthTrackQualitySelectionTool.h"
#include "../RoiSelectionTool.h"
#include "../TrackRoiSelectionTool.h"
#include "../TruthHitDecoratorAlg.h"
#include "../OfflineElectronDecoratorAlg.h"
#include "../OfflineMuonDecoratorAlg.h"
#include "../OfflineTauDecoratorAlg.h"
#include "../TrackObjectSelectionTool.h"
#include "../TrackTruthMatchingTool.h"
#include "../TruthTrackMatchingTool.h"
#include "../DeltaRMatchingTool.h"
#include "../PlotsDefinitionSvc.h"
#include "../JsonPlotsDefReadTool.h"

DECLARE_COMPONENT( InDetTrackPerfMonTool )
DECLARE_COMPONENT( TrackAnalysisDefinitionSvc )
DECLARE_COMPONENT( PlotsDefinitionSvc )
DECLARE_COMPONENT( IDTPM::JsonPlotsDefReadTool )
DECLARE_COMPONENT( IDTPM::TrackQualitySelectionTool )
DECLARE_COMPONENT( IDTPM::TruthTrackQualitySelectionTool )
DECLARE_COMPONENT( IDTPM::RoiSelectionTool )
DECLARE_COMPONENT( IDTPM::TrackRoiSelectionTool )
DECLARE_COMPONENT( IDTPM::TruthHitDecoratorAlg )
DECLARE_COMPONENT( IDTPM::OfflineElectronDecoratorAlg )
DECLARE_COMPONENT( IDTPM::OfflineMuonDecoratorAlg )
DECLARE_COMPONENT( IDTPM::OfflineTauDecoratorAlg )
DECLARE_COMPONENT( IDTPM::TrackObjectSelectionTool )
DECLARE_COMPONENT( IDTPM::TrackTruthMatchingTool )
DECLARE_COMPONENT( IDTPM::TruthTrackMatchingTool )
DECLARE_COMPONENT( IDTPM::DeltaRMatchingTool_trk )
DECLARE_COMPONENT( IDTPM::DeltaRMatchingTool_trkTruth )
DECLARE_COMPONENT( IDTPM::DeltaRMatchingTool_truthTrk )
