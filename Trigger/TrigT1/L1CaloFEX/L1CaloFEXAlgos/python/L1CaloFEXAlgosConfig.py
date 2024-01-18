#
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from L1CaloFEXByteStream.L1CaloFEXByteStreamConfig import jFexInputByteStreamToolCfg
from L1CaloFEXByteStream.L1CaloFEXByteStreamConfig import gFexInputByteStreamToolCfg
from AthenaConfiguration.Enums import Format


def L1CalojFEXDecoratorCfg(flags, name="jFexTower2SCellDecorator", jTowersReadKey="L1_jFexDataTowers", ExtraInfo = False, SCMasking = True):

    acc=ComponentAccumulator()
    
    decorator = CompFactory.LVL1.jFexTower2SCellDecorator(name)
    decorator.SCell = flags.Trigger.L1.L1CaloSuperCellContainerName 
    decorator.jTowersReadKey = jTowersReadKey 
    decorator.ExtraInfo = ExtraInfo
    decorator.SCellMasking = SCMasking
    acc.addEventAlgo(decorator)

    return acc

def L1CaloGTowerDecoratorCfg(flags, name, gTowersReadKey = 'L1_gFexDataTowers'):

    acc=ComponentAccumulator() 
    acc.addEventAlgo( CompFactory.LVL1.gFexTower2SCellDecorator(name, gTowersReadKey=gTowersReadKey) )

    return acc

def eFexTOBDecoratorCfg(flags, name, eFexEMRoIContainer = "L1_eEMRoI", eFexTauRoIContainer = "L1_eTauRoI"):
    """
    Configure the eFEX TOB decorator algorithm
    Requires the eFEXTOBEtTool
    """
    acc = ComponentAccumulator()

    from L1CaloFEXSim.L1CaloFEXSimCfg import eFEXTOBEtToolCfg
    acc.popToolsAndMerge(eFEXTOBEtToolCfg(flags))

    decorator = CompFactory.LVL1.eFexTOBDecorator(name, eFexEMRoIContainer = eFexEMRoIContainer, eFexTauRoIContainer = eFexTauRoIContainer)

    # in case the TOB containers are different from default we also have to change the write handles
    if eFexEMRoIContainer != "L1_eEMRoI":
        decorator.RetaCoreDecDecorKey = eFexEMRoIContainer+".RetaCoreDec"
        decorator.RetaEnvDecDecorKey = eFexEMRoIContainer+".RetaEnvDec"
        decorator.RetaEMDecDecorKey = eFexEMRoIContainer+".RhadEMDec"
        decorator.RhadHadDecDecorKey = eFexEMRoIContainer+".RhadHadDec"
        decorator.WstotDenDecDecorKey = eFexEMRoIContainer+".WstotDenDec"
        decorator.WstotNumDecDecorKey = eFexEMRoIContainer+".WstotNumDec"
        decorator.ClusterEtSumPSDecorKey = eFexEMRoIContainer+".ClusterEtSumPS"
        decorator.ClusterEtSumL1DecorKey = eFexEMRoIContainer+".ClusterEtSumL1"
        decorator.ClusterEtSumL2DecorKey = eFexEMRoIContainer+".ClusterEtSumL2"
        decorator.ClusterEtSumL3DecorKey = eFexEMRoIContainer+".ClusterEtSumL3"

    if eFexEMRoIContainer != "L1_eTauRoI":
        decorator.RCoreDecorKey = eFexTauRoIContainer+".RCoreDec"
        decorator.REnvDecorKey = eFexTauRoIContainer+".REnvDec"
        decorator.REMCoreDecorKey = eFexTauRoIContainer+".REMCoreDec"
        decorator.REMHadDecorKey = eFexTauRoIContainer+".REMHadDec"

    acc.addEventAlgo(decorator)

    return acc

