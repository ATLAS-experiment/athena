# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def DetectorVolumeSvcCfg(flags, name="DetectorVolumeSvc", **kwargs):
    result = ComponentAccumulator()
    detBuilders = []
    if flags.Detector.GeometryMuon:
        from ActsMuonDetector.ActsMuonDetectorCfg import MuonDetectorBuilderToolCfg
        detBuilders += [result.getPrimaryAndMerge(ActsSimpleCylinderDetBuilderToolCfg(flags))]
        detBuilders += [result.getPrimaryAndMerge(MuonDetectorBuilderToolCfg(flags))]
    kwargs.setdefault("DetectorBuilders", detBuilders)
    theSvc = CompFactory.ActsTrk.DetectorVolumeSvc(name, **kwargs)
    result.addService(theSvc, primary=True)
    return result

def ActsSimpleCylinderDetBuilderToolCfg(flags, name="SimpleCylinderDetBuilderTool", **kwargs):
    result = ComponentAccumulator()
    theSvc = CompFactory.ActsTrk.SimpleCylinderDetBuilderTool(name, **kwargs)
    result.setPrivateTools(theSvc)
    return result