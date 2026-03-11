/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "InDetTrackPerfMon/InDetTrackPerfMonTool.h"
#include "InDetTrackPerfMon/TrackAnalysisDefinitionSvc.h"
#include "InDetTrackPerfMon/TrackQualitySelectionTool.h"
#include "InDetTrackPerfMon/VertexQualitySelectionTool.h"
#include "InDetTrackPerfMon/TruthQualitySelectionTool.h"
#include "InDetTrackPerfMon/RoiSelectionTool.h"
#include "InDetTrackPerfMon/TrackRoiSelectionTool.h"
#include "InDetTrackPerfMon/VertexRoiSelectionTool.h"
#include "InDetTrackPerfMon/OfflineElectronDecoratorAlg.h"
#include "InDetTrackPerfMon/OfflineMuonDecoratorAlg.h"
#include "InDetTrackPerfMon/OfflineTauDecoratorAlg.h"
#include "InDetTrackPerfMon/OfflineJetDecoratorAlg.h"
#include "InDetTrackPerfMon/TruthDecoratorAlg.h"
#include "InDetTrackPerfMon/TrackObjectSelectionTool.h"
#include "InDetTrackPerfMon/OfflineTrackQualitySelectionTool.h"
#include "InDetTrackPerfMon/TrackTruthMatchingTool.h"
#include "InDetTrackPerfMon/TruthTrackMatchingTool.h"
#include "InDetTrackPerfMon/EFTrackMatchingTool.h"
#include "InDetTrackPerfMon/DeltaRMatchingTool.h"
#include "InDetTrackPerfMon/PlotsDefinitionSvc.h"
#include "InDetTrackPerfMon/JsonPlotsDefReadTool.h"
#include "InDetTrackPerfMon/TrackAnalysisInfoWriteTool.h"
#include "InDetTrackPerfMon/TrackAnalysis.h"
#include "InDetTrackPerfMon/StableDeltaRMatchingTool.h"

DECLARE_COMPONENT( IDTPM::TrackAnalysis )
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
