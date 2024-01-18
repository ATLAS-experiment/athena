# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import Format
from TrigT1ResultByteStream.TrigT1ResultByteStreamConfig import L1TriggerByteStreamDecoderCfg
from TrigConfigSvc.TrigConfigSvcCfg import TrigConfigSvcCfg
from TriggerJobOpts.TriggerByteStreamConfig import ByteStreamReadCfg
from TrigEDMConfig.TriggerEDM import getTriggerEDMList
from TrigEDMConfig.Utils import edmDictToList
from OutputStreamAthenaPool.OutputStreamConfig import addToAOD, addToESD

from AthenaCommon.Logging import logging
log = logging.getLogger('TriggerRecoConfig')


def TriggerRecoCfg(flags):
    if flags.Input.isMC:
        return TriggerRecoCfgMC(flags)
    else:
        return TriggerRecoCfgData(flags)

def TriggerRecoCfgData(flags):
    """
    Configures trigger data decoding
    Run 3 data:
    HLTResultMTByteStreamDecoderAlg -> TriggerEDMDeserialiserAlg

    Run 2 data:
    TrigBSExtraction -> TrigDecisionMaker -> DecisionConv to xAOD -> NavigationConv to xAOD

    Run 1 data:
    as for Run 2 + Run 1 EDM to xAOD conversion
    """
    log.debug("TriggerRecoCfgData: Preparing the trigger handling of reconstruction of data")
    acc = ComponentAccumulator()
    acc.merge( ByteStreamReadCfg(flags) )
    if flags.Trigger.L1.doMuon or flags.Trigger.L1.doCalo or flags.Trigger.L1.doTopo or flags.Trigger.L1.doCTP:
        acc.merge( L1TriggerByteStreamDecoderCfg(flags) )

    metadataAcc, _ = TriggerMetadataWriterCfg(flags)
    acc.merge( metadataAcc )

    # Run 3+
    if flags.Trigger.EDMVersion >= 3:
        acc.merge(Run3TriggerBSUnpackingCfg(flags))

        from TrigDecisionMaker.TrigDecisionMakerConfig import Run3DecisionMakerCfg
        acc.merge(Run3DecisionMakerCfg(flags))

        if flags.Trigger.doNavigationSlimming:
            from TrigNavSlimmingMT.TrigNavSlimmingMTConfig import TrigNavSlimmingMTCfg
            acc.merge(TrigNavSlimmingMTCfg(flags))

    # Run 1+2
    elif flags.Trigger.EDMVersion in [1, 2]:
        acc.merge( Run1Run2BSExtractionCfg(flags) )

        from TrigDecisionMaker.TrigDecisionMakerConfig import Run1Run2DecisionMakerCfg
        acc.merge (Run1Run2DecisionMakerCfg(flags) )

        acc.merge(Run2Run1NavigationSlimmingCfg(flags))
    else:
        raise RuntimeError("Invalid EDMVersion=%s " % flags.Trigger.EDMVersion)

    # Legacy L1Calo, L1Topo reco
    if flags.Trigger.enableL1CaloLegacy:
        from AnalysisTriggerAlgs.AnalysisTriggerAlgsConfig import RoIBResultToxAODCfg
        xRoIBResultAcc, _ = RoIBResultToxAODCfg(flags)
        acc.merge( xRoIBResultAcc )

        if flags.Input.Format is Format.BS:
            from L1TopoByteStream.L1TopoByteStreamConfig import L1TopoRawDataContainerBSCnvCfg
            acc.merge( L1TopoRawDataContainerBSCnvCfg(flags) )
            topoEDM = ['xAOD::L1TopoRawDataContainer#L1TopoRawData',
                       'xAOD::L1TopoRawDataAuxContainer#L1TopoRawDataAux.']
            acc.merge(addToESD(flags, topoEDM))
            acc.merge(addToAOD(flags, topoEDM))

    acc.merge(TriggerEDMCfg(flags))

    return acc

