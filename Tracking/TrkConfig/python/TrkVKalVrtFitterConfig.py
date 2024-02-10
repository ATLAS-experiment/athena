# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
# Configuration of TrkVKalVrtFitter package

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def TrkVKalVrtFitterCfg(flags, name="TrkVKalVrtFitter", **kwargs):
    from MagFieldServices.MagFieldServicesConfig import (
        AtlasFieldCacheCondAlgCfg)
    acc = AtlasFieldCacheCondAlgCfg(flags) # To produce AtlasFieldCacheCondObj

    if "Extrapolator" not in kwargs:
        from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
        kwargs.setdefault("Extrapolator", acc.popToolsAndMerge(
            AtlasExtrapolatorCfg(flags)))

    acc.setPrivateTools(CompFactory.Trk.TrkVKalVrtFitter(name, **kwargs))
    return acc

def Conversion_TrkVKalVrtFitterCfg(
        flags, name="Conversion_TrkVKalVrtFitter", **kwargs):

    kwargs.setdefault("FirstMeasuredPoint", True)
    kwargs.setdefault("FirstMeasuredPointLimit", True)
    kwargs.setdefault("InputParticleMasses", [0.511, 0.511])
    kwargs.setdefault("IterationNumber", 30)
    kwargs.setdefault("MakeExtendedVertex", True)
    kwargs.setdefault("Robustness", 6)
    kwargs.setdefault("usePhiCnst", True)
    kwargs.setdefault("useThetaCnst", True)
    kwargs.setdefault("CovVrtForConstraint",
                      [0.015*0.015, 0., 0.015*0.015, 0., 0., 10000.*10000.])
    kwargs.setdefault("VertexForConstraint", [0., 0., 0.])

    return TrkVKalVrtFitterCfg(flags, name, **kwargs)

def InDetTrkVKalVrtFitterCfg(flags, name="InDetTrkVKalVrtFitter", **kwargs):
    acc = ComponentAccumulator()

    if "Extrapolator" not in kwargs:
        from TrkConfig.AtlasExtrapolatorConfig import InDetExtrapolatorCfg
        kwargs.setdefault("Extrapolator", acc.popToolsAndMerge(
            InDetExtrapolatorCfg(flags)))
    
    acc.setPrivateTools(acc.popToolsAndMerge(
        TrkVKalVrtFitterCfg(flags, name, **kwargs)))
    return acc

def TrigVKalVrtFitterCfg(flags, name="TrigVKalVrtFitter", **kwargs):
    kwargs.setdefault("IterationNumber",    30)
    return InDetTrkVKalVrtFitterCfg(flags, name, **kwargs)

def BPHY_TrkVKalVrtFitterCfg(flags, name="BPHY_TrkVKalVrtFitter", **kwargs):
    kwargs.setdefault("MakeExtendedVertex", True)
    return InDetTrkVKalVrtFitterCfg(flags, name, **kwargs)

def V0VKalVrtFitterCfg(flags, name="V0VKalVrtFitter", **kwargs):
    kwargs.setdefault("IterationNumber",    30)
    return BPHY_TrkVKalVrtFitterCfg(flags, name, **kwargs)

def JpsiV0VertexFitCfg(flags, name="JpsiV0VertexFit", **kwargs):
    kwargs.setdefault("CascadeCnstPrecision", 1e-6)
    return BPHY_TrkVKalVrtFitterCfg(flags, name, **kwargs)

def BTAG_TrkVKalVrtFitterCfg(flags, name="BTAG_TrkVKalVrtFitter", **kwargs):
    from MagFieldServices.MagFieldServicesConfig import (
        AtlasFieldCacheCondAlgCfg)
    acc = AtlasFieldCacheCondAlgCfg(flags) # To produce AtlasFieldCacheCondObj
    kwargs.setdefault("FirstMeasuredPoint", False)
    kwargs.setdefault("FrozenVersionForBTagging", True)
    # No extrapolator explicitly configured, default constructor used
    acc.setPrivateTools(CompFactory.Trk.TrkVKalVrtFitter(name, **kwargs))
    return acc

def TrkVKalVrtFitterTestAlgCfg(flags, name="TrkVKalVrtFitterTestAlg", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("Tool", acc.popToolsAndMerge(TrkVKalVrtFitterCfg(flags)))
    acc.addEventAlgo(CompFactory.Trk.TrkVKalVrtFitterTestAlg(name, **kwargs))
    return acc


if __name__ == "__main__":

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()

    from AthenaConfiguration.TestDefaults import defaultTestFiles
    flags.Input.Files = defaultTestFiles.RDO_RUN2

    flags.lock()
    flags.dump()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg=MainServicesCfg(flags)
    cfg.merge(PoolReadCfg(flags))

    cfg.merge(TrkVKalVrtFitterTestAlgCfg(flags))
    cfg.run(2)
