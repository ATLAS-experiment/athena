#include "ParticleJetTools/CopyFlavorLabelTruthParticles.h"
#include "ParticleJetTools/CopyBosonTopLabelTruthParticles.h"
#include "ParticleJetTools/CopyTruthPartons.h"
#include "ParticleJetTools/JetPartonTruthLabel.h"
#include "ParticleJetTools/CopyTruthJetParticles.h"
#include "ParticleJetTools/ParticleJetDeltaRLabelTool.h"
#include "ParticleJetTools/ParticleJetGhostLabelTool.h"
#include "ParticleJetTools/JetParticleShrinkingConeAssociation.h"
#include "ParticleJetTools/JetParticleCenterOfMassAssociation.h"
#include "ParticleJetTools/JetParticleOriginVertexAssociation.h"
#include "ParticleJetTools/JetTruthLabelingTool.h"
#include "ParticleJetTools/JetPileupLabelingTool.h"
#include "ParticleJetTools/JetQuarkChargeLabelingTool.h"
#include "ParticleJetTools/FatVertex.h"
#include "ParticleJetTools/FtagLargeRJetTruthLabelTool.h"

#include "../TruthParentDecoratorAlg.h"
#ifndef GENERATIONBASE
// TruthVertexDecoratorAlg uses InDet::InDetTrackTruthOriginTool, whose package
// is not available in AthGeneration. JetTruthVertexSummaryDecoratorAlg is
// excluded together as a pair — both are derivation-only.
#include "../TruthVertexDecoratorAlg.h"
#include "../JetTruthVertexSummaryDecoratorAlg.h"
#endif

using namespace Analysis;


DECLARE_COMPONENT( Analysis::JetPartonTruthLabel )
/// @todo Convert to namespace, tool, etc?
DECLARE_COMPONENT( CopyFlavorLabelTruthParticles )
DECLARE_COMPONENT( CopyBosonTopLabelTruthParticles )
DECLARE_COMPONENT( CopyTruthPartons )
DECLARE_COMPONENT( CopyTruthJetParticles )
DECLARE_COMPONENT( ParticleJetDeltaRLabelTool )
DECLARE_COMPONENT( ParticleJetGhostLabelTool )
DECLARE_COMPONENT( JetParticleShrinkingConeAssociation )
DECLARE_COMPONENT( JetParticleCenterOfMassAssociation )
DECLARE_COMPONENT( JetParticleOriginVertexAssociation )
DECLARE_COMPONENT( JetTruthLabelingTool )
DECLARE_COMPONENT( JetPileupLabelingTool )
DECLARE_COMPONENT( JetQuarkChargeLabelingTool )

DECLARE_COMPONENT( TruthParentDecoratorAlg )
#ifndef GENERATIONBASE
DECLARE_COMPONENT( ParticleJetTools::TruthVertexDecoratorAlg )
DECLARE_COMPONENT( ParticleJetTools::JetTruthVertexSummaryDecoratorAlg )
#endif
DECLARE_COMPONENT( FtagLargeRJetTruthLabelTool )