def TriggerRecoCfgMC(flags):
    """
    Configures trigger MC handing during reconstruction
    Note that a lot of the T0 handling of the trigger output for data is bundled into
    the RDO to RDO_TRIG sub-step for MC, hence there is much less to do here for MC.
    Notably: the xAOD::TrigDecision and all metadata should already be in the RDO_TRIG.

    Run 3 MC:
    Propagation of Run 3 HLT collections from input RDO_TRIG to output POOL files.
    Execution of Run-3 reconstruction-level trigger navigation slimming.

    Run 2 MC:
    Propagation of Run 2 HLT collections from input RDO_TRIG to output POOL files.
    Execution of Run 2 style reconstruction-level trigger navigation slimming.
    Optional conversion of Run 2 navigation to Run 3 navigation.
    Optional execution of Run 3 reconstruction-level trigger navigation slimming on conversion output.
 
    Run 1 MC:
    No current workflows are able to run the Run 1 trigger MC simulation. Unsupported.
    Work would be needed here to do the xAOD conversion (see Run1xAODConversionCfg for how this is done for data)
    """

    # Check for currently unsupported operational modes, these may be supported in the future if needed
    if flags.Input.Format is Format.BS:
        log.warning("TriggerRecoCfgMC does not currently support MC files encoded as bytestream. Switching off handling of trigger inputs.")
        return ComponentAccumulator()
    if flags.Trigger.EDMVersion == 1:
        log.warning("TriggerRecoCfgMC does not currently support MC files with Run 1 trigger payload. Switching off handling of trigger inputs.")
        return ComponentAccumulator()

    log.debug("TriggerRecoCfgMC: Preparing the trigger handling of reconstruction of MC")
    acc = ComponentAccumulator()

    # This will kick into action for Run 2 (Run-1 gets blocked above)
    acc.merge(Run2Run1NavigationSlimmingCfg(flags))

    # This may kick into action for Run 2, based on flags.Trigger.doEDMVersionConversion
    from TrigNavTools.NavConverterConfig import NavConverterCfg
    acc.merge(NavConverterCfg(flags, chainsFilter = ["HLT_.*"])) # Derivations use the TriggerAPI here, but at RDO_TRIG->AOD/ESD we should probably convert everything.

    # This will kick into action for Run 3, and may for Run 2 if based on the navigation conversion above
    from TrigNavSlimmingMT.TrigNavSlimmingMTConfig import TrigNavSlimmingMTCfg
    acc.merge(TrigNavSlimmingMTCfg(flags))

    acc.merge(TriggerEDMCfg(flags))

    return acc

def TriggerMetadataWriterCfg(flags):
    """Sets up access to HLT, L1, BGRP, Monitoring, HLT PS and L1 PS JSON files from 'FILE' or 'DB', writes JSON to metaStore and keys to eventStore"""
    acc = ComponentAccumulator()
    keyWriterOutput = ""
    if flags.Trigger.triggerConfig != 'INFILE':
        acc.merge( TrigConfigSvcCfg(flags) )
        keyWriterTool = CompFactory.TrigConf.KeyWriterTool("KeyWriterToolOffline")
        keyWriterOutput = str(keyWriterTool.ConfKeys)
        acc.addEventAlgo( CompFactory.TrigConf.xAODMenuWriterMT("xAODMenuWriterMT", KeyWriterTool = keyWriterTool) )
    return acc, keyWriterOutput

