
#include "../TrigIDR4Mon.h"
#include "../TrackAnalysis.h"
#include "../GenericTrackSelector.h"
#include "../TrackSelector.h"
#include "../TruthTrackSelector.h"
#include "../ParticleSelector.h"
#include "../TrackViewMatchingTool.h"

DECLARE_COMPONENT( TrigIDR4Mon )
DECLARE_COMPONENT( IDTPM2::TrackAnalysis )
DECLARE_COMPONENT( IDTPM2::GenericTrackSelector )
DECLARE_COMPONENT( IDTPM2::TrackSelector )
DECLARE_COMPONENT( IDTPM2::TruthTrackSelector )
DECLARE_COMPONENT( IDTPM2::ParticleSelector )
DECLARE_COMPONENT_WITH_ID( IDTPM2::TrackViewMatchingTool, "IDTPM2::TrackViewMatchingTool" )

