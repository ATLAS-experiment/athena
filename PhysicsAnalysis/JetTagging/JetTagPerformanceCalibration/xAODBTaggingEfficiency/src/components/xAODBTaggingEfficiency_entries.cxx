

#include "xAODBTaggingEfficiency/BTaggingEfficiencyTool.h"
#include "xAODBTaggingEfficiency/BTaggingSelectionTool.h"
#include "xAODBTaggingEfficiency/BTaggingTruthTaggingTool.h"
#include "xAODBTaggingEfficiency/BTaggingEigenVectorRecompositionTool.h"
#include "xAODBTaggingEfficiency/BTaggingSelectionJsonTool.h"
#include "xAODBTaggingEfficiency/BTaggingEfficiencyJsonTool.h"

#ifndef XAOD_STANDALONE
#include "../ToolTester.h"
#endif
// Should probably alter the namespace

// Project include(s).
#include "AsgTools/AsgComponentFactories.h"

DECLARE_COMPONENT( BTaggingEfficiencyTool )
DECLARE_COMPONENT( BTaggingSelectionTool )
DECLARE_COMPONENT( BTaggingTruthTaggingTool )
DECLARE_COMPONENT( BTaggingEigenVectorRecompositionTool )
DECLARE_COMPONENT( BTaggingSelectionJsonTool )
DECLARE_COMPONENT( BTaggingEfficiencyJsonTool )

#ifndef XAOD_STANDALONE
DECLARE_COMPONENT( BTagToolTester )
#endif
