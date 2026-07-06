# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from PyJobTransforms.TransformUtils import processPreExec, processPreInclude, processPostExec, processPostInclude
from RecJobTransforms.RecoSteering import RecoSteering
from AthenaConfiguration.ComponentFactory import CompFactory 

from AthenaCommon.Logging import logging
log = logging.getLogger('RAWtoDAOD_TLA')


def configureFlags(runArgs):
    # some basic settings here...
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    from PyJobTransforms.CommonRunArgsToFlags import commonRunArgsToFlags
    commonRunArgsToFlags(runArgs, flags)

    # Input
    if hasattr(runArgs, 'inputBSFile'):
        log.warning("Enters the inputBSFile if")
        flags.Input.Files = runArgs.inputBSFile

    if hasattr(runArgs, 'inputRDOFile'):
        log.warning("Enters the inputRDOFile if")
        flags.Input.Files = runArgs.inputRDOFile

    from TrigEDMConfig.DataScoutingInfo import getDataScoutingTypeFromStream, getDataScoutingStreams
    if flags.Input.TriggerStream in getDataScoutingStreams():
       dstype = getDataScoutingTypeFromStream(flags.Input.TriggerStream)


    # Output
    if hasattr(runArgs, 'outputDAOD_TLAFile'):
        flags.Output.AODFileName = runArgs.outputDAOD_TLAFile
        log.info("---------- Configured DAOD_TLA output")
        flags.Trigger.AODEDMSet=dstype
        from AthenaConfiguration.DetectorConfigFlags import allDetectors
        disabled_detectors = allDetectors
    elif hasattr(runArgs, 'outputDAOD_TLAFTAGPEBFile'):
        flags.Output.AODFileName = runArgs.outputDAOD_TLAFTAGPEBFile
        log.info("---------- Configured DAOD_TLAFTAGPEB output")
        flags.Trigger.AODEDMSet=dstype
        disabled_detectors = [
            'TRT',
            'LAr', 'Tile', 'MBTS',
            'CSC', 'MDT', 'RPC', 'TGC',
            'sTGC', 'MM',
            'Lucid', 'ZDC', 'ALFA', 'AFP',
        ]
    elif hasattr(runArgs, 'outputDAOD_TLADJETPEBFile'):
        flags.Output.AODFileName = runArgs.outputDAOD_TLADJETPEBFile
        log.info("---------- Configured DAOD_TLADJETPEB output")
        flags.Trigger.AODEDMSet=dstype
        disabled_detectors = [
            'MBTS',
            'Lucid', 'ZDC', 'ALFA', 'AFP',
        ]
    elif hasattr(runArgs, 'outputDAOD_TLAEGAMPEBFile'):
        flags.Output.AODFileName = runArgs.outputDAOD_TLAEGAMPEBFile
        log.info("---------- Configured DAOD_TLAEGAMPEB output")
        flags.Trigger.AODEDMSet=dstype
        disabled_detectors = [
            'MBTS',
            'CSC', 'MDT', 'RPC', 'TGC',
            'sTGC', 'MM',
            'Lucid', 'ZDC', 'ALFA', 'AFP',
        ]

    from RecJobTransforms.RecoConfigFlags import recoRunArgsToFlags
    recoRunArgsToFlags(runArgs, flags)

    # Set non-default flags 
    flags.Trigger.doLVL1=False
    flags.Trigger.DecisionMakerValidation.Execute = False
    flags.Trigger.doNavigationSlimming = False
    flags.Trigger.L1.doCalo=False
    flags.Trigger.L1.doCTP=False

    from AthenaConfiguration.Enums import ProductionStep
    flags.Common.ProductionStep=ProductionStep.Reconstruction

    from AthenaConfiguration.AutoConfigFlags import GetFileMD
    from AthenaConfiguration.TestDefaults import defaultGeometryTags
    if GetFileMD(flags.Input.Files)["GeoAtlas"] is None:
        flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN3
    
    # Setup detector flags
    from AthenaConfiguration.DetectorConfigFlags import disableDetectors
    disableDetectors(
        flags, toggle_geometry=True,
        detectors=disabled_detectors,
    )

    # Print reco domain status
    from RecJobTransforms.RecoConfigFlags import printRecoFlags
    printRecoFlags(flags)

    # Setup perfmon flags from runargs
    from PerfMonComps.PerfMonConfigHelpers import setPerfmonFlagsFromRunArgs
    setPerfmonFlagsFromRunArgs(flags, runArgs)

    # process pre-include/exec
    processPreInclude(runArgs, flags)
    processPreExec(runArgs, flags)

    # To respect --athenaopts 
    flags.fillFromArgs()

    # Lock flags
    flags.lock()

    return flags



