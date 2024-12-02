# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def FPGATrackSimReportingCfg(flags, name='FPGATrackSimReportingAlg',**kwargs):    

    kwargs.setdefault('perEventReports', False)
    kwargs.setdefault('xAODPixelClusterContainers',["ITkPixelClusters" ,"FPGAPixelClusters"])
    kwargs.setdefault('xAODStripClusterContainers',["ITkStripClusters" ,"FPGAStripClusters"])
    kwargs.setdefault('xAODSpacePointContainersFromFPGA',["FPGAPixelSpacePoints","FPGAStripSpacePoints", "FPGAStripOverlapSpacePoints", "ITkPixelSpacePoints","ITkStripSpacePoints", "ITkStripOverlapSpacePoints"])
    kwargs.setdefault('FPGATrackSimTracks','FPGATracks_1st')
    kwargs.setdefault('FPGATrackSimRoads','FPGARoads_1st')
    kwargs.setdefault('FPGATrackSimProtoTracks',["ActsProtoTracks_1stFromFPGATrack"])
    kwargs.setdefault('FPGAActsTracks',["FPGAActsTracks"])
    kwargs.setdefault('FPGAActsSeeds',['FPGAPixelSeeds','FPGAStripSeeds'])
    kwargs.setdefault('FPGAActsSeedsParam',['FPGAPixelEstimatedTrackParams','FPGAStripEstimatedTrackParams'])
    
    acc = ComponentAccumulator()
    from FPGATrackSimReporting.FPGATrackSimReportingConfig import FPGATrackSimReportingCfg
    acc.merge(FPGATrackSimReportingCfg(flags, name=name,**kwargs))

    return acc


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
    kwarg.setdefault('xAODMaker', containerMakerTool)
    kwarg.setdefault('PassThroughTool', passThroughTool)
    # xclbin and kernels
    kwarg.setdefault('xclbin', '')
    kwarg.setdefault('PixelClusteringKernelName','')
    kwarg.setdefault('SpacepointKernelName','')
    kwarg.setdefault('PassThroughKernelName', '')
    kwarg.setdefault('RunPassThrough', flags.FPGADataPrep.RunPassThrough)
    # Test vectors
    kwarg.setdefault('UseTV', flags.FPGADataPrep.FPGA.UseTV)
    kwarg.setdefault('PixelClusterTV','')
    kwarg.setdefault('PixelClusterRefTV','')
    kwarg.setdefault('SpacepointTV','')
    kwarg.setdefault('SpacepointRefTV','')

    acc.addEventAlgo(CompFactory.DataPreparationPipeline(**kwarg))
    return acc

if __name__=="__main__":
    from EFTrackingFPGAIntegration.IntegrationConfigFlag import addFPGADataPrepFlags
    flags = addFPGADataPrepFlags()

    # The input file should be specified by the user
    flags.Input.Files = [""]
    flags.Output.AODFileName = "DataPrepAOD.pool.root"
    
    # For pass-through kernel
    flags.FPGADataPrep.RunPassThrough = False
    flags.FPGADataPrep.PassThrough.RunSoftware = True
    flags.FPGADataPrep.PassThrough.ClusterOnly = True
    
    # ensure that the xAOD SP and cluster containers are available
    flags.Tracking.ITkMainPass.doAthenaToActsSpacePoint=True
    flags.Tracking.ITkMainPass.doAthenaToActsCluster=True

    # For Spacepoint formation
    if flags.FPGADataPrep.PassThrough.ClusterOnly:
        flags.Detector.EnableITkPixel = True
        flags.Detector.EnableITkStrip = True
        flags.Acts.useCache = False
        flags.Tracking.ITkMainPass.doActsSeed=True
    
    # Disable calo for this test
    flags.Detector.EnableCalo = False

    ###########################################
    # IDTPM flags
    from InDetTrackPerfMon.InDetTrackPerfMonFlags import initializeIDTPMConfigFlags, initializeIDTPMTrkAnaConfigFlags
    flags = initializeIDTPMConfigFlags(flags)
    
    flags.PhysVal.IDTPM.outputFilePrefix = "myIDTPM_CA"
    flags.PhysVal.IDTPM.plotsDefFileList = "InDetTrackPerfMon/PlotsDefFileList_default.txt" # default value - not needed
    flags.PhysVal.IDTPM.plotsCommonValuesFile = "InDetTrackPerfMon/PlotsDefCommonValues.json" # default value - not needed
    flags.PhysVal.OutputFileName = flags.PhysVal.IDTPM.outputFilePrefix + '.HIST.root' # automatically set in IDTPM config - not needed
    flags.Output.doWriteAOD_IDTPM = True
    flags.Output.AOD_IDTPMFileName = flags.PhysVal.IDTPM.outputFilePrefix + '.AOD_IDTPM.pool.root' # automatically set in IDTPM config - not needed
    flags.PhysVal.IDTPM.trkAnaCfgFile = "InDetTrackPerfMon/EFTrkAnaConfig_example.json"
    
    flags = initializeIDTPMTrkAnaConfigFlags(flags)
    ## override respective configurations from trkAnaCfgFile (in case something changes in the config file)
    flags.PhysVal.IDTPM.TrkAnaEF.TrigTrkKey = "FPGATrackParticles"
    flags.PhysVal.IDTPM.TrkAnaDoubleRatio.TrigTrkKey = "FPGATrackParticles"


    flags.fillFromArgs()
    flags.lock()
    flags = flags.cloneAndReplace("Tracking.ActiveConfig", "Tracking.MainPass", keepOriginal=True)
    flags = flags.cloneAndReplace("Tracking.ActiveConfig", "Tracking.ITkMainPass", keepOriginal=True)


    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)
    
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg.merge(PoolReadCfg(flags))
    
    #Truth
    if flags.Input.isMC:
        from xAODTruthCnv.xAODTruthCnvConfig import GEN_AOD2xAODCfg
        cfg.merge(GEN_AOD2xAODCfg(flags))

    # Standard reco
    from InDetConfig.ITkTrackRecoConfig import ITkTrackRecoCfg
    cfg.merge(ITkTrackRecoCfg(flags))

    kwarg = {}
    # The Data Preparation (F100) Pipeline on FPGA
    cfg.merge(DataPrepCfg(flags, **kwarg))
    
    # Connection to ACTS
    if flags.FPGADataPrep.DoActs:
        from EFTrackingFPGAIntegration.DataPrepToActsConfig import DataPrepToActsCfg
        cfg.merge(DataPrepToActsCfg(flags, **kwarg))
    
    cfg.merge(FPGATrackSimReportingCfg(flags))

    # IDTPM running
    from InDetTrackPerfMon.InDetTrackPerfMonConfig import InDetTrackPerfMonCfg
    cfg.merge( InDetTrackPerfMonCfg(flags) )


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

    # When we use test vectors, we only need to run once
    if not flags.FPGADataPrep.RunPassThrough and flags.FPGADataPrep.FPGA.UseTV:
        cfg.run(1)
    else:    
        cfg.run(-1)
