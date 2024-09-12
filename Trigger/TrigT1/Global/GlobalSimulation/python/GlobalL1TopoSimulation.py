#!/usr/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import (ComponentAccumulator,)
from AthenaConfiguration.ComponentFactory import CompFactory
from libpyeformat_helper import SourceIdentifier, SubDetector

from getAlgData import algdata_from_menu
from toolFromAlgData import toolFromAlgData

from AthenaCommon.Logging import logging
logger = logging.getLogger(__name__)
from AthenaCommon.Constants import VERBOSE
logger.setLevel(VERBOSE)

import sys, os

def GlobalL1TopoSimulationCfg(flags, algLogLevel):

    acc = ComponentAccumulator()

    globalSimAlg = CompFactory.GlobalSim.GlobalSimulationAlg("GlobalL1TopoSim")
    globalSimAlg.OutputLevel = algLogLevel
 
    if 'GSDEBUG' in os.environ:
        globalSimAlg.enableDumps = True

    # for now, only run a small sub graph corresponding to
    # existing L1Topo Algorithms.
    root_names = [
        'SC111-CjJ40abpETA26',
        '0DR03-eEM9ab-CjJ40ab',    
        'Mult_jXESPARE8', # cTauMultiplicity COUNT
        'Mult_cTAU30M',
        'Mult_cTAU35M',
        'Mult_cTAUSPARE2',
    ]

    alg_data_list = algdata_from_menu(flags, root_names=root_names, do_dot=True)
    logger.info("number of alg_data " + str(len(alg_data_list)))
  
    [globalSimAlg.globalsim_algs.append(toolFromAlgData(flags, ad)) for
     ad in alg_data_list]

    acc.addEventAlgo(globalSimAlg)
    
    histSvc = CompFactory.THistSvc(Output =["EXPERT DATAFILE='expert-monitoring.root', OPT='RECREATE'"])
    acc.addService(histSvc)
    return acc

