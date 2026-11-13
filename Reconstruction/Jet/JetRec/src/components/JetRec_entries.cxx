#include "JetRec/JetAlgorithm.h"
#include "JetRec/JetRecAlg.h"
#include "JetRec/JetViewAlg.h"
#include "JetRec/JetTrimming.h"
#include "JetRec/JetSoftDropTools.h"

#include "JetRec/PseudoJetMerger.h"
#include "JetRec/PseudoJetAlgorithm.h"
#include "JetRec/MuonSegmentPseudoJetAlgorithm.h"

#include "JetRec/JetFromPseudojet.h"
#include "JetRec/FastJetInterfaceTool.h"

#include "JetRec/JetSorter.h"
#include "JetRec/JetPseudojetRetriever.h"
#include "JetRec/JetPseudojetCopier.h"
#include "JetRec/JetFilterTool.h"
#include "JetRec/JetClusterer.h"
#include "JetRec/JetClustererByVertex.h"

#include "JetRec/JetCopier.h"

#include "JetRec/JetDecorationAlg.h"

DECLARE_COMPONENT( JetFromPseudojet )
DECLARE_COMPONENT( JetSorter )
DECLARE_COMPONENT( JetFilterTool )
DECLARE_COMPONENT( FastJetInterfaceTool )
DECLARE_COMPONENT( JetPseudojetRetriever )
DECLARE_COMPONENT( JetPseudojetCopier )
DECLARE_COMPONENT( JetClusterer )
DECLARE_COMPONENT( JetClustererByVertex )
DECLARE_COMPONENT( JetCopier )
DECLARE_COMPONENT( PseudoJetMerger )

DECLARE_COMPONENT( JetAlgorithm )
DECLARE_COMPONENT( JetRecAlg )
DECLARE_COMPONENT( JetDecorationAlg )
DECLARE_COMPONENT( JetViewAlg )
DECLARE_COMPONENT( PseudoJetAlgorithm )
DECLARE_COMPONENT( MuonSegmentPseudoJetAlgorithm )

DECLARE_COMPONENT( JetGrooming::JetTrimming )
DECLARE_COMPONENT( JetGrooming::SoftDrop )
DECLARE_COMPONENT( JetGrooming::RecSoftDrop )
DECLARE_COMPONENT( JetGrooming::BottomUpSoftDrop )
