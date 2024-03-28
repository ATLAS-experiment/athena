#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

def LArClusterCellMonConfig(flags):

    from AthenaMonitoring.AthMonitorCfgHelper import AthMonitorCfgHelper
    helper = AthMonitorCfgHelper(flags,'LArClusterCellMonAlgCfg')

    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    cfg=ComponentAccumulator()


    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    cfg.merge(LArGMCfg(flags))
    from TileGeoModel.TileGMConfig import TileGMCfg
    cfg.merge(TileGMCfg(flags))

    from DetDescrCnvSvc.DetDescrCnvSvcConfig import DetDescrCnvSvcCfg
    cfg.merge(DetDescrCnvSvcCfg(flags))

    from LArCellRec.LArCollisionTimeConfig import LArCollisionTimeCfg
    cfg.merge(LArCollisionTimeCfg(flags))


    from AthenaConfiguration.ComponentFactory import CompFactory
    lArCellMonAlg=CompFactory.LArClusterCellMonAlg

    algname='LArClusterCellMonAlg'
    from AthenaConfiguration.Enums import BeamType
    if flags.Beam.Type is BeamType.Cosmics:
        algname=algname+'Cosmics'

    algo = LArClusterCellMonConfigCore(helper, lArCellMonAlg, flags,
                                flags.Beam.Type is BeamType.Cosmics,
                                flags.Input.TriggerStream,
                                flags.Input.isMC, flags.Common.isOnline,
                                algname)
    algo.useTrigger = flags.DQ.useTrigger

    #copied from LArClusterCellMonTool
    algo.rndmTriggerNames    = "L1_RD0, L1_RD0_FILLED, L1_RD0_EMPTY, L1_RD1, L1_RD1_NOISE, L1_RD1_HIST, L1_RD1_BGRP4, L1_RD1_BGRP5"
    algo.caloTriggerNames    = "L1_EM[0-9]+, L1_HA[0-9]+, L1_J[0-9]+.*, L1_JB[0-9]+, L1_JF[0-9]+, L1_TE[0-9]+, L1_JE[0-9]+, L1_XE[0-9]+, L1_2EM[0-9]+, L1_2FJ[0-9]+, L1_2J[0-9]+,L1_3J[0-9]+.*,L1_4J[0-9]+.*,L1_5J[0-9]+,L1_6J[0-9]+,L1_FJ[0-9]+.*"
    algo.minBiasTriggerNames = "L1_RD0_FILLED, L1_MBTS_1, L1_MBTS_2, L1_MBTS_1_1"
    algo.metTriggerNames     = "EF_xe[0-9]+.*"
    algo.miscTriggerNames    = ""

    from AthenaMonitoring.AtlasReadyFilterConfig import AtlasReadyFilterCfg
    algo.ReadyFilterTool = cfg.popToolsAndMerge(AtlasReadyFilterCfg(flags))

    if not flags.Input.isMC:
       from AthenaMonitoring.BadLBFilterToolConfig import LArBadLBFilterToolCfg
       algo.BadLBTool=cfg.popToolsAndMerge(LArBadLBFilterToolCfg(flags))

    cfg.merge(helper.result())

    return cfg


