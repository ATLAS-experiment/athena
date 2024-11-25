# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaCommon.Constants import DEBUG

def xAODContainerMakerCfg(flags, name = 'xAODContainerMaker', **kwarg):
    
    acc = ComponentAccumulator()
    
    kwarg.setdefault('name', name)
    kwarg.setdefault('OutputStripName', 'FPGAStripClusters')
    kwarg.setdefault('OutputPixelName', 'FPGAPixelClusters')    
    # Spacepoints below will be further refined when the full pass-through kernel is ready
    kwarg.setdefault('OutputStripSpacePointName', 'PlaceHolderStripSpacePoints') 
    kwarg.setdefault('OutputPixelSpacePointName', 'PlaceHolderPixelSpacePoints')
    
    acc.setPrivateTools(CompFactory.xAODContainerMaker(**kwarg))
    return acc

def PassThroughToolCfg(flags, name = 'PassThroughTool', **kwarg):
        
    acc = ComponentAccumulator()
        
    kwarg.setdefault('name', name)
    kwarg.setdefault('StripClusterContainerKey', 'ITkStripClusters')
    kwarg.setdefault('PixelClusterContainerKey', 'ITkPixelClusters')
    kwarg.setdefault('RunSW', flags.FPGADataPrep.PassThrough.RunSoftware)
    kwarg.setdefault('ClusterOnlyPassThrough', flags.FPGADataPrep.PassThrough.ClusterOnly)
        
    acc.setPrivateTools(CompFactory.PassThroughTool(**kwarg))
    return acc

def DataPrepCfg(flags, name = "DataPreparationPipeline", **kwarg):

    acc = ComponentAccumulator()
    
    containerMakerTool = acc.popToolsAndMerge(xAODContainerMakerCfg(flags))
    passThroughTool = acc.popToolsAndMerge(PassThroughToolCfg(flags))
    
    kwarg.setdefault('name', name)
    kwarg.setdefault('xclbin', '')
    kwarg.setdefault('KernelName', '')
    kwarg.setdefault('RunPassThrough', flags.FPGADataPrep.RunPassThrough)
    kwarg.setdefault('xAODMaker', containerMakerTool)
    kwarg.setdefault('PassThroughTool', passThroughTool)

    acc.addEventAlgo(CompFactory.DataPreparationPipeline(**kwarg))
    return acc

if __name__=="__main__":
    from EFTrackingFPGAIntegration.IntegrationConfigFlag import addFPGADataPrepFlags
    flags = addFPGADataPrepFlags()

    flags.Concurrency.NumThreads = 1
    # The input file should be specified by the user
    flags.Input.Files = [""]
    flags.Output.AODFileName = "DataPrepAOD.pool.root"
    
    # For pass-through kernel
    flags.FPGADataPrep.RunPassThrough = True
    flags.FPGADataPrep.PassThrough.RunSoftware = True
    flags.FPGADataPrep.PassThrough.ClusterOnly = True
    
    # For Spacepoint formation
    if flags.FPGADataPrep.PassThrough.ClusterOnly:
        flags.Detector.EnableITkPixel = True
        flags.Detector.EnableITkStrip = True
        flags.Acts.useCache = False
        flags.Tracking.ITkMainPass.doActsSeed=True
    
    flags.Debug.DumpEvtStore = True
    
    flags.fillFromArgs()
    flags.lock()
    flags = flags.cloneAndReplace("Tracking.ActiveConfig", "Tracking.MainPass", keepOriginal=True)
    flags = flags.cloneAndReplace("Tracking.ActiveConfig", "Tracking.ITkMainPass", keepOriginal=True)


    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)
    
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg.merge(PoolReadCfg(flags))
    
    kwarg = {}
    kwarg["OutputLevel"] = DEBUG

    # The Data Preparation (F100) Pipeline on FPGA
    cfg.merge(DataPrepCfg(flags, **kwarg))
    
    # Connection to ACTS
    from EFTrackingFPGAIntegration.DataPrepToActsConfig import DataPrepToActsCfg
    cfg.merge(DataPrepToActsCfg(flags, **kwarg))
    
    # Prepare output
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from AthenaConfiguration.Enums import MetadataCategory
    cfg.merge(SetupMetaDataForStreamCfg(flags,"AOD", 
                                        createMetadata=[
                                        MetadataCategory.ByteStreamMetaData,
                                        MetadataCategory.LumiBlockMetaData,
                                        MetadataCategory.TruthMetaData,
                                        MetadataCategory.IOVMetaData,],))

    from OutputStreamAthenaPool.OutputStreamConfig import addToAOD
    OutputItemList = [
                    "xAOD::StripClusterContainer#FPGAStripClusters",
                    "xAOD::StripClusterAuxContainer#FPGAStripClustersAux.",
                    "xAOD::PixelClusterContainer#FPGAPixelClusters",
                    "xAOD::PixelClusterAuxContainer#FPGAPixelClustersAux.",
                    "xAOD::TrackParticleContainer#FPGATrackParticles",
                    "xAOD::TrackParticleAuxContainer#FPGATrackParticlesAux."
                    ]
   
    cfg.merge(addToAOD(flags, OutputItemList))
    
    cfg.printConfig()

    cfg.run(-1)
