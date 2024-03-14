#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
import functools
from TrigEDMConfig import DataScoutingInfo
from TrigEDMConfig.TriggerEDM import recordable
from TriggerMenuMT.HLT.Menu import EventBuildingInfo
from TriggerMenuMT.HLT.Config.MenuComponents import ChainStep, MenuSequenceCA, SelectionCA, InEventRecoCA
from TrigPartialEventBuilding.TrigPartialEventBuildingConfig import StaticPEBInfoWriterToolCfg, RoIPEBInfoWriterToolCfg
from HLTSeeding.HLTSeedingConfig import mapThresholdToL1DecisionCollection
from libpyeformat_helper import SourceIdentifier, SubDetector
from AthenaConfiguration.ComponentFactory import CompFactory
from .LATOMESourceIDs import LATOMESourceIDs
from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)


def addEventBuildingSequence(flags, chain, eventBuildType, chainDict):
    '''
    Add an extra ChainStep to a Chain object with a PEBInfoWriter sequence configured for the eventBuildType
    '''
    if not eventBuildType:
        log.error('No eventBuildType specified')
        return
    if eventBuildType not in EventBuildingInfo.getAllEventBuildingIdentifiers():
        log.error('eventBuildType \'%s\' not found in the allowed Event Building identifiers', eventBuildType)
        return

    seq = functools.partial(pebMenuSequenceCfg, flags, chain=chain, eventBuildType=eventBuildType, chainDict=chainDict)

    if len(chain.steps)==0:
        # noalg PEB chain
        step_name = 'Step_PEBInfoWriter_{:s}'.format( eventBuildType)
        step = ChainStep(name=step_name,
                         Sequences=[seq],
                         chainDicts=[chainDict])
    else:
        # standard PEB chain
        prevStep = chain.steps[-1]        
        step_name = 'Step_merged{:s}_PEBInfoWriter_{:s}'.format(prevStep.name, eventBuildType)
        step = ChainStep(name=step_name,
                         Sequences=[seq for leg in prevStep.legIds],
                         multiplicity=prevStep.multiplicity,
                         chainDicts=prevStep.stepDicts)

    chain.steps.append(step)

