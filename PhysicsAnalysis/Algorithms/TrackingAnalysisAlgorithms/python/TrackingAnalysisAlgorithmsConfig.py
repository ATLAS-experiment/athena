# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def InDetSecVtxTruthMatchToolCfg(flags, name="InDetSecVtxTruthMatchTool", **kwargs):
    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.InDetSecVtxTruthMatchTool(**kwargs))
    return acc

def SecVertexTruthMatchAlgCfg(flags, name="SecVertexTruthMatchAlg", useLRTTracks = False, **kwargs):

    acc = ComponentAccumulator()

    if useLRTTracks:
        from DerivationFrameworkInDet.InDetToolsConfig import InDetLRTMergeCfg
        acc.merge(InDetLRTMergeCfg(flags))
        kwargs.setdefault("TrackParticleContainer", "InDetWithLRTTrackParticles")

    kwargs.setdefault("TruthVertexContainer", "TruthVertices")
    kwargs.setdefault("SecondaryVertexContainer", "VrtSecInclusive_SecondaryVertices")
    kwargs.setdefault("TargetPDGIDs", [511,521])
    kwargs.setdefault("MatchTool", acc.popToolsAndMerge(InDetSecVtxTruthMatchToolCfg(flags)))

    truthMatchAlg = CompFactory.CP.SecVertexTruthMatchAlg(name, **kwargs)

    acc.addEventAlgo(truthMatchAlg)
    acc.addService(CompFactory.THistSvc(Output = [f"ANALYSIS DATAFILE='{flags.Output.HISTFileName}' OPT='RECREATE'"]))
    acc.setAppProperty("HistogramPersistency","ROOT")
    return acc

