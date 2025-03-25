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

def xAODClusterMakerCfg(flags, name = 'xAODClusterMaker', **kwarg):
    """Configure the xAODClusterMaker tool"""
    
    acc = ComponentAccumulator()
    
    kwarg.setdefault('PixelClusterContainerKey', 'FPGAPixelClusters')
    kwarg.setdefault('StripClusterContainerKey', 'FPGAStripClusters')
    
    acc.setPrivateTools(CompFactory.xAODClusterMaker(name, **kwarg))
    return acc

def xAODSpacePointMakerCfg(flags, name = 'xAODSpacePointMaker', **kwarg):
    """Configure the xAODSpacePointMaker tool"""
    
    acc = ComponentAccumulator()
    
    # Input clusters to read from
    kwarg.setdefault('PixelClusterContainerKey', 'FPGAPixelClusters')
    kwarg.setdefault('StripClusterContainerKey', 'FPGAStripClusters')
    # Output space points to create
    kwarg.setdefault('PixelSpacePointContainerKey', 'FPGAPixelSpacePoints')
    kwarg.setdefault('StripSpacePointContainerKey', 'FPGAStripSpacePoints')
    
    acc.setPrivateTools(CompFactory.xAODSpacePointMaker(name, **kwarg))
    return acc

def PassThroughToolCfg(flags, name = 'PassThroughTool', **kwarg):
        
    acc = ComponentAccumulator()
        
    kwarg.setdefault('name', name)
    kwarg.setdefault('StripClusterContainerKey', 'ITkStripClusters')
    kwarg.setdefault('PixelClusterContainerKey', 'ITkPixelClusters')
    kwarg.setdefault('RunSW', flags.FPGADataPrep.PassThrough.RunSoftware)
    kwarg.setdefault('ClusterOnlyPassThrough', flags.FPGADataPrep.PassThrough.ClusterOnly)
    kwarg.setdefault('MaxClusterNum', flags.FPGADataPrep.PassThrough.MaxClusterNum)
    kwarg.setdefault('MaxSpacePointNum', flags.FPGADataPrep.PassThrough.MaxSpacePointNum)
        
    acc.setPrivateTools(CompFactory.PassThroughTool(**kwarg))
    return acc

def DataPrepCfg(flags, name = "DataPreparationPipeline", **kwarg):

    acc = ComponentAccumulator()
    
    # Configure both tools
    clusterMakerTool = acc.popToolsAndMerge(xAODClusterMakerCfg(flags))
    spacePointMakerTool = acc.popToolsAndMerge(xAODSpacePointMakerCfg(flags))
    passThroughTool = acc.popToolsAndMerge(PassThroughToolCfg(flags))
    
    kwarg.setdefault('name', name)
    kwarg.setdefault('xAODClusterMaker', clusterMakerTool)
    kwarg.setdefault('xAODSpacePointMaker', spacePointMakerTool)
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
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    
    flags = initConfigFlags()
    flags = addFPGADataPrepFlags(flags)
    
    # useful for testing -> /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/EFTracking/ATLAS-P2-RUN4-03-00-00/RDO/reg0_singlemu.root

    # The input file should be specified by the user
    flags.Input.Files = [""]
    
    # Single muon with PU 200
    flags.Input.Files = ["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/RDO/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.900492.PG_single_muonpm_Pt1_etaFlatnp0_43.recon.RDO.e8481_s4149_r14697/RDO.33645151._000047.pool.root.1"]
    
    # ttbar with PU 200
    # flags.Input.Files = ["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/RDO/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.RDO.e8481_s4149_r14700/RDO.33629020._000047.pool.root.1"]
    
    flags.Output.AODFileName = "DataPrepAOD.pool.root"
    
    # For pass-through kernel
    flags.FPGADataPrep.RunPassThrough = False
    flags.FPGADataPrep.PassThrough.RunSoftware = True
    flags.FPGADataPrep.PassThrough.ClusterOnly = True
    # ensure that the xAOD SP and cluster containers are available
    flags.Tracking.ITkMainPass.doAthenaToActsSpacePoint=True
    flags.Tracking.ITkMainPass.doAthenaToActsCluster=True
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
                    "xAOD::SpacePointContainer#FPGAPixelSpacePoints",
                    "xAOD::SpacePointAuxContainer#FPGAPixelSpacePointsAux.-measurements",
                    "xAOD::SpacePointContainer#FPGAStripSpacePoints",
                    "xAOD::SpacePointAuxContainer#FPGAStripSpacePointsAux.-measurements",
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
