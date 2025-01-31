#include "DerivationFrameworkLLP/JetLargeD0TrackParticleThinning.h"
#include "DerivationFrameworkLLP/VSITrackParticleThinning.h"
#include "DerivationFrameworkLLP/RCJetSubstructureAug.h"
#include "DerivationFrameworkLLP/TrackParametersKVU.h"
#include "DerivationFrameworkLLP/PixeldEdxTrackParticleThinning.h"
#include "DerivationFrameworkLLP/TrackParticleCaloCellDecorator.h"
#include "DerivationFrameworkLLP/AugmentationToolLeadingJets.h"
#include "DerivationFrameworkLLP/DESDM_EXOTHIP_SkimmingTool.h"

using namespace DerivationFramework;

DECLARE_COMPONENT( JetLargeD0TrackParticleThinning )
DECLARE_COMPONENT( VSITrackParticleThinning )
DECLARE_COMPONENT( RCJetSubstructureAug )
DECLARE_COMPONENT( TrackParametersKVU )
DECLARE_COMPONENT( PixeldEdxTrackParticleThinning )
DECLARE_COMPONENT( TrackParticleCaloCellDecorator )
DECLARE_COMPONENT( AugmentationToolLeadingJets )
DECLARE_COMPONENT( DESDM_EXOTHIP_SkimmingTool )

