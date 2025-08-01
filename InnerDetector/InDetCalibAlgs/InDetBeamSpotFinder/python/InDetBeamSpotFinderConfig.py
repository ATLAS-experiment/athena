# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
# Configuration of InDetBeamSpotFinder algorithms
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def InDetBeamSpotRooFitCfg(flags, jobConfig={}, name="InDetBeamSpotRooFit", **kwargs):
    result = ComponentAccumulator()

    kwargs.setdefault("OutputLevel",min(3,jobConfig['outputlevel']))
    if 'InitialKFactor'   in jobConfig: kwargs.setdefault("InitialKFactor",jobConfig['InitialKFactor'])
    if 'ConstantKFactor'  in jobConfig: kwargs.setdefault("ConstantKFactor",jobConfig['ConstantKFactor'])
    if 'RooFitMaxTransverseErr'  in jobConfig: kwargs.setdefault("vtxResCut",jobConfig['RooFitMaxTransverseErr'])
    
    result.setPrivateTools(
        CompFactory.InDet.InDetBeamSpotRooFit(name, **kwargs))
    return result

def InDetBeamSpotVertexCfg(flags, jobConfig={}, name="InDetBeamSpotVertex", **kwargs):
    result = ComponentAccumulator()

    kwargs.setdefault("OutputLevel",min(3,jobConfig['outputlevel']))
    if 'MaxSigmaTr'       in jobConfig: kwargs.setdefault("MaxSigmaTr",jobConfig['MaxSigmaTr']) 
    if 'OutlierChi2Tr'    in jobConfig: kwargs.setdefault("OutlierChi2Tr",jobConfig['OutlierChi2Tr'])
    if 'InitialKFactor'   in jobConfig: kwargs.setdefault("InitParK",jobConfig['InitialKFactor'])
    if 'InitialSigmaX'    in jobConfig: kwargs.setdefault("InitParSigmaX",jobConfig['InitialSigmaX'])
    if 'InitialSigmaY'    in jobConfig: kwargs.setdefault("InitParSigmaY",jobConfig['InitialSigmaY'])
    if 'InitialSigmaZ'    in jobConfig: kwargs.setdefault("InitParSigmaZ",jobConfig['InitialSigmaZ'])
    if 'ConstantKFactor'  in jobConfig: kwargs.setdefault("FixParK",jobConfig['ConstantKFactor'])
    if 'FixWidth'         in jobConfig: kwargs.setdefault("FixWidth",jobConfig['FixWidth'])
    if 'TruncatedRMS'     in jobConfig: kwargs.setdefault("TruncatedRMS",jobConfig['TruncatedRMS'])
    if 'SetInitialRMS'    in jobConfig: kwargs.setdefault("SetInitialRMS",jobConfig['SetInitialRMS'])
    if 'MaxSigmaVtx'      in jobConfig: kwargs.setdefault("InitParMaxSigmaX",jobConfig['MaxSigmaVtx'])
    if 'MaxSigmaVtx'      in jobConfig: kwargs.setdefault("InitParMaxSigmaY",jobConfig['MaxSigmaVtx'])
    
    result.setPrivateTools(
        CompFactory.InDet.InDetBeamSpotVertex(name, **kwargs))
    return result

def InDetBeamSpotFinderCfg(flags, jobConfig={}, name="InDetBeamSpotFinder", **kwargs):
    result = ComponentAccumulator()
    if 'UseFilledBCIDsOnly' in jobConfig:
        if jobConfig['UseFilledBCIDsOnly']:
            # Add BunchCrossingCondData
            print("Set bunch crossing")
            from LumiBlockComps.BunchCrossingCondAlgConfig import BunchCrossingCondAlgCfg
            result = BunchCrossingCondAlgCfg(flags)

    if "BeamSpotToolList" not in kwargs:
        kwargs.setdefault("BeamSpotToolList", [
            result.addPublicTool(result.popToolsAndMerge(
                InDetBeamSpotRooFitCfg(flags,jobConfig))),
            result.addPublicTool(result.popToolsAndMerge(
                InDetBeamSpotVertexCfg(flags,jobConfig))) ])

    # Job configuration
    kwargs.setdefault("OutputLevel",min(3,jobConfig['outputlevel']))
    if 'VertexNtuple'       in jobConfig: kwargs.setdefault("VertexNtuple",jobConfig['VertexNtuple']) 
    if 'WriteAllVertices'   in jobConfig: kwargs.setdefault("WriteAllVertices",jobConfig['WriteAllVertices']) 
    if 'VertexTreeName'     in jobConfig: kwargs.setdefault("VertexTreeName",jobConfig['VertexTreeName'])
    #Event selection options
    if 'UseBCID'            in jobConfig: kwargs.setdefault("UseBCID",jobConfig['UseBCID'])
    if 'UseFilledBCIDsOnly' in jobConfig: kwargs.setdefault("UseFilledBCIDsOnly",jobConfig['UseFilledBCIDsOnly'])
    #Vertex Selection options
    if 'VertexContainer'    in jobConfig: kwargs.setdefault("VertexContainer",jobConfig['VertexContainer'])
    if 'MinTracksPerVtx'    in jobConfig: kwargs.setdefault("MinTracksPerVtx",jobConfig['MinTracksPerVtx'])
    if 'MaxTracksPerVtx'    in jobConfig: kwargs.setdefault("MaxTracksPerVtx",jobConfig['MaxTracksPerVtx'])
    if 'MaxVtxNum'          in jobConfig: kwargs.setdefault("MaxVtxNum",jobConfig['MaxVtxNum'])
    if 'MaxVtxChi2'         in jobConfig: kwargs.setdefault("MaxVtxChi2",jobConfig['MaxVtxChi2'])
    if 'MaxTransverseErr'   in jobConfig: kwargs.setdefault("MaxTransverseErr",jobConfig['MaxTransverseErr'])
    if 'MaxAbsCorrelXY'     in jobConfig: kwargs.setdefault("MaxAbsCorrelXY",jobConfig['MaxAbsCorrelXY'])
    
    if 'VertexTypes'        in jobConfig:
        kwargs.setdefault("VertexTypes",jobConfig['VertexTypes'])
    else:
        kwargs.setdefault("VertexTypes", ["PriVtx"])
    
    if 'MinVtxProb'         in jobConfig: kwargs.setdefault("MinVtxProb",jobConfig['MinVtxProb'])
    #Beamspot Sorting options
    if 'LumiRange'          in jobConfig: kwargs.setdefault("LumiRange",jobConfig['LumiRange'])
    if 'RunRange'           in jobConfig: kwargs.setdefault("RunRange",jobConfig['RunRange'])
    if 'EventRange'         in jobConfig: kwargs.setdefault("EventRange",jobConfig['EventRange'])
    if 'GroupFitsBy'        in jobConfig: kwargs.setdefault("GroupFitsBy",jobConfig['GroupFitsBy'])

    result.addEventAlgo(CompFactory.InDet.InDetBeamSpotFinder(name, **kwargs))
    return result
