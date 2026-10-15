#include "AssociationUtils/OverlapRemovalTool.h"
#include "AssociationUtils/DeltaROverlapTool.h"
#include "AssociationUtils/EleEleOverlapTool.h"
#include "AssociationUtils/EleJetOverlapTool.h"
#include "AssociationUtils/EleMuSharedTrkOverlapTool.h"
#include "AssociationUtils/MuJetOverlapTool.h"
#include "AssociationUtils/AltMuJetOverlapTool.h"
#include "AssociationUtils/ObjLinkOverlapTool.h"
#include "AssociationUtils/TauLooseEleOverlapTool.h"
#include "AssociationUtils/TauLooseMuOverlapTool.h"
#include "AssociationUtils/TauJetOverlapTool.h"
#include "AssociationUtils/TauAntiTauJetOverlapTool.h"
#include "AssociationUtils/OverlapRemovalGenUseAlg.h"
#include "AssociationUtils/FEAssociationTool.h"
#include "AssociationUtils/FEAssociationAlg.h"
#include "AssociationUtils/FEOverlapTool.h"

#ifndef XAOD_STANDALONE
#include "../OverlapRemovalTestAlg.h"
#include "../ORToolBoxTestAlg.h"
#endif

// Project include(s).
#include "AsgTools/AsgComponentFactories.h"

DECLARE_COMPONENT( ADD_CMODE(ORUtils)::OverlapRemovalTool )
DECLARE_COMPONENT( ADD_CMODE(ORUtils)::DeltaROverlapTool )
DECLARE_COMPONENT( ADD_CMODE(ORUtils)::EleEleOverlapTool )
DECLARE_COMPONENT( ADD_CMODE(ORUtils)::EleJetOverlapTool )
DECLARE_COMPONENT( ADD_CMODE(ORUtils)::EleMuSharedTrkOverlapTool )
DECLARE_COMPONENT( ADD_CMODE(ORUtils)::MuJetOverlapTool )
DECLARE_COMPONENT( ADD_CMODE(ORUtils)::AltMuJetOverlapTool )
DECLARE_COMPONENT( ADD_CMODE(ORUtils)::ObjLinkOverlapTool )
DECLARE_COMPONENT( ADD_CMODE(ORUtils)::TauLooseEleOverlapTool )
DECLARE_COMPONENT( ADD_CMODE(ORUtils)::TauLooseMuOverlapTool )
DECLARE_COMPONENT( ADD_CMODE(ORUtils)::TauJetOverlapTool )
DECLARE_COMPONENT( ADD_CMODE(ORUtils)::TauAntiTauJetOverlapTool )
DECLARE_COMPONENT( ADD_CMODE(ORUtils)::OverlapRemovalGenUseAlg )
DECLARE_COMPONENT( ADD_CMODE(ORUtils)::FEAssociationTool )
DECLARE_COMPONENT( ADD_CMODE(ORUtils)::FEAssociationAlg )
DECLARE_COMPONENT( ADD_CMODE(ORUtils)::FEOverlapTool )

#ifndef XAOD_STANDALONE
DECLARE_COMPONENT( ADD_CMODE(ORUtils)::OverlapRemovalTestAlg )
DECLARE_COMPONENT( ADD_CMODE(ORUtils)::ORToolBoxTestAlg )
#endif
