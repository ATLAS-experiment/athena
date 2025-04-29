# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from ElectronPhotonSelectorTools.AsgElectronLikelihoodToolsConfig import AsgElectronLikelihoodToolCfg
from MuonSelectorTools.MuonSelectorToolsConfig import MuonSelectionToolCfg
from ElectronPhotonSelectorTools.LikelihoodEnums import LikeEnum

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def TrackLeptonDecorationCfg(cfgFlags) -> ComponentAccumulator:
    """Decorate tracks with information about reconstructed leptons"""
    acc = ComponentAccumulator()

    electronID_tool = acc.popToolsAndMerge(
        AsgElectronLikelihoodToolCfg(cfgFlags, name="ftagElectronID", quality=LikeEnum.VeryLoose)
    )
    muonID_tool = acc.popToolsAndMerge( # loose quality selection
        MuonSelectionToolCfg(cfgFlags, name="ftagMuonID", MuQuality=2, MaxEta=2.5) 
    )
    acc.addEventAlgo(CompFactory.FlavorTagDiscriminants.TrackLeptonDecoratorAlg(
        'TrackLeptonDecoratorAlg',
        trackContainer="InDetTrackParticles",
        electronSelectionTool=electronID_tool,
        muonSelectionTool=muonID_tool,
    ))

    return acc