def pebInfoWriterToolCfg(flags, name, eventBuildType):
    """Create PEBInfoWriterTool configuration for the eventBuildType"""

    # Main HLTResult ROB:
    HLT_ROB = SourceIdentifier(SubDetector.TDAQ_HLT, DataScoutingInfo.getFullHLTResultID())

    acc = None
    if 'BeamSpotPEB' == eventBuildType:
        acc = StaticPEBInfoWriterToolCfg(
            flags, name,
            subDets = [SubDetector.PIXEL_BARREL,
                       SubDetector.PIXEL_DISK_SIDE, # note different name in C++, ADHI-4753
                       SubDetector.PIXEL_B_LAYER,
                       SubDetector.PIXEL_IBL,
                       SubDetector.SCT_BARREL_A_SIDE,
                       SubDetector.SCT_BARREL_C_SIDE,
                       SubDetector.SCT_ENDCAP_A_SIDE,
                       SubDetector.SCT_ENDCAP_C_SIDE,
                       SubDetector.TDAQ_CTP] )

    elif 'MuonTrkPEB' == eventBuildType:
        acc = RoIPEBInfoWriterToolCfg(
            flags, name,
            regSelDets = ['Pixel', 'SCT', 'TRT', 'MDT', 'RPC', 'TGC', 'CSC', 'MM', 'sTGC'],
            MaxRoIs = 99,
            EtaWidth = 0.75, # values from run2 check later
            PhiWidth = 0.75, # values from run2 check later
            subDets = [SubDetector.TDAQ_CTP], # add full CTP data to the output
            ROBs = [HLT_ROB] )

    elif 'IDCalibPEB' == eventBuildType:
        acc = RoIPEBInfoWriterToolCfg(
            flags, name,
            regSelDets = ['Pixel', 'SCT', 'TRT'],
            EtaWidth = 0.1,
            PhiWidth = 0.1,
            subDets = [SubDetector.TDAQ_CTP] )

    elif 'LArPEBCalib' == eventBuildType:
        acc = StaticPEBInfoWriterToolCfg(
            flags, name,
            subDets = [SubDetector.LAR_EM_BARREL_A_SIDE,
                       SubDetector.LAR_EM_BARREL_C_SIDE,
                       SubDetector.LAR_EM_ENDCAP_A_SIDE,
                       SubDetector.LAR_EM_ENDCAP_C_SIDE,
                       SubDetector.LAR_HAD_ENDCAP_A_SIDE,
                       SubDetector.LAR_HAD_ENDCAP_C_SIDE,
                       SubDetector.LAR_FCAL_A_SIDE,
                       SubDetector.LAR_FCAL_C_SIDE,
                       SubDetector.LAR_EM_BARREL_ENDCAP_A_SIDE,
                       SubDetector.LAR_EM_BARREL_ENDCAP_C_SIDE,
                       SubDetector.LAR_EM_HAD_ENDCAP_A_SIDE,
                       SubDetector.LAR_EM_HAD_ENDCAP_C_SIDE,
                       SubDetector.TDAQ_CTP] )

    elif eventBuildType in ('LArPEBHLT', 'LArPEB'):
        acc = RoIPEBInfoWriterToolCfg(
            flags, name,
            regSelDets = ['Pixel', 'SCT', 'TRT', 'TTEM', 'TTHEC', 'FCALEM', 'FCALHAD'],
            MaxRoIs = 5,
            ROBs = [HLT_ROB] + LATOMESourceIDs, # add full-scan LATOME data
            subDets = [SubDetector.TDAQ_CTP] )

    elif 'LATOMEPEB' == eventBuildType:
        acc = StaticPEBInfoWriterToolCfg(
            flags, name,
            ROBs = LATOMESourceIDs, # add full-scan LATOME data
            subDets = [SubDetector.TDAQ_CTP] )

    elif 'SCTPEB' == eventBuildType:
        acc = StaticPEBInfoWriterToolCfg(
            flags, name,
            subDets = [SubDetector.SCT_BARREL_A_SIDE,
                       SubDetector.SCT_BARREL_C_SIDE,
                       SubDetector.SCT_ENDCAP_A_SIDE,
                       SubDetector.SCT_ENDCAP_C_SIDE,
                       SubDetector.TDAQ_CTP] )

    elif 'TilePEB' == eventBuildType:
        acc = StaticPEBInfoWriterToolCfg(
            flags, name,
            subDets = [SubDetector.TILECAL_LASER_CRATE,
                       SubDetector.TILECAL_BARREL_A_SIDE,
                       SubDetector.TILECAL_BARREL_C_SIDE,
                       SubDetector.TILECAL_EXT_A_SIDE,
                       SubDetector.TILECAL_EXT_C_SIDE,
                       SubDetector.TDAQ_CTP,
                       SubDetector.TDAQ_CALO_PREPROC, # = 0x71
                       SubDetector.TDAQ_CALO_CLUSTER_PROC_DAQ, # = 0x72
                       SubDetector.TDAQ_CALO_CLUSTER_PROC_ROI, # = 0x73
                       SubDetector.TDAQ_CALO_JET_PROC_DAQ, # = 0x74
                       SubDetector.TDAQ_CALO_JET_PROC_ROI # = 0x75
                       ] )

    elif 'LArPEBNoise' == eventBuildType:
        acc = RoIPEBInfoWriterToolCfg(
            flags, name,
            regSelDets = ['Pixel', 'SCT', 'TRT', 'TTEM', 'TTHEC', 'FCALEM', 'FCALHAD'],
            MaxRoIs = 5,
            ROBs = [HLT_ROB] + LATOMESourceIDs,
            subDets = [SubDetector.MUON_MMEGA_ENDCAP_A_SIDE,
                       SubDetector.MUON_MMEGA_ENDCAP_C_SIDE,
                       SubDetector.MUON_STGC_ENDCAP_A_SIDE,
                       SubDetector.MUON_STGC_ENDCAP_C_SIDE,
                       SubDetector.TDAQ_CTP] )

    elif 'ZDCPEB' == eventBuildType:
        acc = StaticPEBInfoWriterToolCfg(
            flags, name,
            subDets = [SubDetector.FORWARD_ZDC,
                       SubDetector.TDAQ_CTP] )

    elif 'AFPPEB' == eventBuildType:
        acc = StaticPEBInfoWriterToolCfg(
            flags, name,
            subDets = [SubDetector.FORWARD_AFP,
                       SubDetector.TDAQ_CTP] )

    elif 'LumiPEB' == eventBuildType:
        acc = StaticPEBInfoWriterToolCfg(
            flags, name,
            ROBs = [HLT_ROB],
            subDets = [SubDetector.PIXEL_IBL,
                       SubDetector.PIXEL_BARREL,
                       SubDetector.PIXEL_DISK_SIDE,
                       SubDetector.PIXEL_B_LAYER,
                       SubDetector.SCT_BARREL_A_SIDE,
                       SubDetector.SCT_BARREL_C_SIDE,
                       SubDetector.SCT_ENDCAP_A_SIDE,
                       SubDetector.SCT_ENDCAP_C_SIDE,
                       SubDetector.PIXEL_DBM,
                       SubDetector.TDAQ_CTP] )

    elif 'Lvl1CaloPEB' == eventBuildType:
        acc = StaticPEBInfoWriterToolCfg(
            flags, name,
            ROBs = [HLT_ROB],
            MaxRoIs = 1,
            subDets = [SubDetector.TDAQ_CALO_PREPROC,
                       SubDetector.TDAQ_CALO_CLUSTER_PROC_DAQ,
                       SubDetector.TDAQ_CALO_CLUSTER_PROC_ROI,
                       SubDetector.TDAQ_CALO_JET_PROC_DAQ,
                       SubDetector.TDAQ_CALO_JET_PROC_ROI,
                       SubDetector.TDAQ_CTP] )

    elif 'DarkJetPEBTLA' == eventBuildType:
        acc = RoIPEBInfoWriterToolCfg(
            flags, name,
            # Add subdetectors within a ROI for PEB
            regSelDets = ['Pixel', 'SCT', 'TRT', 'TTEM', 'TTHEC', 'FCALEM', 'FCALHAD', 'TILE'],
            # DS HLT result
            ROBs = [SourceIdentifier(SubDetector.TDAQ_HLT,
                                     DataScoutingInfo.getDataScoutingResultID(eventBuildType))],
            EtaWidth = 0.6, # half-width (the RoI is between etaJet-EtaWidth and etaJet+EtaWidth)
            PhiWidth = 0.6,
            MaxRoIs = 3 )

    elif 'EgammaPEBTLA' == eventBuildType:
        acc = RoIPEBInfoWriterToolCfg(
            flags, name,
            # Add subdetectors within a ROI for PEB
            regSelDets = ['Pixel', 'SCT', 'TRT', 'TTEM', 'TTHEC', 'FCALEM', 'FCALHAD', 'TILE'],
            # DS HLT result
            ROBs = [SourceIdentifier(SubDetector.TDAQ_HLT,
                                     DataScoutingInfo.getDataScoutingResultID(eventBuildType))],
            EtaWidth = 0.4, # half-width (the RoI is between etaJet-EtaWidth and etaJet+EtaWidth)
            PhiWidth = 0.4,
            MaxRoIs = 6 )
    elif 'FTagPEBTLA' == eventBuildType:
        acc = RoIPEBInfoWriterToolCfg(
            flags, name,
            # Add subdetectors within a ROI for PEB
            regSelDets = ['Pixel', 'SCT'],
            # DS HLT result
            ROBs = [SourceIdentifier(SubDetector.TDAQ_HLT,
                                     DataScoutingInfo.getDataScoutingResultID(eventBuildType))],
            EtaWidth = 0.4, # half-width (the RoI is between etaJet-EtaWidth and etaJet+EtaWidth)
            PhiWidth = 0.4,
            MaxRoIs = 6 )

    elif eventBuildType in DataScoutingInfo.getAllDataScoutingIdentifiers():
        # Pure DataScouting configuration
        acc = StaticPEBInfoWriterToolCfg(
            flags, name,
            ROBs = [SourceIdentifier(SubDetector.TDAQ_HLT,
                                     DataScoutingInfo.getDataScoutingResultID(eventBuildType))] )

    # Name not matched
    if acc is None:
        log.error('PEBInfoWriterTool configuration is missing for event building identifier \'%s\'', eventBuildType)

    return acc

