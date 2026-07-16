# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration


def ActsCaloExtensionBuilderCfg(flags, name="ActsCaloExtensionBuilder", **kwargs):

    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    from AthenaConfiguration.ComponentFactory import CompFactory

    result = ComponentAccumulator()
    from ActsConfig.ActsGeometryConfig import ActsTrackingGeometryToolCfg
    kwargs.setdefault("TrackingGeometryTool", result.getPrimaryAndMerge(ActsTrackingGeometryToolCfg(flags)))
    the_alg = CompFactory.ActsTrk.CaloExtensionAlg(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result