def LArClusterCellMonConfigCore(helper, algclass, flags, isCosmics=False, TriggerStream='CosmicCalo', isMC=False, isOnline=False, algname='LArClusterCellMonAlg'):


    LArClusterCellMonAlg = helper.addAlgorithm(algclass, algname)

    if not isCosmics and not isMC:
        LArClusterCellMonAlg.useReadyFilterTool=True
    else:
        LArClusterCellMonAlg.useReadyFilterTool=False

    if isMC:
        LArClusterCellMonAlg.useBadLBTool=False
    else:
        LArClusterCellMonAlg.useBadLBTool=True

    if isCosmics or TriggerStream!='physics_CosmicCalo':
       LArClusterCellMonAlg.useBeamBackgroundRemoval = False
       LArClusterCellMonAlg.useLArCollisionFilterTool = False 
    else:
       LArClusterCellMonAlg.useBeamBackgroundRemoval = True
       LArClusterCellMonAlg.useLArCollisionFilterTool = True

    if isOnline:
       LArClusterCellMonAlg.useLArNoisyAlg = False   
    else:   
       LArClusterCellMonAlg.useLArNoisyAlg = True

    GroupName="LArClusterCell"
    LArClusterCellMonAlg.MonGroupName = GroupName

    LArClusterCellMonAlg.EnableLumi = True
    
    
    from CaloMonitoring.LArCellBinning import lArCellBinningScheme

    binlabels=["TotalEvents","ATLAS Ready","with Good LAr LB","with No LAr Collision","with No Beam Background", "with No Trigger Filter","with No LArError"] 

    # triggers to use
    LArClusterCellMonAlg.rndmTriggerNames    = "L1_RD0, L1_RD0_FILLED, L1_RD0_EMPTY, L1_RD1, L1_RD1_NOISE, L1_RD1_HIST, L1_RD1_BGRP4, L1_RD1_BGRP5"
    LArClusterCellMonAlg.caloTriggerNames    = "L1_EM[0-9]+, L1_HA[0-9]+, L1_J[0-9]+.*, L1_JB[0-9]+, L1_JF[0-9]+, L1_TE[0-9]+, L1_JE[0-9]+, L1_XE[0-9]+, L1_2EM[0-9]+, L1_2FJ[0-9]+, L1_2J[0-9]+,L1_3J[0-9]+.*,L1_4J[0-9]+.*,L1_5J[0-9]+,L1_6J[0-9]+,L1_FJ[0-9]+.*"
    LArClusterCellMonAlg.minBiasTriggerNames = "L1_RD0_FILLED, L1_MBTS_1, L1_MBTS_2, L1_MBTS_1_1"
    LArClusterCellMonAlg.metTriggerNames     = "EF_xe[0-9]+.*"
    LArClusterCellMonAlg.miscTriggerNames    = ""

    # Global Settings:

   #---single Group for non threshold histograms
    cellMonGroup = helper.addGroup(
        LArClusterCellMonAlg,
        GroupName,
        '/CaloMonitoring/LArClusterCellMon/'

    )


    #--define histograms
    # Summary
    summ_hist_path='Summary/'

    from CaloMonitoring.CaloMonAlgBase import CaloBaseHistConfig
    CaloBaseHistConfig(cellMonGroup,summ_hist_path,binlabels)  

    cellMonGroup.defineHistogram('trigType;nEvtsByTrigger',
                                 title='Total Events: bin 0, RNDM Trigger: 1, Calo Trigger: 2, MinBias Trigger: 3, MET Trigger: 4, Misc Trigger: 5, Events Accepted 6 ',
                                 type='TH1I', path=summ_hist_path,
                                 xbins=lArCellBinningScheme.larCellSummary["xbins"][0], xmin=lArCellBinningScheme.larCellSummary["xbins"][1], xmax=lArCellBinningScheme.larCellSummary["xbins"][2],
                                 xlabels=lArCellBinningScheme.larCellSummary["xlabels"])

    ##total events, for normalisation
    cellMonGroup.defineHistogram('eventCounter',
                                 title='Events events passed Trigger and Background removal for each threshold (for normalisation)',
                                 type='TH1I', path=summ_hist_path,
                                 xbins=1
                             )
    
    cellMonGroup.defineHistogram('cellhash',
                                 title='Total number of cells contributing to clusters, per online-hash',
                                 type='TH1I', path=summ_hist_path,
                                 xbins=195072,xmin=-0.5,xmax=195072.5
                             )



    for part in LArClusterCellMonAlg.LayerNames:
        cellMonGroup.defineHistogram('celleta_'+part+',cellphi_'+part+';NClusteredCells_'+part,
                                     title="Total number of cells contributing to clusters in (#eta,#phi) for "+part+";cell #eta;cell #phi",
                                     weight='NClusteredCells_'+part,
                                     type='TH2I', path="ClusterCell/", 
                                     xbins = lArCellBinningScheme.etaRange[part],
                                     ybins = lArCellBinningScheme.phiRange[part]
)
     
        pass #part loop


    return LArClusterCellMonAlg


if __name__=='__main__':

    # Set the Athena configuration flags
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    flags.Input.Files = defaultTestFiles.RAW_RUN3
    flags.Output.HISTFileName = 'LArClusterCellMonOutput.root'
    flags.DQ.enableLumiAccess = True
    flags.DQ.useTrigger = True
    flags.DQ.Environment = 'tier0'
    from AthenaConfiguration.Enums import LHCPeriod
    flags.GeoModel.Run=LHCPeriod.Run3
    flags.Concurrency.NumThreads=1
    flags.lock()


    
    # Initialize configuration object, add accumulator, merge, and run.
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg 
    cfg = MainServicesCfg(flags)

    from TrigConfigSvc.TrigConfigSvcCfg import L1ConfigSvcCfg, HLTConfigSvcCfg, L1PrescaleCondAlgCfg, HLTPrescaleCondAlgCfg
    from TrigT1ResultByteStream.TrigT1ResultByteStreamConfig import L1TriggerByteStreamDecoderCfg

    cfg.merge( L1TriggerByteStreamDecoderCfg(flags) )
    cfg.merge( L1ConfigSvcCfg(flags) )
    cfg.merge( HLTConfigSvcCfg(flags) )
    cfg.merge( L1PrescaleCondAlgCfg(flags) )
    cfg.merge( HLTPrescaleCondAlgCfg(flags) )

    from TrigConfigSvc.TrigConfigSvcCfg import BunchGroupCondAlgCfg
    cfg.merge( BunchGroupCondAlgCfg( flags ) )

    from AthenaConfiguration.ComponentFactory import CompFactory
    tdm = CompFactory.getComp('TrigDec::TrigDecisionMakerMT')()
    tdm.doL1 = True
    tdm.doHLT = False
    cfg.addEventAlgo( tdm, 'AthAlgSeq' )



    # in case of tier0 workflow:
    from CaloRec.CaloRecoConfig import CaloRecoCfg
    cfg.merge(CaloRecoCfg(flags))


    cfg.merge(LArClusterCellMonConfig(flags)) 

    cfg.run(10) #use cfg.run() to run on all events