def add_subsystems(subsystems, acc, args):
    
    decoderTools = []
    outputEDM = []
    maybeMissingRobs = []

    from TrigT1ResultByteStream.TrigT1ResultByteStreamConfig import RoIBResultByteStreamToolCfg
    roibResultTool = acc.popToolsAndMerge(RoIBResultByteStreamToolCfg(flags, name="RoIBResultBSDecoderTool", writeBS=False))
    decoderTools += [roibResultTool]

    
    for module_id in roibResultTool.L1TopoModuleIds:
        maybeMissingRobs.append(int(SourceIdentifier(SubDetector.TDAQ_CALO_TOPO_PROC, module_id)))

    for module_id in roibResultTool.JetModuleIds:
        maybeMissingRobs.append(int(SourceIdentifier(SubDetector.TDAQ_CALO_JET_PROC_ROI, module_id)))

    for module_id in roibResultTool.EMModuleIds:
        maybeMissingRobs.append(int(SourceIdentifier(SubDetector.TDAQ_CALO_CLUSTER_PROC_ROI, module_id)))

  
    def addEDM(edmType, edmName):
        auxType = edmType.replace('Container','AuxContainer')
        return [f'{edmType}#{edmName}', f'{auxType}#{edmName}Aux.']
    
    if 'jFex' in subsystems:
        from L1CaloFEXByteStream.L1CaloFEXByteStreamConfig import (
            jFexRoiByteStreamToolCfg,jFexInputByteStreamToolCfg)
        
        
        jFexTool = acc.popToolsAndMerge(jFexRoiByteStreamToolCfg(
            flags, 'jFexBSDecoder', writeBS=False))
        
        decoderTools += [jFexTool]
        outputEDM += addEDM('xAOD::jFexSRJetRoIContainer',
                            jFexTool.jJRoIContainerWriteKey.Path)
        
        outputEDM += addEDM('xAOD::jFexLRJetRoIContainer',
                            jFexTool.jLJRoIContainerWriteKey.Path)
        
        outputEDM += addEDM('xAOD::jFexTauRoIContainer'  ,
                            jFexTool.jTauRoIContainerWriteKey.Path)
        
        outputEDM += addEDM('xAOD::jFexFwdElRoIContainer',
                            jFexTool.jEMRoIContainerWriteKey.Path)
        
        outputEDM += addEDM('xAOD::jFexSumETRoIContainer',
                            jFexTool.jTERoIContainerWriteKey.Path)
        outputEDM += addEDM('xAOD::jFexMETRoIContainer'  ,
                            jFexTool.jXERoIContainerWriteKey.Path)
        maybeMissingRobs += jFexTool.ROBIDs
        
        if args.doCaloInput:
            
            jFexInputByteStreamTool = acc.popToolsAndMerge(
                jFexInputByteStreamToolCfg(flags,
                                           'jFexInputBSDecoderTool',
                                           writeBS=False))
        
            decoderTools += [jFexInputByteStreamTool]
            outputEDM += addEDM('xAOD::jFexTowerContainer',
                                jFexInputByteStreamTool.jTowersWriteKey.Path)
            maybeMissingRobs += jFexInputByteStreamTool.ROBIDs

    if 'eFex' in subsystems:
        from L1CaloFEXByteStream.L1CaloFEXByteStreamConfig import (
            eFexByteStreamToolCfg,)
          
        eFexTool = acc.popToolsAndMerge(
            eFexByteStreamToolCfg(flags,
                                  'eFexBSDecoder',
                                  writeBS=False,
                                  decodeInputs=args.doCaloInput))
        
        decoderTools += [eFexTool]
        outputEDM += addEDM('xAOD::eFexEMRoIContainer',
                            eFexTool.eEMContainerWriteKey.Path)
        outputEDM += addEDM('xAOD::eFexTauRoIContainer',
                            eFexTool.eTAUContainerWriteKey.Path)

        if args.doCaloInput:
            outputEDM += addEDM('xAOD::eFexTowerContainer',
                                    eFexTool.eTowerContainerWriteKey.Path)

        maybeMissingRobs += eFexTool.ROBIDs

    if 'gFex' in subsystems:
        from L1CaloFEXByteStream.L1CaloFEXByteStreamConfig import (
            gFexByteStreamToolCfg,gFexInputByteStreamToolCfg,)
        
        gFexTool = acc.popToolsAndMerge(gFexByteStreamToolCfg(
            flags, 'gFexBSDecoder', writeBS=False))
        
        decoderTools += [gFexTool]
        outputEDM += addEDM(
            'xAOD::gFexJetRoIContainer',
            gFexTool.gFexRhoOutputContainerWriteKey.Path)
        
        outputEDM += addEDM(
            'xAOD::gFexJetRoIContainer',
            gFexTool.gFexSRJetOutputContainerWriteKey.Path)
      
        outputEDM += addEDM(
            'xAOD::gFexJetRoIContainer',
            gFexTool.gFexLRJetOutputContainerWriteKey.Path)
      
        outputEDM += addEDM(
            'xAOD::gFexGlobalRoIContainer',
            gFexTool.gScalarEJwojOutputContainerWriteKey.Path)
      
        outputEDM += addEDM(
            'xAOD::gFexGlobalRoIContainer',
            gFexTool.gMETComponentsJwojOutputContainerWriteKey.Path)
      
        outputEDM += addEDM(
            'xAOD::gFexGlobalRoIContainer',
            gFexTool.gMHTComponentsJwojOutputContainerWriteKey.Path)
      
        outputEDM += addEDM(
            'xAOD::gFexGlobalRoIContainer',
            gFexTool.gMSTComponentsJwojOutputContainerWriteKey.Path)
      
        outputEDM += addEDM(
            'xAOD::gFexGlobalRoIContainer',
            gFexTool.gMETComponentsNoiseCutOutputContainerWriteKey.Path)
      
        outputEDM += addEDM(
            'xAOD::gFexGlobalRoIContainer',
            gFexTool.gMETComponentsRmsOutputContainerWriteKey.Path)
      
        outputEDM += addEDM(
            'xAOD::gFexGlobalRoIContainer',
            gFexTool.gScalarENoiseCutOutputContainerWriteKey.Path)
      
        outputEDM += addEDM(
            'xAOD::gFexGlobalRoIContainer',
            gFexTool.gScalarERmsOutputContainerWriteKey.Path)
      
        maybeMissingRobs += gFexTool.ROBIDs
      
        if args.doCaloInput:
            gFexInputByteStreamTool = acc.popToolsAndMerge(
                gFexInputByteStreamToolCfg(
                    flags, 'gFexInputByteStreamTool', writeBS=False))
          
            decoderTools += [gFexInputByteStreamTool]
            outputEDM += addEDM('xAOD::gFexTowerContainer',
                                gFexInputByteStreamTool.gTowersWriteKey.Path)
          
            maybeMissingRobs += gFexInputByteStreamTool.ROBIDs

    decoderAlg = CompFactory.L1TriggerByteStreamDecoderAlg(
        name="L1TriggerByteStreamDecoder",
        DecoderTools=decoderTools,
        MaybeMissingROBs=maybeMissingRobs,
        OutputLevel=algLogLevel)
        
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    log.debug('Adding the following output EDM to ItemList: %s', outputEDM)

    acc.addEventAlgo(decoderAlg, sequenceName='AthAlgSeq')
    
    acc.merge(OutputStreamCfg(flags, 'AOD', ItemList=outputEDM))


