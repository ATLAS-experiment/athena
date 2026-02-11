# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from OutputStreamAthenaPool.OutputStreamConfig import addToAOD, addToESD

def HoughVtxFinderToolCfg(flags, name="HoughVtxFinderTool", **kwargs):
    """Configures HoughVtxFinderTool"""
    acc = ComponentAccumulator()

    from BeamSpotConditions.BeamSpotConditionsConfig import BeamSpotCondAlgCfg
    acc.merge(BeamSpotCondAlgCfg(flags))
    kwargs.setdefault("BeamSpotKey", "BeamSpotData")

    acc.setPrivateTools(CompFactory.ActsTrk.HoughVtxFinderTool(name, **kwargs))
    return acc

def HIHoughVtxRecoCfg(flags, name="HIHoughVtxReco", **kwargs):
    """Configures HIHoughVtxReco"""
    acc = ComponentAccumulator()

    if "HoughVtxFinderTool" not in kwargs:
        houghVtxFinderTool = acc.popToolsAndMerge(HoughVtxFinderToolCfg(flags, name = "HoughVtxFinderTool"))
        kwargs.setdefault("HoughVtxFinderTool", houghVtxFinderTool)

    kwargs.setdefault("inputPixelSpacePoints", "PixelSpacePoints")
    kwargs.setdefault("outputHoughVtx", "HoughVertices")

    acc.addEventAlgo(CompFactory.HIHoughVtxReco(name, **kwargs))
    return acc

def HIHoughVtxFinderCfg(flags):
    """Configures Heavy Ion Global quantities """
    acc = ComponentAccumulator()

    acc.merge(HIHoughVtxRecoCfg(flags))
    output = [ "xAOD::VertexContainer#HoughVertices", "xAOD::VertexAuxContainer#HoughVerticesAux."]

    acc.merge(addToESD(flags, output))
    acc.merge(addToAOD(flags, output))

    return acc


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultConditionsTags, defaultGeometryTags, defaultTestFiles
    flags = initConfigFlags()

    flags.Input.Files = [defaultTestFiles.d + "/RecJobTransformTests/data23_hi/data23_hi.00462809.physics_EnhancedBias.merge.RAW._lb0422._SFO-11._0001.1"]
    flags.IOVDb.GlobalTag = defaultConditionsTags.RUN3_DATA
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN3

    flags.Exec.MaxEvents=20
    flags.Concurrency.NumThreads=1

    flags.Output.doWriteAOD = True
    flags.Output.AODFileName = "myAOD.pool.root"
    flags.Output.doWriteESD = True
    flags.Output.ESDFileName = "myESD.pool.root"

    # enable to pass flags from command line, e.g.: 
    ## python -m HIGlobal.HIHoughVtxFinderConfig Exec.FPE=100
    flags.fillFromArgs() 
    
    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)
    from TriggerJobOpts.TriggerByteStreamConfig import ByteStreamReadCfg
    acc.merge(ByteStreamReadCfg(flags))

    # need PixelSpacePoints first
    from InDetConfig.InDetPrepRawDataFormationConfig  import PixelClusterizationCfg, SCTClusterizationCfg
    acc.merge(PixelClusterizationCfg(flags))
    acc.merge(SCTClusterizationCfg(flags))
    from InDetConfig.SiSpacePointFormationConfig  import InDetSiTrackerSpacePointFinderCfg, IDInDetToXAODSpacePointConversionCfg
    acc.merge(InDetSiTrackerSpacePointFinderCfg(flags))
    acc.merge(IDInDetToXAODSpacePointConversionCfg(flags))

    # main algorithm
    acc.merge(HIHoughVtxFinderCfg(flags))

    # output
    from AthenaPoolCnvSvc.PoolWriteConfig import PoolWriteCfg
    acc.merge(PoolWriteCfg(flags))

    # debug
    from AthenaCommon.Constants import DEBUG
    acc.foreach_component("*Hough*").OutputLevel=DEBUG

    acc.printConfig(withDetails=True, summariseProps=True)
    flags.dump()

    import sys
    sys.exit(acc.run().isFailure())