def getPEBBuildSuffix(chain, eventBuildType):
    '''
    Define suffix for unique configurations - prevents config clashes.
    '''

    suffix=''

    _isFullscan = isFullScan(chain)
    _isRoIBasedPEB = EventBuildingInfo.isRoIBasedPEB(eventBuildType)
    _isNoalg = isNoAlg(chain)

    if _isNoalg or not _isRoIBasedPEB: suffix+='_noSeed'
    if _isFullscan and _isRoIBasedPEB: suffix+='_RoIBasedFS'

    return suffix

def pebInputMaker(flags, chain, eventBuildType):

    suffix= getPEBBuildSuffix(chain, eventBuildType)

    # Check if we are configuring a chain with at least one full-scan leg
    isFullscan = isFullScan(chain)
    # Check if we are configuring RoI-based PEB
    _isRoIBasedPEB = EventBuildingInfo.isRoIBasedPEB(eventBuildType)
    _isNoalg = isNoAlg(chain)

    # Configure the InputMaker
    maker = CompFactory.InputMakerForRoI("IMpeb_" + eventBuildType + suffix)
    maker.RoIs = "pebInputRoI_" + eventBuildType + suffix
    # Allow more than one feature per input RoI if we care about RoIs, and have at least one Step
    maker.mergeUsingFeature = _isRoIBasedPEB and not _isNoalg

    # Configure the InputMaker RoI tool
    if _isNoalg or not _isRoIBasedPEB:
        # Streamers or static PEB: use initial RoI
        maker.RoITool = CompFactory.ViewCreatorInitialROITool()
    elif isFullscan and _isRoIBasedPEB:
        # Full-scan chains with RoI-based PEB: create RoI around feature IParticle
        maker.RoITool = CompFactory.ViewCreatorCentredOnIParticleROITool()
        maker.RoITool.RoisWriteHandleKey = recordable("HLT_Roi_" + eventBuildType)
    else:
        # Other chains: use previous RoI
        maker.RoITool = CompFactory.ViewCreatorPreviousROITool()

    return maker