def TriggerEDMCfg(flags):
    """Configures which trigger collections are recorded"""
    acc = ComponentAccumulator()

    # Check if we have anything to do
    if flags.Output.doWriteESD is False and flags.Output.doWriteAOD is False:
        log.debug("TriggerEDMCfg: Nothing to do as both Output.doWriteAOD and Output.doWriteESD are False")
        return acc

    # standard collections & metadata
    # TODO consider unifying with TriggerConfig.triggerPOOLOutputCfg - there the assumption is that Run3 
    # metadata
    menuMetadata = ["xAOD::TriggerMenuJsonContainer#*", "xAOD::TriggerMenuJsonAuxContainer#*",]
    if flags.Trigger.EDMVersion in [1,2]:
        menuMetadata += ['xAOD::TriggerMenuAuxContainer#*', 'xAOD::TriggerMenuContainer#*',]
        # Add LVL1 collections (for Run-3 they are part of the "regular" EDM lists)
        from TrigEDMConfig.TriggerEDM import getLvl1ESDList, getLvl1AODList
        acc.merge(addToESD(flags, edmDictToList(getLvl1ESDList())))
        acc.merge(addToAOD(flags, edmDictToList(getLvl1AODList())))

    edmVersion = max(2, flags.Trigger.EDMVersion)
    _TriggerESDList = getTriggerEDMList(flags.Trigger.ESDEDMSet, edmVersion, flags.Trigger.ExtraEDMList)
    _TriggerAODList = getTriggerEDMList(flags.Trigger.AODEDMSet, edmVersion, flags.Trigger.ExtraEDMList)
    log.debug("ESD EDM list: %s", _TriggerESDList)
    log.debug("AOD EDM list: %s", _TriggerAODList)
    
    # Highlight what is in AOD list but not in ESD list, as this can cause
    # the "different number of entries in branch" problem, when it is in the
    # AOD list but the empty container per event is not created
    # Just compares keys of dicts, which are the class names, not their string keys in StoreGate
    not_in = [ element for element in  _TriggerAODList if element not in _TriggerESDList ]
    if (len(not_in)>0):
        log.warning("In AOD list but not in ESD list: ")
        log.warning(not_in)
    else:
        log.info("AOD list is subset of ESD list - good.")

    # there is internal gating in addTo* if AOD or ESD do not need to be written out
    acc.merge(addToESD(flags, edmDictToList(_TriggerESDList), MetadataItemList = menuMetadata))
    acc.merge(addToAOD(flags, edmDictToList(_TriggerAODList), MetadataItemList = menuMetadata))
    
    log.info("AOD content set according to the AODEDMSet flag: %s and EDM version %d", flags.Trigger.AODEDMSet, flags.Trigger.EDMVersion)
    # navigation for Run 3
    if flags.Trigger.EDMVersion == 3 and not flags.Trigger.doOnlineNavigationCompactification and not flags.Trigger.doNavigationSlimming:
        nav = ['xAOD::TrigCompositeContainer#HLTNav*', 'xAOD::TrigCompositeAuxContainer#HLTNav*',]
        acc.merge(addToAOD(flags, nav))
        acc.merge(addToESD(flags, nav))
    # extra jet keys
    jetSpecials = ["JetKeyDescriptor#JetKeyMap", "JetMomentMap#TrigJetRecMomentMap",]
    acc.merge(addToESD(flags, jetSpecials))
    acc.merge(addToAOD(flags, jetSpecials))

    # RoIs
    if flags.Output.doWriteAOD and flags.Trigger.EDMVersion == 2:
        from TrigRoiConversion.TrigRoiConversionConfig import RoiWriterCfg
        acc.merge(RoiWriterCfg(flags))

    return acc

def Run2Run1NavigationSlimmingCfg(flags):
    """Configures legacy Run1/2 navigation slimming"""
    acc = ComponentAccumulator()

    if flags.Trigger.decodeHLT is False:
        log.debug("Run2Run1NavigationSlimmingCfg: Nothing to do as Trigger.decodeHLT is False")
        return acc

    if flags.Trigger.doNavigationSlimming is False:
        log.debug("Run2Run1NavigationSlimmingCfg: Nothing to do as Trigger.doNavigationSlimming is False")
        return acc

    if flags.Trigger.EDMVersion >= 3:
        log.debug("Run2Run1NavigationSlimmingCfg: Nothing to do for EDMVersion >= 3.")
        return acc

    if flags.Trigger.ExtraEDMList:
        log.warning("Run2Run1NavigationSlimmingCfg: ExtraEDMList only works for Run 3 and beyond")

    def _flatten(edm):
        return list(y.split('-')[0] for x in edm.values() for y in x)
    from TrigNavTools.TrigNavToolsConfig import TrigNavigationThinningSvcCfg
    
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg

    if flags.Output.doWriteAOD:
        _TriggerAODList = getTriggerEDMList(flags.Trigger.AODEDMSet,  flags.Trigger.EDMVersion)
        thinningSvc = acc.getPrimaryAndMerge(TrigNavigationThinningSvcCfg(flags, 
                                                                          {'name' : 'HLTNav_StreamAOD',
                                                                           'mode' : 'cleanup_noreload', 
                                                                           'result' : 'HLTResult_HLT',
                                                                           'features' : _flatten(_TriggerAODList)}))
        acc.merge(OutputStreamCfg(flags, "AOD", trigNavThinningSvc = thinningSvc))

    if flags.Output.doWriteESD:
        _TriggerESDList = getTriggerEDMList(flags.Trigger.ESDEDMSet,  flags.Trigger.EDMVersion)
        thinningSvc = acc.getPrimaryAndMerge(TrigNavigationThinningSvcCfg(flags,
                                                                          {'name' : 'HLTNav_StreamESD',
                                                                           'mode' : 'cleanup_noreload', 
                                                                           'result' : 'HLTResult_HLT',
                                                                           'features' : _flatten(_TriggerESDList)}))
        acc.merge(OutputStreamCfg(flags, "ESD", trigNavThinningSvc = thinningSvc))

    return acc


