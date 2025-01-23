# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def BPHY_InDetV0FinderToolCfg(flags, derivation="",
                              V0ContainerName="",
                              KshortContainerName="",
                              LambdaContainerName="",
                              LambdabarContainerName="",
                              TrackParticleCollection="InDetTrackParticles",
                              RelinkTracks=[],
                              **kwargs):

    from InDetConfig.InDetV0FinderConfig import InDetV0FinderToolCfg
    return InDetV0FinderToolCfg(
        flags, name = derivation + "_InDetV0FinderTool",
        TrackParticleCollection = TrackParticleCollection,
        V0ContainerName = V0ContainerName,
        KshortContainerName = KshortContainerName,
        LambdaContainerName = LambdaContainerName,
        LambdabarContainerName = LambdabarContainerName,
        RelinkTracks = RelinkTracks,
        **kwargs)

def BPHY_V0MainDecoratorCfg(flags, derivation="",
                            V0ContainerName="",
                            KshortContainerName="",
                            LambdaContainerName="",
                            LambdabarContainerName="",
                            **kwargs):
    acc = ComponentAccumulator()

    if "V0Tools" not in kwargs:
        from DerivationFrameworkBPhys.commonBPHYMethodsCfg import BPHY_V0ToolCfg
        kwargs.setdefault("V0Tools", acc.popToolsAndMerge(
            BPHY_V0ToolCfg(flags, derivation)))

    from InDetConfig.InDetV0FinderConfig import V0MainDecoratorCfg
    acc.setPrivateTools(acc.popToolsAndMerge(V0MainDecoratorCfg(
        flags, name = derivation + "V0Decorator",
        V0ContainerName = V0ContainerName,
        KshortContainerName = KshortContainerName,
        LambdaContainerName = LambdaContainerName,
        LambdabarContainerName = LambdabarContainerName,
        **kwargs)))
    return acc

def BPHY_Reco_V0FinderCfg(flags, derivation="", suffix="",
                          V0ContainerName="",
                          KshortContainerName="",
                          LambdaContainerName="",
                          LambdabarContainerName="",
                          CheckVertexContainers=[],
                          **kwargs):
    acc = ComponentAccumulator()

    if "V0FinderTool" not in kwargs:
        kwargs.setdefault("V0FinderTool", acc.popToolsAndMerge(
            BPHY_InDetV0FinderToolCfg(
                flags, derivation,
                V0ContainerName = V0ContainerName,
                KshortContainerName = KshortContainerName,
                LambdaContainerName = LambdaContainerName,
                LambdabarContainerName = LambdabarContainerName)))

    if "Decorator" not in kwargs:
        kwargs.setdefault("Decorator", acc.popToolsAndMerge(
            BPHY_V0MainDecoratorCfg(
                flags, derivation,
                V0ContainerName = V0ContainerName,
                KshortContainerName = KshortContainerName,
                LambdaContainerName = LambdaContainerName,
                LambdabarContainerName = LambdabarContainerName)))

    acc.setPrivateTools(CompFactory.DerivationFramework.Reco_V0Finder(
        name = derivation + "_Reco_V0Finder" + suffix,
        V0ContainerName        = V0ContainerName,
        KshortContainerName    = KshortContainerName,
        LambdaContainerName    = LambdaContainerName,
        LambdabarContainerName = LambdabarContainerName,
        CheckVertexContainers  = CheckVertexContainers,
        **kwargs))
    return acc