def pebMenuSequenceCfg(flags, chain, eventBuildType, chainDict):
    '''
    Return the MenuSequenceCA for the PEB input maker for this chain.
    '''

    def pebInfoWriterToolGenerator(chainDict):
        return pebInfoWriterToolCfg(flags, chainDict['chainName'], eventBuildType)

    suffix = getPEBBuildSuffix(chain, eventBuildType)

    inputMaker = pebInputMaker(flags, chain, eventBuildType)
    recoAcc = InEventRecoCA("pebSequence_"+eventBuildType, inputMaker=inputMaker)
    selAcc = SelectionCA("pebMainSeq_"+eventBuildType+suffix)
    selAcc.mergeReco(recoAcc)
    selAcc.addHypoAlgo(CompFactory.PEBInfoWriterAlg('PEBInfoWriterAlg_' + eventBuildType+suffix))

    return MenuSequenceCA(flags,
                          selAcc,
                          HypoToolGen = pebInfoWriterToolGenerator)


def findEventBuildingStep(chainConfig):
    pebSteps = [s for s in chainConfig.steps if 'PEBInfoWriter' in s.name and 'EmptyPEBAlign' not in s.name]
    if len(pebSteps) == 0:
        return None
    elif len(pebSteps) > 1:
        raise RuntimeError('Multiple Event Building steps in one chain are not supported but found in chain ' + chainConfig.name)
    return pebSteps[0]


def alignEventBuildingSteps(chain_configs, chain_dicts):
    def is_peb_dict(chainNameAndDict):
        return len(chainNameAndDict[1]['eventBuildType']) > 0

    all_peb_chain_dicts = dict(filter(is_peb_dict, chain_dicts.items()))
    all_peb_chain_names = list(all_peb_chain_dicts.keys())

    def is_peb_config(chainNameAndConfig):
        return chainNameAndConfig[0] in all_peb_chain_names

    all_peb_chain_configs = dict(filter(is_peb_config, chain_configs.items()))

    maxPebStepPosition = {} # {eventBuildType: N}

    def getPebStepPosition(chainConfig):
        pebStep = findEventBuildingStep(chainConfig)
        return chainConfig.steps.index(pebStep) + 1

    # First loop to find the maximal PEB step positions to which we need to align
    for chainName, chainConfig in all_peb_chain_configs.items():        
        pebStepPosition = getPebStepPosition(chainConfig)
        ebt = all_peb_chain_dicts[chainName]['eventBuildType']
        if ebt not in maxPebStepPosition or pebStepPosition > maxPebStepPosition[ebt]:
            maxPebStepPosition[ebt] = pebStepPosition

    # Second loop to insert empty steps before the PEB steps where needed
    for chainName, chainConfig in all_peb_chain_configs.items():
        pebStepPosition = getPebStepPosition(chainConfig)
        ebt = all_peb_chain_dicts[chainName]['eventBuildType']
        if pebStepPosition < maxPebStepPosition[ebt]:
            numStepsNeeded = maxPebStepPosition[ebt] - pebStepPosition
            log.debug('Aligning PEB step for chain %s by adding %d empty steps', chainName, numStepsNeeded)
            chainConfig.insertEmptySteps('EmptyPEBAlign', numStepsNeeded, pebStepPosition-1)
            chainConfig.numberAllSteps()