def Run1Run2BSExtractionCfg( flags ):
    """Configures Trigger data from BS extraction """
    from SGComps.AddressRemappingConfig import InputRenameCfg

    acc = ComponentAccumulator()
    extr = CompFactory.TrigBSExtraction()
    robIDMap = {}   # map of result keys and their ROB ID

    # Add fictional output to ensure data dependency in AthenaMT
    extr.ExtraOutputs.add(("TrigBSExtractionOutput", "StoreGateSvc+TrigBSExtractionOutput"))

    if flags.Trigger.decodeHLT:
        # Run-1: add xAOD conversion tool
        if flags.Trigger.EDMVersion == 1 and flags.Trigger.doxAODConversion:
            extr.BStoxAOD = acc.popToolsAndMerge( Run1xAODConversionCfg(flags) )

        serialiserTool = CompFactory.TrigTSerializer()
        acc.addPublicTool(serialiserTool)
        extr.NavigationForL2 = CompFactory.HLT.Navigation("NavigationForL2", 
                                                          ClassesFromPayloadIgnore = ["TrigPassBits#passbits"]) # Ignore the L2 TrigPassBits to avoid clash with EF (ATR-23411)

        extr.Navigation = CompFactory.HLT.Navigation("Navigation")
        from TrigEDMConfig.TriggerEDM import getEDMLibraries
        extr.Navigation.Dlls = getEDMLibraries()            
        from TrigEDMConfig.TriggerEDM import getPreregistrationList
        extr.Navigation.ClassesToPreregister = getPreregistrationList(flags.Trigger.EDMVersion, flags.Trigger.doxAODConversion)
        from eformat import helper as efh
 
        if flags.Trigger.EDMVersion == 1:  # Run-1 has L2 and EF result
            acc.merge(InputRenameCfg("HLT::HLTResult", "HLTResult_L2", "HLTResult_L2_BS"))
            acc.merge(InputRenameCfg("HLT::HLTResult", "HLTResult_EF", "HLTResult_EF_BS"))
            robIDMap["HLTResult_L2_BS"] = efh.SourceIdentifier(efh.SubDetector.TDAQ_LVL2, 0).code()
            robIDMap["HLTResult_EF_BS"] = efh.SourceIdentifier(efh.SubDetector.TDAQ_EVENT_FILTER, 0).code()
            extr.L2ResultKeyIn = "HLTResult_L2_BS"
            extr.L2ResultKeyOut = "HLTResult_L2"
            extr.HLTResultKeyIn = "HLTResult_EF_BS"
            extr.HLTResultKeyOut = "HLTResult_EF"
        else:
            acc.merge(InputRenameCfg("HLT::HLTResult", "HLTResult_HLT", "HLTResult_HLT_BS"))
            robIDMap["HLTResult_HLT_BS"] = efh.SourceIdentifier(efh.SubDetector.TDAQ_HLT, 0).code()
            extr.HLTResultKeyIn = "HLTResult_HLT_BS"
            extr.HLTResultKeyOut = "HLTResult_HLT"
        
        # Configure Run-2 DataScouting
        if flags.Trigger.EDMVersion == 2:
          stream = flags.Input.TriggerStream
          if stream.startswith('calibration_DataScouting_'):
              ds_tag = '_'.join(stream.split('_')[1:3])   # e.g. DataScouting_05
              ds_id = int(stream.split('_')[2])           # e.g. 05
              acc.merge(InputRenameCfg("HLT::HLTResult", ds_tag, ds_tag+"_BS"))
              robIDMap[ds_tag+"_BS"] = efh.SourceIdentifier(efh.SubDetector.TDAQ_HLT, ds_id).code()
              extr.DSResultKeysIn += [ ds_tag+"_BS" ]
              extr.DSResultKeysOut += [ ds_tag ]

    else:
        log.info("Will not schedule real HLT bytestream extraction, instead EDM gap filling is running")
        # if data doesn't have HLT info set HLTResult keys as empty strings to avoid warnings
        # but the extraction algorithm must run
        extr.HLTResultKeyIn = ""
        extr.HLTResultKeyOut = ""

    HLTResults = [ f"HLT::HLTResult/{k}" for k in robIDMap.keys() ]
    acc.addService( CompFactory.ByteStreamAddressProviderSvc( TypeNames = HLTResults) )

    from TrigEDMConfig.TriggerEDM import getTPList
    acc.addPublicTool( CompFactory.TrigSerTPTool(TPMap = getTPList((flags.Trigger.EDMVersion))) )
    
    acc.addPublicTool( CompFactory.TrigSerializeConvHelper(doTP = True) )

    acc.addPublicTool( CompFactory.HLT.HLTResultByteStreamTool(HLTResultRobIdMap = robIDMap))

    acc.addEventAlgo(extr)

    return acc

