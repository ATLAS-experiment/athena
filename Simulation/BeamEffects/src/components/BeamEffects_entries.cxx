#include "../GenEventValidityChecker.h"
#include "../ZeroLifetimePositioner.h"
#include "../GenEventVertexPositioner.h"
#include "../VertexBeamCondPositioner.h"
#include "../LRAVertexPositioner.h"
#include "../LongBeamspotVertexPositioner.h"
#include "../CrabKissingVertexPositioner.h"
#include "../GenEventBeamEffectBooster.h"
#include "../GenEventRotator.h"
#include "../BeamEffectsAlg.h"
#include "../BeamSpotFixerAlg.h"
#include "../BeamSpotReweightingAlg.h"

#ifndef SIMULATIONBASE
#include "../MatchingBkgVertexPositioner.h"
#endif

DECLARE_COMPONENT( Simulation::ZeroLifetimePositioner )
DECLARE_COMPONENT( Simulation::GenEventValidityChecker )
DECLARE_COMPONENT( Simulation::GenEventVertexPositioner )
DECLARE_COMPONENT( Simulation::VertexBeamCondPositioner )
DECLARE_COMPONENT( Simulation::LRAVertexPositioner )
DECLARE_COMPONENT( Simulation::LongBeamspotVertexPositioner )
DECLARE_COMPONENT( Simulation::CrabKissingVertexPositioner )
DECLARE_COMPONENT( Simulation::GenEventBeamEffectBooster )
DECLARE_COMPONENT( Simulation::GenEventRotator )
DECLARE_COMPONENT( Simulation::BeamEffectsAlg )
DECLARE_COMPONENT( Simulation::BeamSpotFixerAlg )
DECLARE_COMPONENT( Simulation::BeamSpotReweightingAlg )

#ifndef SIMULATIONBASE
DECLARE_COMPONENT( Simulation::MatchingBkgVertexPositioner )
#endif
