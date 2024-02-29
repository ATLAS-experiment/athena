# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def ActsMuonAlignCondAlgCfg(flags, name="ActsMuonAlignCondAlg", **kwargs):
    result = ComponentAccumulator()
    ### Do not setup the Acts alignment cond alg if no alignment or passivation is requested
    if not flags.Muon.usePhaseIIGeoSetup or ( not flags.Muon.enableAlignment and \
                                            not flags.Muon.applyMMPassivation):
        return result
    
    from MuonConfig.MuonGeometryConfig import MuonAlignmentCondAlgCfg
    kwargs.setdefault("applyMmPassivation", flags.Muon.applyMMPassivation)

    if kwargs["applyMmPassivation"]:
        from MuonConfig.MuonCondAlgConfig import NswPassivationDbAlgCfg
        result.merge(NswPassivationDbAlgCfg(flags))
    if flags.Muon.enableAlignment:
        result.merge(MuonAlignmentCondAlgCfg(flags))
    kwargs.setdefault("applyALines", len([alg for alg in result.getCondAlgos() if alg.name == "MuonAlignmentCondAlg"])>0)
    kwargs.setdefault("applyBLines", len([alg for alg in result.getCondAlgos() if alg.name == "MuonAlignmentCondAlg"])>0)
    kwargs.setdefault("applyNswAsBuilt", len([alg for alg in result.getCondAlgos() if alg.name == "NswAsBuiltCondAlg"])>0)
    kwargs.setdefault("applyMdtAsBuilt", len([alg for alg in result.getCondAlgos() if alg.name == "MdtAsBuiltCondAlg"])>0)

    the_alg = CompFactory.ActsMuonAlignCondAlg(name, **kwargs)
    result.addCondAlgo(the_alg)
    return result

def ActsGeomContextAlgCfg(flags, name="ActsGeomContextAlg", **kwargs):
    result = ComponentAccumulator()
    ### The Acts Muon align cond alg only works with the new Detector manager
    if not flags.Muon.usePhaseIIGeoSetup: 
        return result
    
    result.merge(ActsMuonAlignCondAlgCfg(flags))
    
    inAlignContainers = []
    if (flags.Muon.enableAlignment or flags.Muon.applyMMPassivation):
        if flags.Detector.GeometryMDT: inAlignContainers += ["MdtActsAlignContainer"]
        if flags.Detector.GeometryRPC: inAlignContainers += ["RpcActsAlignContainer"]
        if flags.Detector.GeometryTGC: inAlignContainers += ["TgcActsAlignContainer"]
    
    kwargs.setdefault("AlignKeys", inAlignContainers)
    the_alg = CompFactory.ActsMuonGeomContextAlg(name, **kwargs)
    result.addCondAlgo(the_alg)
    return result

    