def Run1xAODConversionCfg(flags):
    """Convert Run 1 EDM collections to xAOD classes"""
    acc = ComponentAccumulator()

    log.info("Will configure Run 1 trigger EDM to xAOD conversion")
    from TrigEDMConfig.TriggerEDM import getTriggerEDMList
    from TrigEDMConfig.TriggerEDM import getEFRun1BSList,getEFRun2EquivalentList,getL2Run1BSList,getL2Run2EquivalentList

    from TrkConfig.TrkParticleCreatorConfig import TrackParticleCreatorToolCfg
    partCreatorTool = acc.popToolsAndMerge(TrackParticleCreatorToolCfg(flags,
                                                                       PixelToTPIDTool=None
                                                                       )
                                          )
    acc.addPublicTool(partCreatorTool)

    from xAODTrackingCnv.xAODTrackingCnvConfig import TrackCollectionCnvToolCfg,RecTrackParticleContainerCnvToolCfg
    trackCollCnvTool = acc.popToolsAndMerge(TrackCollectionCnvToolCfg(flags,
                                                                      name="TrackCollectionCnvTool",
                                                                      TrackParticleCreator= partCreatorTool
                                                                      )
                                            )

    recPartCnvTool = acc.popToolsAndMerge(RecTrackParticleContainerCnvToolCfg(flags,
                                                                              name="RecParticleCnv",
                                                                              TrackParticleCreator=partCreatorTool
                                                                              )
                                          )
    
    bstoxaodTool = CompFactory.TrigBStoxAODTool("BStoxAOD", 
                                                ContainersToConvert = getL2Run1BSList() + getEFRun1BSList(), 
                                                NewContainers = getL2Run2EquivalentList() + getEFRun2EquivalentList(),
                                                TrackCollectionCnvTool = trackCollCnvTool,
                                                TrackParticleContainerCnvTool = recPartCnvTool
                                                )
    acc.setPrivateTools(bstoxaodTool)

    # write the xAOD (Run-2) classes to the output
    acc.merge(addToESD(flags, edmDictToList(getTriggerEDMList(flags.Trigger.ESDEDMSet, runVersion=2))))
    acc.merge(addToAOD(flags, edmDictToList(getTriggerEDMList(flags.Trigger.AODEDMSet, runVersion=2))))

    return acc