if __name__ == '__main__':
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaCommon.Logging import logging

    from AthenaCommon.Constants import DEBUG

    log = logging.getLogger('globalSim')
    log.setLevel(DEBUG)
    algLogLevel = DEBUG

    import argparse
    from argparse import RawTextHelpFormatter

    parser = argparse.ArgumentParser(
        "Running L1TopoSimulation standalone for the BS input",
        formatter_class=RawTextHelpFormatter)

    parser.add_argument(
        "-i",
        "--inputs",
        nargs='*',
        action="store",
        dest="inputFiles",
        help="files to process",
        required=True)
    

    parser.add_argument(
        "-ifex",
        "--doCaloInput",
        action="store_true",
        dest="doCaloInput",
        help="Decoding L1Calo inputs",
        default=False,
        required=False)

    parser.add_argument(
        "-n",
        "--nevent",
        type=int,
        action="store",
        dest="nevent",
        help="Maximum number of events will be executed.",
        default=0,
        required=False)

    parser.add_argument(
        "-s",
        "--skipEvents",
        type=int,
        action="store",
        dest="skipEvents",
        help="Number of  events to skip.",
        default=0,
        required=False)

    args = parser.parse_args()

    logger.debug('args:')

    logger.debug(args)
    
    flags = initConfigFlags()
    
    if(args.nevent > 0):
        flags.Exec.MaxEvents = args.nevent

    if args.inputFiles:
        flags.Input.Files = args.inputFiles
    else:
        flags.Input.Files = ['/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/data23/RAW/data23_13p6TeV.00452463.physics_Main.daq.RAW/540events.data23_13p6TeV.00452463.physics_Main.daq.RAW._lb0514._SFO-16._0004.data']
        
  
    flags.Concurrency.NumThreads = 1
    flags.Concurrency.NumConcurrentEvents = 1
    flags.Exec.SkipEvents = args.skipEvents

    flags.Output.AODFileName = 'AOD.pool.root'
    flags.Trigger.triggerMenuSetup = 'PhysicsP1_pp_run3_v1'

    flags.GeoModel.AtlasVersion="ATLAS-R3S-2021-03-01-00"

    

    print (flags.dump())

    flags.lock()
    
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)

    from TriggerJobOpts.TriggerByteStreamConfig import ByteStreamReadCfg
    acc.merge(ByteStreamReadCfg(flags, type_names=['CTP_RDO/CTP_RDO']))

    # Generate run3 L1 menu
    from TrigConfigSvc.TrigConfigSvcCfg import L1ConfigSvcCfg, generateL1Menu
    acc.merge(L1ConfigSvcCfg(flags))
    generateL1Menu(flags)

    subsystems = ('jFex', 'eFex')
    add_subsystems(subsystems, acc, args)

    acc.merge(GlobalL1TopoSimulationCfg(flags, algLogLevel))
      
    
    roib2topo = CompFactory.LVL1.RoiB2TopoInputDataCnv(
        name='RoiB2TopoInputDataCnv')
    
    roib2topo.OutputLevel = algLogLevel
    acc.addEventAlgo(roib2topo, sequenceName="AthAlgSeq")

    from L1TopoByteStream.L1TopoByteStreamConfig import L1TopoByteStreamCfg
    acc.merge(L1TopoByteStreamCfg(flags), sequenceName='AthAlgSeq')
    
    if acc.run().isFailure():
        sys.exit(1)

    
