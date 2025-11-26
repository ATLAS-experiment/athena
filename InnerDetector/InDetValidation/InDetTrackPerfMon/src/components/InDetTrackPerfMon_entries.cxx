/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "../InDetTrackPerfMonTool.h"
#include "../TrackAnalysisDefinitionSvc.h"
#include "../TrackQualitySelectionTool.h"
#include "../VertexQualitySelectionTool.h"
#include "../TruthQualitySelectionTool.h"
#include "../RoiSelectionTool.h"
#include "../TrackRoiSelectionTool.h"
#include "../VertexRoiSelectionTool.h"
#include "../OfflineElectronDecoratorAlg.h"
#include "../OfflineMuonDecoratorAlg.h"
#include "../OfflineTauDecoratorAlg.h"
#include "../OfflineJetDecoratorAlg.h"
#include "../TruthDecoratorAlg.h"
#include "../TrackObjectSelectionTool.h"
#include "../OfflineTrackQualitySelectionTool.h"
#include "../TrackTruthMatchingTool.h"
#include "../TruthTrackMatchingTool.h"
#include "../EFTrackMatchingTool.h"
#include "../DeltaRMatchingTool.h"
#include "../StableDeltaRMatchingTool.h"
#include "../PlotsDefinitionSvc.h"
#include "../JsonPlotsDefReadTool.h"
#include "../TrackAnalysisInfoWriteTool.h"

DECLARE_COMPONENT( InDetTrackPerfMonTool )
DECLARE_COMPONENT( TrackAnalysisDefinitionSvc )
DECLARE_COMPONENT( PlotsDefinitionSvc )
DECLARE_COMPONENT( IDTPM::JsonPlotsDefReadTool )
DECLARE_COMPONENT( IDTPM::TrackQualitySelectionTool )
DECLARE_COMPONENT( IDTPM::VertexQualitySelectionTool )
DECLARE_COMPONENT( IDTPM::TruthQualitySelectionTool )
DECLARE_COMPONENT( IDTPM::RoiSelectionTool )
DECLARE_COMPONENT( IDTPM::TrackRoiSelectionTool )
DECLARE_COMPONENT( IDTPM::VertexRoiSelectionTool )
DECLARE_COMPONENT( IDTPM::OfflineElectronDecoratorAlg )
DECLARE_COMPONENT( IDTPM::OfflineMuonDecoratorAlg )
DECLARE_COMPONENT( IDTPM::OfflineTauDecoratorAlg )
DECLARE_COMPONENT( IDTPM::OfflineJetDecoratorAlg )
DECLARE_COMPONENT( IDTPM::TruthDecoratorAlg )
DECLARE_COMPONENT( IDTPM::TrackObjectSelectionTool )
DECLARE_COMPONENT( IDTPM::OfflineTrackQualitySelectionTool )
DECLARE_COMPONENT( IDTPM::TrackTruthMatchingTool )
DECLARE_COMPONENT( IDTPM::TruthTrackMatchingTool )
DECLARE_COMPONENT( IDTPM::EFTrackMatchingTool )
DECLARE_COMPONENT( IDTPM::DeltaRMatchingTool_trk )
DECLARE_COMPONENT( IDTPM::StableDeltaRMatchingTool_trk )
DECLARE_COMPONENT( IDTPM::DeltaRMatchingTool_trkTruth )
DECLARE_COMPONENT( IDTPM::StableDeltaRMatchingTool_trkTruth )
DECLARE_COMPONENT( IDTPM::DeltaRMatchingTool_truthTrk )
DECLARE_COMPONENT( IDTPM::StableDeltaRMatchingTool_truthTrk )
DECLARE_COMPONENT( IDTPM::TrackAnalysisInfoWriteTool )
