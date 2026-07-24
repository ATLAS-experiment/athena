# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration


def ActsCaloExtensionBuilderCfg(flags, name="ActsCaloExtensionBuilder", **kwargs):

    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    from AthenaConfiguration.ComponentFactory import CompFactory

    result = ComponentAccumulator()
    from ActsConfig.ActsGeometryConfig import ActsExtrapolationToolCfg
    kwargs.setdefault("ExtrapolationTool", result.popToolsAndMerge(ActsExtrapolationToolCfg(flags, 
                                                                                            MaxSteps=10000,
                                                                                            InteractionEloss = True,
                                                                                            InteractionMultiScatering = True)))

    the_alg = CompFactory.ActsTrk.CaloExtensionAlg(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result
