# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#
# @file egammaD3PDAnalysis/python/egammaTruthParticleConfig.py
# @author scott snyder <snyder@bnl.gov>
# @date Mar, 2011
# @brief Configure algorithms to build filtered TruthParticleContainer
#        for egamma truth.
#


from D3PDMakerConfig.D3PDMakerFlags           import D3PDMakerFlags
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

D3PD = CompFactory.D3PD


def egammaTruthParticleCfg (flags,
                            algname = 'egammaTruthBuilder',
                            sequenceName = None,
                            **kwargs):

    acc = ComponentAccumulator()

    if 'ParticleCaloExtensionTool' not in kwargs:
        from TrkConfig.AtlasExtrapolatorConfig import \
            MCTruthClassifierExtrapolatorCfg
        extrapolator = acc.popToolsAndMerge \
            (MCTruthClassifierExtrapolatorCfg (flags))

        from TrackToCalo.TrackToCaloConfig import EMParticleCaloExtensionToolCfg
        extension = EMParticleCaloExtensionToolCfg \
            (flags, Extrapolator=extrapolator)

        kwargs['ParticleCaloExtensionTool'] = acc.popToolsAndMerge (extension)

    kwargs.setdefault ('InputKey', D3PDMakerFlags.TruthSGKey)
    kwargs.setdefault ('OutputKey', 'egammaTruth')
    kwargs.setdefault ('AuxPrefix', D3PDMakerFlags.EgammaUserDataPrefix)

    # From egammaD3PDAnalysis
    acc.addEventAlgo (D3PD.egammaTruthAlg (algname, **kwargs))
    return acc

