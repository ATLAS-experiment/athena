/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "TrkAlignGenTools/MatrixTool.h"
#include "TrkAlignGenTools/IPCMatrixTool.h"
#include "TrkAlignGenTools/TrkAlignDBTool.h"
#include "TrkAlignGenTools/AnalyticalDerivCalcTool.h"
#include "TrkAlignGenTools/ShiftingDerivCalcTool.h"
#include "TrkAlignGenTools/AlignTrackPreProcessor.h"
#include "TrkAlignGenTools/TrackCollectionProvider.h"
#include "TrkAlignGenTools/AlignTrackCreator.h"
#include "TrkAlignGenTools/AlignTrackDresser.h"
#include "TrkAlignGenTools/AlignModuleTool.h"
#include "TrkAlignGenTools/AlignResidualCalculator.h"
#include "TrkAlignGenTools/BeamspotVertexPreProcessor.h"
#include "TrkAlignGenTools/ConstrainedTrackProvider.h"

DECLARE_COMPONENT( Trk::MatrixTool )
DECLARE_COMPONENT( Trk::IPCMatrixTool )
DECLARE_COMPONENT( Trk::TrkAlignDBTool )
DECLARE_COMPONENT( Trk::AnalyticalDerivCalcTool )
DECLARE_COMPONENT( Trk::ShiftingDerivCalcTool )
DECLARE_COMPONENT( Trk::AlignTrackPreProcessor )
DECLARE_COMPONENT( Trk::TrackCollectionProvider )
DECLARE_COMPONENT( Trk::AlignTrackCreator )
DECLARE_COMPONENT( Trk::AlignTrackDresser )
DECLARE_COMPONENT( Trk::AlignModuleTool )
DECLARE_COMPONENT( Trk::AlignResidualCalculator )
DECLARE_COMPONENT( Trk::BeamspotVertexPreProcessor )
DECLARE_COMPONENT( Trk::ConstrainedTrackProvider )

