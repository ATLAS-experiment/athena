#include "TauAnalysisTools/CommonEfficiencyTool.h"
#include "TauAnalysisTools/CommonSmearingTool.h"
#include "TauAnalysisTools/TauSelectionTool.h"
#include "TauAnalysisTools/TauSmearingTool.h"
#include "TauAnalysisTools/TauTruthMatchingTool.h"
#include "TauAnalysisTools/TauTruthTrackMatchingTool.h"
#include "TauAnalysisTools/TauEfficiencyCorrectionsTool.h"
#include "TauAnalysisTools/BuildTruthTaus.h"
#include "TauAnalysisTools/DiTauTruthMatchingTool.h"
#include "TauAnalysisTools/CommonDiTauEfficiencyTool.h"
#include "TauAnalysisTools/CommonDiTauSmearingTool.h"
#include "TauAnalysisTools/DiTauSelectionTool.h"
#include "TauAnalysisTools/DiTauSmearingTool.h"
#include "TauAnalysisTools/DiTauEfficiencyCorrectionsTool.h"

#ifndef XAOD_STANDALONE
#include "../TauAnalysisToolsExampleAthena.h"
#endif

// Project include(s).
#include "AsgTools/AsgComponentFactories.h"

DECLARE_COMPONENT( TauAnalysisTools::CommonEfficiencyTool )
DECLARE_COMPONENT( TauAnalysisTools::CommonSmearingTool )
DECLARE_COMPONENT( TauAnalysisTools::TauSelectionTool )
DECLARE_COMPONENT( TauAnalysisTools::TauSmearingTool )
DECLARE_COMPONENT( TauAnalysisTools::TauTruthMatchingTool )
DECLARE_COMPONENT( TauAnalysisTools::TauTruthTrackMatchingTool )
DECLARE_COMPONENT( TauAnalysisTools::TauEfficiencyCorrectionsTool )
DECLARE_COMPONENT( TauAnalysisTools::BuildTruthTaus )
DECLARE_COMPONENT( TauAnalysisTools::DiTauTruthMatchingTool )
DECLARE_COMPONENT( TauAnalysisTools::CommonDiTauEfficiencyTool )
DECLARE_COMPONENT( TauAnalysisTools::CommonDiTauSmearingTool )
DECLARE_COMPONENT( TauAnalysisTools::DiTauSelectionTool )
DECLARE_COMPONENT( TauAnalysisTools::DiTauSmearingTool )
DECLARE_COMPONENT( TauAnalysisTools::DiTauEfficiencyCorrectionsTool )

#ifndef XAOD_STANDALONE
DECLARE_COMPONENT( TauAnalysisTools::TauAnalysisToolsExampleAthena )
#endif

