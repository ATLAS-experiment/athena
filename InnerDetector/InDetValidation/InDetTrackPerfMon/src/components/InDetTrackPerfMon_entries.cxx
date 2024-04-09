/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "../InDetTrackPerfMonTool.h"
#include "../TrackAnalysisDefinitionSvc.h"
#include "../TrackQualitySelectionTool.h"
#include "../RoiSelectionTool.h"
#include "../TrackRoiSelectionTool.h"
#include "../TruthHitDecoratorAlg.h"
#include "../OfflineElectronDecoratorAlg.h"
#include "../OfflineMuonDecoratorAlg.h"
#include "../OfflineTauDecoratorAlg.h"
#include "../TrackObjectSelectionTool.h"
#include "../TrackTruthMatchingTool.h"
/// TODO - To be included in later MRs
//#include "InDetTrackPerfMon/DeltaRtrackMatchingTool_trk.h"
//#include "InDetTrackPerfMon/DeltaRtrackMatchingTool_trkTruth.h"
//#include "InDetTrackPerfMon/HistogramDefinitionSvc.h"
//#include "InDetTrackPerfMon/ReadJsonHistoDefTool.h"

DECLARE_COMPONENT( InDetTrackPerfMonTool )
DECLARE_COMPONENT( TrackAnalysisDefinitionSvc )
DECLARE_COMPONENT( IDTPM::TrackQualitySelectionTool )
DECLARE_COMPONENT( IDTPM::RoiSelectionTool )
DECLARE_COMPONENT( IDTPM::TrackRoiSelectionTool )
DECLARE_COMPONENT( IDTPM::TruthHitDecoratorAlg )
DECLARE_COMPONENT( IDTPM::OfflineElectronDecoratorAlg )
DECLARE_COMPONENT( IDTPM::OfflineMuonDecoratorAlg )
DECLARE_COMPONENT( IDTPM::OfflineTauDecoratorAlg )
DECLARE_COMPONENT( IDTPM::TrackObjectSelectionTool )
DECLARE_COMPONENT( IDTPM::TrackTruthMatchingTool )
/// TODO - To be included in later MRs
//DECLARE_COMPONENT( ReadJsonHistoDefTool )
//DECLARE_COMPONENT( IDTPM::HistogramDefinitionSvc )
//DECLARE_COMPONENT( IDTPM::DeltaRtrackMatchingTool_trk )
//DECLARE_COMPONENT( IDTPM::DeltaRtrackMatchingTool_trkTruth )
