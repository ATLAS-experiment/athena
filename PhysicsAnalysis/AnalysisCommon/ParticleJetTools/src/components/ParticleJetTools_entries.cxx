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

#include "../TruthParentDecoratorAlg.h"

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
DECLARE_COMPONENT( TruthParentDecoratorAlg )
