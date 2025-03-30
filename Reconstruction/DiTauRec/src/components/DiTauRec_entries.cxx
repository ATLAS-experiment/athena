#include "DiTauRec/DiTauBuilder.h"
#include "DiTauRec/DiTauToolBase.h"
#include "DiTauRec/SeedJetBuilder.h"
#include "DiTauRec/SubjetBuilder.h"
#include "DiTauRec/VertexFinder.h"
#include "DiTauRec/DiTauTrackFinder.h"
#include "DiTauRec/ClusterFinder.h"
#include "DiTauRec/CellFinder.h"
#include "DiTauRec/IDVarCalculator.h"
#include "DiTauRec/DiTauConstituentFinder.h"
#include "DiTauRec/DiTauIDVarDecorator.h"
#include "DiTauRec/DiTauOnnxDiscriminantTool.h"

DECLARE_COMPONENT( DiTauBuilder )
DECLARE_COMPONENT( SeedJetBuilder )
DECLARE_COMPONENT( SubjetBuilder )
DECLARE_COMPONENT( VertexFinder )
DECLARE_COMPONENT( DiTauTrackFinder )
DECLARE_COMPONENT( ClusterFinder )
DECLARE_COMPONENT( CellFinder )
DECLARE_COMPONENT( IDVarCalculator )
DECLARE_COMPONENT( DiTauConstituentFinder )
DECLARE_COMPONENT( DiTauIDVarDecorator )
DECLARE_COMPONENT( DiTauOnnxDiscriminantTool )