def fromRunArgs(runArgs):

    log.info('****************** STARTING TLA RAW Decoding (RAWtoDAOD_TLA) *****************')

    log.info('**** Transformation run arguments')
    log.info(str(runArgs))

    import time
    timeStart = time.time()

    flags = configureFlags(runArgs)
    log.info("Configuring according to flag values listed below")
    flags.dump()

    cfg = RecoSteering(flags)

    # import the TLA decoding
    cfg.flagPerfmonDomain('Trigger')

    # add additional objects reconstructed in RAWtoDAOD step (i.e. not in trigger EDM)
    additional_output_items = {
        'PhysicsTLA': [],
        'FTagPEBTLA':
        [
            'xAOD::JetAuxContainer#HLT_AntiKt4EMPFlowJets_subresjesgscIS_ftf_TLAAux.TracksForBTagging.GN2v01_pb.GN2v01_pc.GN2v01_pu.GN2v01_ptau',
        ],
        'DarkJetPEBTLA': [],
        'EgammaPEBTLA': [],
        'MuonDS': [],
    }[flags.Trigger.AODEDMSet]
    from TLARecoConfig.DAOD_TLA_OutputConfig import DAOD_TLA_OutputCfg
    cfg.merge( DAOD_TLA_OutputCfg(flags, additional_output_items) )

    if flags.Trigger.AODEDMSet == 'FTagPEBTLA':
        from TLARecoConfig.FTagPEBRecoConfig import FTagPEBJetTagConfig
        cfg.merge(FTagPEBJetTagConfig(flags))

    #For MC, set up seeded decoding + online CaloCellMaker
    ebType=flags.Trigger.AODEDMSet
    if flags.Input.isMC:
        if flags.Detector.GeometryMDT:
            cfg.getEventAlgo('MdtRdoToMdtPrepData').DoSeededDecoding=True
            cfg.getEventAlgo('MdtRdoToMdtPrepData').RoIs='HLT_Roi_Selected_'+ebType
        if flags.Detector.GeometryRPC:
            cfg.getEventAlgo('RpcRdoToRpcPrepData').DoSeededDecoding=True
            cfg.getEventAlgo('RpcRdoToRpcPrepData').RoIs='HLT_Roi_Selected_'+ebType
        if flags.Detector.GeometryTGC:
            cfg.getEventAlgo('TgcRdoToTgcPrepData').DoSeededDecoding=True
            cfg.getEventAlgo('TgcRdoToTgcPrepData').RoIs='HLT_Roi_Selected_'+ebType
        if flags.Detector.GeometryMM:
            cfg.getEventAlgo('MM_RdoToMM_PrepData').DoSeededDecoding=True
            cfg.getEventAlgo('MM_RdoToMM_PrepData').RoIs='HLT_Roi_Selected_'+ebType
        if flags.Detector.GeometrysTGC:
            cfg.getEventAlgo('StgcRdoToStgcPrepData').DoSeededDecoding=True
            cfg.getEventAlgo('StgcRdoToStgcPrepData').RoIs='HLT_Roi_Selected_'+ebType

        if flags.Detector.GeometryPixel:
            cfg.getEventAlgo('InDetPixelClusterization').isRoI_Seeded=True
            cfg.getEventAlgo('InDetPixelClusterization').RoIs='HLT_Roi_Selected_'+ebType
            from RegionSelector.RegSelToolConfig import regSelTool_Pixel_Cfg
            cfg.getEventAlgo('InDetPixelClusterization').RegSelTool=cfg.popToolsAndMerge(regSelTool_Pixel_Cfg(flags))
        if flags.Detector.GeometrySCT:
            cfg.getEventAlgo('InDetSCT_Clusterization').isRoI_Seeded=True
            cfg.getEventAlgo('InDetSCT_Clusterization').RoIs='HLT_Roi_Selected_'+ebType
            from RegionSelector.RegSelToolConfig import regSelTool_SCT_Cfg
            cfg.getEventAlgo('InDetSCT_Clusterization').RegSelTool=cfg.popToolsAndMerge(regSelTool_SCT_Cfg(flags))
        if flags.Detector.GeometryTRT:
            cfg.getEventAlgo('InDetTRT_RIO_Maker').isRoI_Seeded=True
            cfg.getEventAlgo('InDetTRT_RIO_Maker').RoIs='HLT_Roi_Selected_'+ebType
            from RegionSelector.RegSelToolConfig import regSelTool_TRT_Cfg
            cfg.getEventAlgo('InDetTRT_RIO_Maker').RegSelTool=cfg.popToolsAndMerge(regSelTool_TRT_Cfg(flags))

        if flags.Detector.GeometryCalo:
            from TrigCaloRec.TrigCaloRecConfig import hltCaloCellMakerCfg
            cfg.merge(hltCaloCellMakerCfg(flags,name='RoICaloCellmaker', roisKey='HLT_Roi_Selected_'+ebType, CellsName='AllCalo'))
        else:
            #needed to read SCell container in RDO files (maybe could get away with a more minimal set of algorithms)
            from TrigT2CaloCommon.TrigCaloDataAccessConfig import trigCaloDataAccessSvcCfg
            cfg.merge(trigCaloDataAccessSvcCfg(flags))

    # setup Metadata writer
    from AthenaConfiguration.Enums import MetadataCategory
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    cfg.merge(SetupMetaDataForStreamCfg(flags,'AOD', createMetadata=[MetadataCategory.CutFlowMetaData]))

    # Write some metadata into TagInfo
    from EventInfoMgt.TagInfoMgrConfig import TagInfoMgrCfg
    cfg.merge(TagInfoMgrCfg(flags,
                            tagValuePairs={
                                "beam_type": flags.Beam.Type.value,
                                "beam_energy": str(int(flags.Beam.Energy)),
                                "triggerStreamOfFile": "" if flags.Input.isMC else flags.Input.TriggerStream,
                                "project_name": "IS_SIMULATION" if flags.Input.isMC else flags.Input.ProjectName,
                            }))
    if not flags.Input.isMC and flags.Input.DataYear > 0:
        cfg.merge(TagInfoMgrCfg(flags,
                                tagValuePairs={
                                    "data_year": str(flags.Input.DataYear)
                                }))

    # Post-include
    processPostInclude(runArgs, flags, cfg)

    # Post-exec
    processPostExec(runArgs, flags, cfg)

    from AthenaCommon.Constants import INFO
    if flags.Exec.OutputLevel <= INFO:
        cfg.printConfig()

    # Run the final accumulator
    sc = cfg.run()
    timeFinal = time.time()
    log.info("Run RAWtoDAOD_TLA_skeleton in %d seconds", timeFinal - timeStart)

    import sys
    sys.exit(sc.isFailure())
