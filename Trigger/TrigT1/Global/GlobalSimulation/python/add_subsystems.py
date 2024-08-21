# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from libpyeformat_helper import SourceIdentifier, SubDetector
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator


from AthenaCommon.Constants import DEBUG
from AthenaCommon.Logging import logging
logger = logging.getLogger('add_subsystems')
logger.setLevel(DEBUG)

def add_subsystems(flags, subsystems,  args, OutputLevel):

    acc = ComponentAccumulator()

    decoderTools = []
    outputEDM = []
    maybeMissingRobs = []
    
    from TrigT1ResultByteStream.TrigT1ResultByteStreamConfig import (
        RoIBResultByteStreamToolCfg,)
    
    roibResultTool = acc.popToolsAndMerge(
        RoIBResultByteStreamToolCfg(flags,
                                    name="RoIBResultBSDecoderTool",
                                    writeBS=False))
    
    decoderTools += [roibResultTool]

    
    for module_id in roibResultTool.L1TopoModuleIds:
        maybeMissingRobs.append(
            int(SourceIdentifier(SubDetector.TDAQ_CALO_TOPO_PROC, module_id)))

    for module_id in roibResultTool.JetModuleIds:
        maybeMissingRobs.append(
            int(SourceIdentifier(SubDetector.TDAQ_CALO_JET_PROC_ROI,
                                 module_id)))

    for module_id in roibResultTool.EMModuleIds:
        maybeMissingRobs.append(int(SourceIdentifier(
            SubDetector.TDAQ_CALO_CLUSTER_PROC_ROI, module_id)))

  
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
        OutputLevel=OutputLevel)
        
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    logger.debug('Adding the following output EDM to ItemList: %s', outputEDM)

    acc.addEventAlgo(decoderAlg, sequenceName='AthAlgSeq')
    
    acc.merge(OutputStreamCfg(flags, 'AOD', ItemList=outputEDM))

    return acc