def Run3TriggerBSUnpackingCfg(flags):
    """Configures conversions BS -> HLTResultMT -> Collections """
    acc = ComponentAccumulator()

    if flags.Trigger.decodeHLT is False:
        log.debug("Run3TriggerBSUnpackingCfg: Nothing to do as Trigger.decodeHLT is False")
        return acc

    from AthenaCommon.CFElements import seqAND
    from TrigEDMConfig.DataScoutingInfo import (
        getDataScoutingStreams, getDataScoutingTypeFromStream,
        getFullHLTResultID, getDataScoutingResultID,
    )
    from TriggerJobOpts.TriggerConfig import triggerEDMGapFillerCfg
    from TrigDecisionTool.TrigDecisionToolConfig import getRun3NavigationContainerFromInput
    ids_to_decode = []

    # Map the trigger stream to the event building type
    if flags.Input.TriggerStream in getDataScoutingStreams():
        dstype = getDataScoutingTypeFromStream(flags.Input.TriggerStream)
        # If writing a Data Scouting EDM format, only decode that ID
        log.info(f'Configuring BS decoding/deserialisation of HLT result for \'{dstype}\' only')
        ids_to_decode = [getDataScoutingResultID(dstype)]
        id_to_deserialise = {dstype: getDataScoutingResultID(dstype)}
    else:
        # Set up to read the full HLT result ID
        ids_to_decode = [getFullHLTResultID()]
        id_to_deserialise = {'': getFullHLTResultID()}

        # In addition, decode any available IDs. We need to check which streams were active in the file.
        log.debug('Configuring BS decoding of all HLT results and deserialisation of full HLT')
        partialHLT_streams = [
            stream for stream in getDataScoutingStreams() if stream.split('_')[1] in flags.Input.ProcessingTags
        ]
        partialHLT_dstypes = [
            getDataScoutingTypeFromStream(stream) for stream in partialHLT_streams
        ]
        partialHLT_dsids = [
            getDataScoutingResultID(dstype) for dstype in partialHLT_dstypes
        ]
        log.debug(f'Configuring BS decoding and deserialisation of all HLT results: full HLT + {partialHLT_dstypes}')

        ids_to_decode += partialHLT_dsids
        # A bit ugly but avoid importing the actual dict to ensure we never modify it
        id_to_deserialise.update({
            dstype:dsid for dstype,dsid in zip(partialHLT_dstypes, partialHLT_dsids)
        })

    decoder = CompFactory.HLTResultMTByteStreamDecoderAlg(ModuleIdsToDecode=ids_to_decode)
    acc.addSequence(seqAND("HLTDecodingSeq"))
    acc.addEventAlgo( decoder, "HLTDecodingSeq")
    for dstype, id in id_to_deserialise.items():
        deserialiser = CompFactory.TriggerEDMDeserialiserAlg(
            f"TrigDeserialiser{dstype}",
            ModuleID=id,
        )
        # Full HLT result also has slimmed navigation summary
        if dstype == '':
            deserialiser.ExtraOutputs.add(('xAOD::TrigCompositeContainer' , 'StoreGateSvc+DummyForGapFiller'))
        else:
            deserialiser.SkipDuplicateRecords = True
            deserialiser.PermitMissingModule = True
        acc.addEventAlgo( deserialiser, "HLTDecodingSeq")

        if dstype=='':
            # Create empty EDM collections for types not created online
            # Add output dependency on Navigation to enforce ordering
            gapFiller = triggerEDMGapFillerCfg(
                flags, edmSet=['BS'],
                extraInputs=[('xAOD::TrigCompositeContainer' , 'StoreGateSvc+DummyForGapFiller')],
                extraOutputs=[(
                    'xAOD::TrigCompositeContainer',
                    'StoreGateSvc+'+getRun3NavigationContainerFromInput(flags)
                )])
        else:
            # Create empty EDM collections for types not created online
            gapFiller = triggerEDMGapFillerCfg(flags, edmSet=[dstype])
        acc.merge( gapFiller, "HLTDecodingSeq" )

    log.debug("Configured HLT result BS decoding sequence")
    return acc


if __name__ == '__main__':
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaConfiguration.AllConfigFlags import initConfigFlags

    flags = initConfigFlags()
    args = flags.fillFromArgs()

    if not args.filesInput:
        from AthenaConfiguration.TestDefaults import defaultTestFiles
        flags.Input.Files = defaultTestFiles.RAW_RUN3 # need to update this depending on EDMversion
        flags.Exec.MaxEvents = 5
        log.info('Checking setup for EDMVersion %d with default input files', flags.Trigger.EDMVersion)
        if flags.Trigger.EDMVersion==1:
            flags.Input.Files = defaultTestFiles.RAW_RUN1
        elif flags.Trigger.EDMVersion==2:
            flags.Input.Files = defaultTestFiles.RAW_RUN2
        elif flags.Trigger.EDMVersion==3:
            flags.Input.Files = defaultTestFiles.RAW_RUN3

    flags.lock()

    acc = MainServicesCfg(flags)
    acc.merge( TriggerRecoCfg(flags) )
    acc.printConfig(withDetails=True)

    import sys
    sys.exit(acc.run().isFailure())
