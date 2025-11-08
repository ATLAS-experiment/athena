#
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import Format

def gFexEmulatedTowersCfg(flags, name="L1_gFexEmulatedTowers", writeKey="L1_gFexEmulatedTowers"):
    """
    Config for emulating gFex input data from LATOME readout
    """
    acc=ComponentAccumulator()

    acc.addEventAlgo( CompFactory.LVL1.gFexTowerBuilder(name=name,
                                                             SCell=flags.Trigger.L1.L1CaloSuperCellContainerName,
                                                             gTowersWriteKey = writeKey,
                                                             isDATA = not flags.Input.isMC,
                                                             gFEX2SCmapping = "Run3L1CaloSimulation/L1CaloFEXAlgos/gFEX/gCaloTowers_to_scells_v1.txt",
                                                             gFEX2Tilemapping = "Run3L1CaloSimulation/L1CaloFEXAlgos/gFEX/gCaloTowers_to_tile_v1.txt",
                                                             gFexFiberTowerMapping = "Run3L1CaloSimulation/L1CaloFEXAlgos/gFEX/gFex_gCaloTowerMap_weighted_v1.txt") )
    return acc

def jFexEmulatedTowersCfg(flags, name="jFexEmulatedTowerMaker", writeKey="L1_jFexEmulatedTowers", SCin="", OfflineCaloCell=""):
    """
    Config for emulating jFex input data from LATOME readout
    Supply SCin to build from non-standard SCell container
    Supply OfflineCaloCell e.g. AllCalo to add offline cell energy decoration
    """
    acc=ComponentAccumulator()
    
    emulator = CompFactory.LVL1.jFexEmulatedTowers(name)
    emulator.SCell = flags.Trigger.L1.L1CaloSuperCellContainerName if not SCin else SCin
    emulator.jTowersWriteKey = writeKey
    emulator.isDATA = not flags.Input.isMC
    # only set the necessary containers, tools, decorations for non empty OfflineCaloCell
    emulator.CaloCell = OfflineCaloCell
    emulator.CaloSuperCellIDTool = "CaloSuperCellIDTool" if OfflineCaloCell else ""
    emulator.TileOfflineETKey = "CaloCellETByLayer" if (OfflineCaloCell and not flags.Input.isMC) else ""
    emulator.CaloCellSumEtdecorKey = "CaloCellSumEt" if OfflineCaloCell else ""

    acc.addEventAlgo(emulator)

    return acc

def eFexEmulatedTowersCfg(flags, name, writeKey = "L1_eFexEmulatedTowers"):
    """
    Config for emulating eFex input data from LATOME readout
    """    
    acc=ComponentAccumulator()
    
    emulator = CompFactory.LVL1.eFexTowerBuilder(name,ApplyMasking=not flags.Input.isMC)
    emulator.eFexContainerWriteKey   = writeKey
    acc.addEventAlgo(emulator)

    return acc


if __name__ == '__main__':
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaCommon.Logging import logging
    import glob
    import sys

    import argparse
    parser = argparse.ArgumentParser(prog='python -m L1CaloFEXAlgos.FexEmulatedTowersConfig',
                                   description="""Emulator tools for FEX towers athena script.\n\n
                                   Example: python -m L1CaloFEXAlgos.FexEmulatedTowersConfig --filesInput "data22*" --evtMax 10 --outputs jTowers """)
    parser.add_argument('--evtMax',type=int,default=-1,help="number of events")
    parser.add_argument('--filesInput',nargs='+',help="input files",required=True)
    parser.add_argument('--outputs',nargs='+',choices={"jTowers","eTowers"},required=True, help="What data to decode and emulate")
    parser.add_argument('--outputLevel',default="WARNING",choices={ 'INFO','WARNING','DEBUG','VERBOSE'})
    args = parser.parse_args()


    log = logging.getLogger('FexEmulatedTowersConfig')
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

    if not flags.Input.isMC:
        from AthenaConfiguration.TestDefaults import defaultGeometryTags
        flags.GeoModel.AtlasVersion = defaultGeometryTags.autoconfigure(flags)

    if any(["data" in f for f in args.filesInput]):
        s=args.filesInput[0].replace('*','').replace('.data','')
        flags.Output.AODFileName = "AOD."+(s.split("/")[-1]).split('_SFO')[0]+"pool.root"
    else:
        flags.Output.AODFileName = 'AOD.pool.root'  

    flags.Trigger.EDMVersion = 3
    flags.Trigger.doLVL1 = True
    flags.Trigger.enableL1CaloPhase1 = True

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
    # LAr and Tile information   
    ########################################
    
    # Decodes LATOME into SCell container
    from L1CaloFEXSim.L1CaloFEXSimCfg import ReadSCellFromByteStreamCfg,TriggerTowersInputCfg
    acc.merge(ReadSCellFromByteStreamCfg(flags))
    
    # Creates the TriggerTower container
    acc.merge(TriggerTowersInputCfg(flags))
    

    ########################################
    # Emulated jFex Towers
    ########################################
    if 'jTowers' in args.outputs:
        acc.merge(jFexEmulatedTowersCfg(flags,'jFexEmulatedTowers'))
        outputEDM += addEDM('xAOD::jFexTowerContainer', 'L1_jFexEmulatedTowers')
        
        # decode any data towers for comparison with emulated
        from L1CaloFEXByteStream.L1CaloFEXByteStreamConfig import jFexInputByteStreamToolCfg
        inputjFexTool = acc.popToolsAndMerge(jFexInputByteStreamToolCfg(flags, 'jFexInputBSDecoder'))
        for module_id in inputjFexTool.ROBIDs:
            maybeMissingRobs.append(module_id)

        decoderTools += [inputjFexTool]
        # saving/adding the jTower xAOD container
        outputEDM += addEDM('xAOD::jFexTowerContainer', inputjFexTool.jTowersWriteKey.Path)

    ########################################
    # Emulated eFex     
    ########################################
    if 'eTowers' in args.outputs:
        acc.merge(eFexEmulatedTowersCfg(flags,'L1_eFexEmulatedTowers'))
        outputEDM += addEDM('xAOD::eFexTowerContainer', 'L1_eFexEmulatedTowers')

        # decode any data towers for comparison with emulated
        from L1CaloFEXByteStream.L1CaloFEXByteStreamConfig import eFexByteStreamToolCfg
        inputeFexTool = acc.popToolsAndMerge(eFexByteStreamToolCfg(
            flags,'eFexBSDecoder',TOBs=False,xTOBs=False,decodeInputs=True))
        for module_id in inputeFexTool.ROBIDs:
            maybeMissingRobs.append(module_id)    

        decoderTools += [inputeFexTool]
        # saving/adding the eTower xAOD container
        outputEDM += addEDM('xAOD::eFexTowerContainer', 'L1_eFexDataTowers')

    #
    decoderAlg = CompFactory.L1TriggerByteStreamDecoderAlg(name="L1TriggerByteStreamDecoder",
                                                           DecoderTools=decoderTools, OutputLevel=algLogLevel, 
                                                           MaybeMissingROBs=maybeMissingRobs)

    acc.addEventAlgo(decoderAlg, sequenceName='AthAlgSeq')
    

    # Saving containers
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    log.debug('Adding the following output EDM to ItemList: %s', outputEDM)
    acc.merge(OutputStreamCfg(flags, 'AOD', ItemList=outputEDM))

    acc.getEventAlgo("EventInfoTagBuilder").PropagateInput = (flags.Input.Format != Format.BS)

    if acc.run().isFailure():
        sys.exit(1)
