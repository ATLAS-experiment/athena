# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
# @file TruthD3PDMaker/python/MCTruthClassifierConfig.py
# @author scott snyder <snyder@bnl.gov>
# @date Jan, 2010
# @brief Configure the MCTruthClassifier tool for D3PD making.
#

# Create the classifier tool, and set the MC collection name to match
# what's in D3PDMakerFlags.

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from D3PDMakerCoreComps.resolveSGKey      import resolveSGKey


def D3PDMCTruthClassifierCfg (flags):
    acc = ComponentAccumulator()

    from TrkConfig.AtlasExtrapolatorConfig import \
        MCTruthClassifierExtrapolatorCfg
    extrapolator = acc.popToolsAndMerge(MCTruthClassifierExtrapolatorCfg(flags))

    from TrackToCalo.TrackToCaloConfig import EMParticleCaloExtensionToolCfg
    extension = EMParticleCaloExtensionToolCfg(flags, Extrapolator=extrapolator)
    particleCaloExtensionTool = acc.popToolsAndMerge(extension)

    mckey = resolveSGKey (flags, flags.D3PD.TruthSGKey)
    
    acc.addPublicTool \
        (CompFactory.D3PD.D3PDMCTruthClassifier
         ('D3PDMCTruthClassifier',
          CaloDetDescrManager = 'CaloDetDescrManager',
          xAODTruthParticleContainerName = mckey,
          ParticleCaloExtensionTool = particleCaloExtensionTool,
          pTNeutralPartCut = 1e-3,
          partExtrConePhi = 0.6,  # 0.4
          partExtrConeEta = 0.2,  # 0.2
          ROICone = True))

    return acc
