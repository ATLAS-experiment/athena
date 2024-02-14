# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.CfgGetter import addAlgorithm
## Algorithms
addAlgorithm("BeamEffects.BeamEffectsConfigLegacy.getBeamSpotFixerAlg",             "BeamSpotFixerAlg")
addAlgorithm("BeamEffects.BeamEffectsConfigLegacy.getBeamSpotReweightingAlg",       "BeamSpotReweightingAlg")

