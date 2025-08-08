# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def InDetSecVtxTruthMatchToolCfg(flags, name="InDetSecVtxTruthMatchTool", **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault("trackMatchProb", 0.5)
    kwargs.setdefault("vertexMatchWeight", 0.5)
    kwargs.setdefault("trackPtCut", 1000.0)
    kwargs.setdefault("doSMOrigin", False)

    acc.setPrivateTools(CompFactory.InDetSecVtxTruthMatchTool(**kwargs))
    return acc

def InDetSecVtxTruthMatchToolMuSaCfg(flags, name="InDetSecVtxTruthMatchTool", **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault("trackMatchProb", 0.99999)
    kwargs.setdefault("vertexMatchWeight", 0.99999) #2trk means 1 real 1 fake would pass if 0.5
    kwargs.setdefault("trackPtCut", 1000.0)
    kwargs.setdefault("doMuSA", True)
    kwargs.setdefault("doSMOrigin", False)

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
    kwargs.setdefault("doSMOrigin", False)
    
    kwargs.setdefault("MatchTool", acc.popToolsAndMerge(InDetSecVtxTruthMatchToolCfg(
        flags, doSMOrigin=kwargs["doSMOrigin"])))

    acc.addEventAlgo(CompFactory.CP.SecVertexTruthMatchAlg(name, **kwargs))
    acc.addService(CompFactory.THistSvc(Output = [f"ANALYSIS DATAFILE='{flags.Output.HISTFileName}' OPT='RECREATE'"]))
    acc.setAppProperty("HistogramPersistency","ROOT")
    return acc

def SecVertexTruthMatchMuSaAlgCfg(flags, name="SecVertexTruthMatchMuSaAlg", **kwargs):

    acc = ComponentAccumulator()

    kwargs.setdefault("TruthVertexContainer", "TruthVertices")
    kwargs.setdefault("SecondaryVertexContainer", "MuSAVertices")
    kwargs.setdefault("TrackParticleContainer", "MuonSpectrometerTrackParticles")
    kwargs.setdefault("TargetPDGIDs", [50, 72, 31, 32, 3000001])
    kwargs.setdefault("doMuSA", True)
    kwargs.setdefault("doSMOrigin", False)

    kwargs.setdefault("MatchTool", acc.popToolsAndMerge(InDetSecVtxTruthMatchToolMuSaCfg(
        flags, doSMOrigin=kwargs["doSMOrigin"])))

    acc.addEventAlgo(CompFactory.CP.SecVertexTruthMatchAlg(name, **kwargs))
    acc.addService(CompFactory.THistSvc(Output = [f"ANALYSIS DATAFILE='{flags.Output.HISTFileName}' OPT='RECREATE'"]))
    acc.setAppProperty("HistogramPersistency","ROOT")
    return acc