if __name__ == '__main__':
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaCommon.Logging import logging
    import glob
    import sys

    import argparse
    parser = argparse.ArgumentParser(prog='python -m L1CaloFEXAlgos.L1CaloFEXAlgosConfig',
                                   description="""Decorator tool for FEX towers athena script.\n\n
                                   Example: python -m L1CaloFEXAlgos.L1CaloFEXAlgosConfig --filesInput "data22*" --evtMax 10 --outputs eTOBs """)
    parser.add_argument('--evtMax',type=int,default=-1,help="number of events")
    parser.add_argument('--filesInput',nargs='+',help="input files",required=True)
    parser.add_argument('--outputLevel',default="WARNING",choices={ 'INFO','WARNING','DEBUG','VERBOSE'})
    parser.add_argument('--outputs',nargs='+',choices={"jTowers", "gTowers", "jTOBs","eTOBs"},required=True, help="What data to decode and output.")
    args = parser.parse_args()


    log = logging.getLogger('L1CaloFEXAlgosConfig')
    log.setLevel(logging.DEBUG)

    from AthenaCommon import Constants
    algLogLevel = getattr(Constants,args.outputLevel)

    flags = initConfigFlags()
    if any(["data" in f for f in args.filesInput]):
        flags.Trigger.triggerConfig='DB'

    flags.Exec.OutputLevel = algLogLevel
    flags.Exec.MaxEvents = args.evtMax
    flags.Input.Files = [file for x in args.filesInput for file in glob.glob(x)]
    flags.Concurrency.NumThreads = 1
    flags.Concurrency.NumConcurrentEvents = 1
  
    if any(["data" in f for f in args.filesInput]):
        s=args.filesInput[0].replace('*','').replace('.data','')
        flags.Output.AODFileName = "AOD."+(s.split("/")[-1]).split('_SFO')[0]+"pool.root"
    else:
        flags.Output.AODFileName = 'AOD.pool.root'  

    flags.Trigger.EDMVersion = 3
    flags.Trigger.doLVL1 = True
    flags.Trigger.enableL1CaloPhase1 = True

    from AthenaConfiguration.Enums import LHCPeriod
    if not flags.Input.isMC and flags.GeoModel.Run is LHCPeriod.Run2:
        flags.GeoModel.AtlasVersion = 'ATLAS-R2-2016-01-00-01'

    # Enable only calo for this test
    from AthenaConfiguration.DetectorConfigFlags import setupDetectorsFromList
    setupDetectorsFromList(flags,['LAr','Tile','MBTS'],True)

    flags.lock()

    
    # Set up the main service "acc"
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)
    
    # The decoderAlg needs to load ByteStreamMetadata for the detector mask
    from TriggerJobOpts.TriggerByteStreamConfig import ByteStreamReadCfg
    acc.merge(ByteStreamReadCfg(flags))
        
    # Generate run3 L1 menu
    from TrigConfigSvc.TrigConfigSvcCfg import L1ConfigSvcCfg
    acc.merge(L1ConfigSvcCfg(flags))
    
    decoderTools = []
    outputEDM = []
    maybeMissingRobs = []

    def addEDM(edmType, edmName):
        auxType = edmType.replace('Container','AuxContainer')
        return [f'{edmType}#{edmName}',
            f'{auxType}#{edmName}Aux.']

    ########################################
    # jFEX ROIs
    ########################################
    if 'jTOBs' in args.outputs:
        from L1CaloFEXByteStream.L1CaloFEXByteStreamConfig import jFexRoiByteStreamToolCfg
        jFexTool = acc.popToolsAndMerge(jFexRoiByteStreamToolCfg(flags, 'jFexBSDecoder'))
        for module_id in jFexTool.ROBIDs:
            maybeMissingRobs.append(module_id)

        decoderTools += [jFexTool]


    ########################################
    # jFEX input Data
    ########################################
    if 'jTowers' in args.outputs:
        inputjFexTool = acc.popToolsAndMerge(jFexInputByteStreamToolCfg(flags, 'jFexInputBSDecoder'))
        for module_id in inputjFexTool.ROBIDs:
            maybeMissingRobs.append(module_id)

        decoderTools += [inputjFexTool]
        # saving/adding the jTower xAOD container
        outputEDM += addEDM('xAOD::jFexTowerContainer', inputjFexTool.jTowersWriteKey.Path)


    ########################################
    # eFEX ROIs
    ########################################
    if 'eTOBs' in args.outputs:
        from L1CaloFEXByteStream.L1CaloFEXByteStreamConfig import eFexByteStreamToolCfg
        eFexTool = acc.popToolsAndMerge(eFexByteStreamToolCfg(flags, 'eFexBSDecoder', TOBs=True, xTOBs=False, decodeInputs=False))
        for module_id in eFexTool.ROBIDs:
            maybeMissingRobs.append(module_id)

        decoderTools += [eFexTool]
        # saving/adding EDM
        outputEDM += addEDM('xAOD::eFexEMRoIContainer', eFexTool.eEMContainerWriteKey.Path)
        outputEDM += addEDM('xAOD::eFexTauRoIContainer', eFexTool.eTAUContainerWriteKey.Path)


    # Add the decoder tools
    decoderAlg = CompFactory.L1TriggerByteStreamDecoderAlg(name="L1TriggerByteStreamDecoder",
                                                           DecoderTools=decoderTools, OutputLevel=algLogLevel, MaybeMissingROBs=maybeMissingRobs)
    acc.addEventAlgo(decoderAlg, sequenceName='AthAlgSeq')

    ########################################
    # gFEX ROIs
    ########################################
    if 'gTOBs' in args.outputs:
        from L1CaloFEXByteStream.L1CaloFEXByteStreamConfig import gFexByteStreamToolCfg
        gFexTool = acc.popToolsAndMerge(gFexByteStreamToolCfg(flags, 'gFexBSDecoder'))
        decoderTools += [gFexTool]


    ########################################
    # gFEX input Data
    ########################################
    if 'gTowers' in args.outputs:
        inputgFexTool = acc.popToolsAndMerge(gFexInputByteStreamToolCfg(flags, 'gFexInputBSDecoder'))
        decoderTools += [inputgFexTool]
        # saving/adding the gTower xAOD container
        outputEDM += addEDM('xAOD::gFexTowerContainer', inputgFexTool.gTowersWriteKey.Path)

    decoderAlg = CompFactory.L1TriggerByteStreamDecoderAlg(name="L1TriggerByteStreamDecoder",
                                                         DecoderTools=decoderTools, OutputLevel=algLogLevel)

    acc.addEventAlgo(decoderAlg, sequenceName='AthAlgSeq')

    ########################################
    # Decorators   
    ########################################
    
    # Decodes LATOME into SCell container
    from L1CaloFEXSim.L1CaloFEXSimCfg import ReadSCellFromByteStreamCfg,TriggerTowersInputCfg
    acc.merge(ReadSCellFromByteStreamCfg(flags))
    
    # Creates the TriggerTower container
    acc.merge(TriggerTowersInputCfg(flags))
    
    # Uses SCell to decorate the jTowers (with extra info)
    if 'jTowers' in args.outputs:
        DecoratorAlgo = L1CalojFEXDecoratorCfg(flags, ExtraInfo = True)   
        acc.merge(DecoratorAlgo)

    # Decorate eFEX RoIs
    if 'eTOBs' in args.outputs:
        DecoratorAlgo = eFexTOBDecoratorCfg(flags,'eFexTOBDecorator')
        acc.merge(DecoratorAlgo)

    # Uses SCell to decorate the gTowers
    if 'gTowers' in args.outputs:
        gTowerDecoratorAlgo = L1CaloGTowerDecoratorCfg(flags, 'gFexTower2SCellDecorator')   
        acc.merge(gTowerDecoratorAlgo)


    # Saving containers
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    log.debug('Adding the following output EDM to ItemList: %s', outputEDM)
    acc.merge(OutputStreamCfg(flags, 'AOD', ItemList=outputEDM))

    acc.getEventAlgo("EventInfoTagBuilder").PropagateInput = (flags.Input.Format != Format.BS)

    if acc.run().isFailure():
        sys.exit(1)
