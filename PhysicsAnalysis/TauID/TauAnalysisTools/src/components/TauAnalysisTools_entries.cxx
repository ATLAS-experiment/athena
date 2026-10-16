#include "TauAnalysisTools/CommonEfficiencyTool.h"
#include "TauAnalysisTools/CommonSmearingTool.h"
#include "TauAnalysisTools/TauSelectionTool.h"
#include "TauAnalysisTools/TauSmearingTool.h"
#include "TauAnalysisTools/TauResolutionTool.h"
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
#include "../BuildTruthTausAlg.h"
#endif

// Project include(s).
#include "AsgTools/AsgComponentFactories.h"

DECLARE_COMPONENT( ADD_CMODE(TauAnalysisTools)::CommonEfficiencyTool )
DECLARE_COMPONENT( ADD_CMODE(TauAnalysisTools)::CommonSmearingTool )
DECLARE_COMPONENT( ADD_CMODE(TauAnalysisTools)::TauSelectionTool )
DECLARE_COMPONENT( ADD_CMODE(TauAnalysisTools)::TauSmearingTool )
DECLARE_COMPONENT( ADD_CMODE(TauAnalysisTools)::TauResolutionTool )	
DECLARE_COMPONENT( ADD_CMODE(TauAnalysisTools)::TauTruthMatchingTool )
DECLARE_COMPONENT( ADD_CMODE(TauAnalysisTools)::TauTruthTrackMatchingTool )
DECLARE_COMPONENT( ADD_CMODE(TauAnalysisTools)::TauEfficiencyCorrectionsTool )
DECLARE_COMPONENT( ADD_CMODE(TauAnalysisTools)::BuildTruthTaus )
DECLARE_COMPONENT( ADD_CMODE(TauAnalysisTools)::DiTauTruthMatchingTool )
DECLARE_COMPONENT( ADD_CMODE(TauAnalysisTools)::CommonDiTauEfficiencyTool )
DECLARE_COMPONENT( ADD_CMODE(TauAnalysisTools)::CommonDiTauSmearingTool )
DECLARE_COMPONENT( ADD_CMODE(TauAnalysisTools)::DiTauSelectionTool )
DECLARE_COMPONENT( ADD_CMODE(TauAnalysisTools)::DiTauSmearingTool )
DECLARE_COMPONENT( ADD_CMODE(TauAnalysisTools)::DiTauEfficiencyCorrectionsTool )

#ifndef XAOD_STANDALONE
DECLARE_COMPONENT( ADD_CMODE(TauAnalysisTools)::TauAnalysisToolsExampleAthena )
DECLARE_COMPONENT( ADD_CMODE(TauAnalysisTools)::BuildTruthTausAlg )
#endif