def isFullScan(chain):
    '''Helper function to determine if chain is full scan'''
    # Check if we are configuring a chain with at least one full-scan leg
    return (mapThresholdToL1DecisionCollection('FSNOSEED') in chain.L1decisions)


def isNoAlg(chain):
    '''Helper function to determine if chain has HLT reco'''
    return (len(chain.steps) == 0)


# Unit test
if __name__ == "__main__":
    from AthenaConfiguration.TestDefaults import defaultTestFiles, defaultGeometryTags
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.RAW_RUN3
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN3
    flags.lock()

    # Ensure all DS identifiers have been added to the EB list:
    assert( set(DataScoutingInfo.getAllDataScoutingIdentifiers()).issubset(
        EventBuildingInfo.getAllEventBuildingIdentifiers()) )

    failures = 0
    for eb_identifier in EventBuildingInfo.getAllEventBuildingIdentifiers():
        log.info('Checking %s event building identifier', eb_identifier)
        tool = None
        try:
            cfg = pebInfoWriterToolCfg(flags, 'TestTool_'+eb_identifier, eb_identifier)
            tool = cfg.popPrivateTools()
        except Exception as ex:
            failures += 1
            log.error('Caught exception while configuring PEBInfoWriterTool for %s: %s', eb_identifier, ex)
            continue

        cfg.wasMerged()

        if not tool:
            failures += 1
            log.error('No tool created for %s', eb_identifier)
            continue

        if not isinstance(tool, (CompFactory.StaticPEBInfoWriterTool, CompFactory.RoIPEBInfoWriterTool)):
            failures += 1
            log.error('Unexpected tool type for %s: %s', eb_identifier, tool.getType())
            continue

        isRoIBasedPEB = EventBuildingInfo.isRoIBasedPEB(eb_identifier)
        if isinstance(tool, CompFactory.RoIPEBInfoWriterTool) != isRoIBasedPEB:
            failures += 1
            log.error('Tool type %s for %s but isRoIBasedPEB==%s',
                      tool.getType(), eb_identifier, isRoIBasedPEB)
            continue

        robs = tool.ROBList if tool.getType() == 'StaticPEBInfoWriterTool' else tool.ExtraROBs
        dets = tool.SubDetList if tool.getType() == 'StaticPEBInfoWriterTool' else tool.ExtraSubDets
        robs_check_passed = True
        is_data_scouting = False
        for rob_id in robs:
            rob_sid = SourceIdentifier(rob_id)
            rob_det_id = rob_sid.subdetector_id()
            if rob_det_id == SubDetector.TDAQ_HLT and rob_sid.module_id() != DataScoutingInfo.getFullHLTResultID():
                is_data_scouting = True
            if int(rob_det_id) in dets:
                robs_check_passed = False
                log.error('Redundant configuration for %s: ROB %s added to the ROB list while full SubDetector '
                          '%s is already in the SubDets list', eb_identifier, rob_sid.human(), str(rob_det_id))

        if not robs_check_passed:
            failures += 1
            continue

        # Check for always-present fragment to avoid PEB becoming FEB (ATR-24378)
        # DataScouting is exempt as it always comes with a dedicated HLT result
        always_present_rob = SourceIdentifier(SubDetector.TDAQ_CTP, 0)  # cannot run HLT without CTP so it is always present
        if not is_data_scouting and \
                always_present_rob.code() not in robs and \
                always_present_rob.subdetector_id() not in dets:
            log.error('Bug-prone configuration for %s: without always-present CTP data in the PEB list, the '
                      'streaming may break when all requested detectors are disabled. Add CTP data to this PEB '
                      'configuration to prevent the bug (ATR-24378).', eb_identifier)
            failures += 1
            continue

        log.info('%s correctly configured', tool.name() if callable(tool.name) else tool.name)

    import sys
    sys.exit(failures)
