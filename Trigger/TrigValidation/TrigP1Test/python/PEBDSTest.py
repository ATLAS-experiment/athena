#
#  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#

'''
This test defines its own version of the Dev_pp_run3_v1 menu and the corresponding PEB/DS configuration,
and executes several chains testing various types of Partial Event Building and Data Scouting
'''

from TrigEDMConfig import DataScoutingInfo
from TrigEDMConfig.TriggerEDM import AllowedOutputFormats
from TriggerMenuMT.HLT.Menu import Dev_pp_run3_v1, EventBuildingInfo, StreamInfo
from TriggerMenuMT.HLT.Config.Utility.ChainDefInMenu import ChainProp
from TriggerMenuMT.HLT.CommonSequences import EventBuildingSequences
from TrigPartialEventBuilding.TrigPartialEventBuildingConfig import StaticPEBInfoWriterToolCfg, RoIPEBInfoWriterToolCfg
from libpyeformat_helper import SubDetector, SourceIdentifier
from AthenaCommon.Logging import logging
log = logging.getLogger('dataScoutingTest')

# Add new allowed event building identifiers                              : isRoIBasedPEB
EventBuildingInfo._PartialEventBuildingIdentifiers |= {'TestPEBOne'       : False,
                                                       'TestPEBTwo'       : False,
                                                       'TestPEBThree'     : True,
                                                       'TestPEBFour'      : True,
                                                       'ElectronDSTest'   : False,
                                                       'ElectronDSPEBTest': False}
# Modify these private maps only for the purposes of this test
DataScoutingInfo._DataScoutingIdentifiers['ElectronDSTest'] = 3
DataScoutingInfo._DataScoutingIdentifiers['ElectronDSPEBTest'] = 3
DataScoutingInfo.TruncationThresholds[3] = 5*(1024**2) # 5 MB

# Hack/update AllowedOutputFormats for this test
AllowedOutputFormats.extend(DataScoutingInfo.getAllDataScoutingIdentifiers())

# Override the setupMenu function from Dev_pp_run3_v1
def myMenu():
    log.debug('Executing myMenu')

    from TriggerMenuMT.HLT.Menu.SignatureDicts import ChainStore
    chains = ChainStore()
    chains['Egamma'] = [
        # DS+PEB chain (special HLT result and subset of detector data saved)
        ChainProp(name='HLT_e3_etcut_ElectronDSPEBTest_L1eEM5', stream=['ElectronDSPEBTest'], groups=['RATE:Test','BW:Other']),

        # Pure DS chain (only special HLT result saved and no detector data saved)
        ChainProp(name='HLT_e5_etcut_ElectronDSTest_L1eEM5', stream=['ElectronDSTest'], groups=['RATE:Test','BW:Other']),

        # PEB chain (full HLT result and fixed subset of detector data saved)
        ChainProp(name='HLT_e7_etcut_TestPEBOne_L1eEM5', stream=['TestPEBOne'], groups=['RATE:Test','BW:Other']),

        # PEB chain (full HLT result and RoI-based subset of detector data saved)
        ChainProp(name='HLT_e10_etcut_TestPEBThree_L1eEM5', stream=['TestPEBThree'], groups=['RATE:Test','BW:Other']),

        # Standard chain (full HLT result and full detector data saved)
        ChainProp(name='HLT_e12_etcut_L1eEM5', stream=['Main'], groups=['RATE:SingleElectron', 'BW:Electron']),
    ]

    chains['Muon'] = [
        # PEB chain (fixed subset of detector data saved and no HLT result)
        ChainProp(name='HLT_mu6_TestPEBTwo_L1MU5VF', stream=['TestPEBTwo'], groups=['RATE:Test','BW:Other']),

        # PEB chain (RoI-based subset of detector data saved and no HLT result)
        ChainProp(name='HLT_mu6_TestPEBFour_L1MU5VF', stream=['TestPEBFour'], groups=['RATE:Test','BW:Other']),

        # Standard chain (full HLT result and full detector data saved)
        ChainProp(name='HLT_2mu6_L12MU5VF', stream=['Main'], groups=['RATE:SingleMuon', 'BW:Muon']),
    ]
    return chains

Dev_pp_run3_v1.setupMenu = myMenu

# Override the pebInfoWriterTool function from EventBuildingSequences
def myPebInfoWriterToolCfg(flags, name, eventBuildType):
    log.debug('Executing myPebInfoWriterToolCfg')

    # Main CTP and HLT ROB:
    HLT_ROB = SourceIdentifier(SubDetector.TDAQ_HLT, DataScoutingInfo.getFullHLTResultID())

    acc = None
    if 'TestPEBOne' == eventBuildType:
        # TestPEBOne is an example which saves a few detector ROBs
        # and the full HLT result (typically saved in physics streams)
        acc = StaticPEBInfoWriterToolCfg(
            flags, name,
            ROBs = [0x42002e, 0x420060, 0x420064, # a few example LAr ROBs
                    HLT_ROB] )

    elif 'TestPEBTwo' == eventBuildType:
        # TestPEBTwo is an example which saves some detector data,
        # but no HLT result (not needed in detector calibration streams)
        acc = StaticPEBInfoWriterToolCfg(
            flags, name,
            subDets = [SubDetector.MUON_RPC_BARREL_A_SIDE,
                       SubDetector.MUON_RPC_BARREL_C_SIDE] ) # example: RPC side A and C

    elif 'TestPEBThree' == eventBuildType:
        # TestPEBThree is an example using RoIPEBInfoWriterTool which writes
        # all ROBs within a given RoI, and also the main (full) HLT result
        acc = RoIPEBInfoWriterToolCfg(
            flags, name,
            EtaEdge = 5.0,
            EtaWidth = 0.1,
            PhiWidth = 0.1,
            MaxRoIs = 3,
            regSelDets = ['All'],
            ROBs = [HLT_ROB] )

    elif 'TestPEBFour' == eventBuildType:
        # TestPEBFour is similar to TestPEBThree, but saves only muon detector
        # ROBs within a larger RoI and no HLT result
        acc = RoIPEBInfoWriterToolCfg(
            flags, name,
            EtaWidth = 0.5,
            PhiWidth = 0.5,
            regSelDets = ['MDT', 'CSC', 'RPC', 'TGC', 'MM', 'sTGC']) # all muon detectors

    elif 'ElectronDSTest' == eventBuildType:
        # ElectronDSTest is an example of pure Data Scouting,
        # where only the special HLT result is saved and nothing else
        acc = StaticPEBInfoWriterToolCfg(
            flags, name,
            ROBs = [SourceIdentifier(SubDetector.TDAQ_HLT,
                                     DataScoutingInfo.getDataScoutingResultID(eventBuildType))])

    elif 'ElectronDSPEBTest' == eventBuildType:
        # ElectronDSPEBTest is an example of Data Scouting with PEB,
        # where a special HLT result and some detector data are saved
        # (ID + LAr data within an RoI)
        acc = RoIPEBInfoWriterToolCfg(
            flags, name,
            ROBs = [SourceIdentifier(SubDetector.TDAQ_HLT,
                                     DataScoutingInfo.getDataScoutingResultID(eventBuildType))],
            EtaWidth = 0.3,
            PhiWidth = 0.3,
            regSelDets = ['Pixel', 'SCT', 'TRT', 'TTEM', 'TTHEC', 'FCALEM', 'FCALHAD'])

    # Name not matched
    if acc is None:
        log.error('PEBInfoWriterTool configuration is missing for event building identifier \'%s\'', eventBuildType)

    return acc

EventBuildingSequences.pebInfoWriterToolCfg = myPebInfoWriterToolCfg

# Define streams and override StreamInfo
myAllStreams = [
    # [name, type, obeysLumiBlock, forceFullEventBuilding]
    StreamInfo.StreamInfo('Main',               'physics',      True, True),
    StreamInfo.StreamInfo('TestPEBOne',         'physics',      True, False),
    StreamInfo.StreamInfo('TestPEBTwo',         'calibration',  True, False),
    StreamInfo.StreamInfo('TestPEBThree',       'physics',      True, False),
    StreamInfo.StreamInfo('TestPEBFour',        'calibration',  True, False),
    StreamInfo.StreamInfo('ElectronDSTest',     'physics',      True, False),
    StreamInfo.StreamInfo('ElectronDSPEBTest',  'physics',      True, False),
]

StreamInfo._all_streams = myAllStreams

def run(flags):
    from TriggerJobOpts import runHLT
    # Set and customize default flags
    runHLT.set_flags(flags)
    flags.Trigger.triggerMenuSetup = 'Dev_pp_run3_v1'
    flags.Trigger.enabledSignatures = ['Egamma','Muon']
    flags.Trigger.doLVL1 = True
    # Add ElectronDSTest ElectronDSPEBTest to a number of existing EDM entries in the Run 3 electron EDM 
    flags.Trigger.ExtraEDMList = [
            ('xAOD::TrigElectronContainer#HLT_FastElectrons',                      'BS ESD AODFULL ElectronDSTest ElectronDSPEBTest', 'Egamma'),
            ('xAOD::TrigElectronAuxContainer#HLT_FastElectronsAux.',               'BS ESD AODFULL ElectronDSTest ElectronDSPEBTest', 'Egamma'),
            ('xAOD::TrackParticleContainer#HLT_IDTrack_Electron_FTF',              'BS ESD AODFULL ElectronDSTest ElectronDSPEBTest', 'Egamma'),
            ('xAOD::TrackParticleAuxContainer#HLT_IDTrack_Electron_FTFAux.',       'BS ESD AODFULL ElectronDSTest ElectronDSPEBTest', 'Egamma'),
            ('xAOD::TrackParticleContainer#HLT_IDTrack_Electron_IDTrig',           'BS ESD AODFULL ElectronDSTest ElectronDSPEBTest', 'Egamma'),
            ('xAOD::TrackParticleAuxContainer#HLT_IDTrack_Electron_IDTrigAux.',    'BS ESD AODFULL ElectronDSTest ElectronDSPEBTest', 'Egamma'),
            ('xAOD::TrackParticleContainer#HLT_IDTrack_Electron_GSF',              'BS ESD AODFULL ElectronDSTest ElectronDSPEBTest', 'Egamma'),
            ('xAOD::TrackParticleAuxContainer#HLT_IDTrack_Electron_GSFAux.',       'BS ESD AODFULL ElectronDSTest ElectronDSPEBTest', 'Egamma'),
            ('xAOD::CaloClusterContainer#HLT_CaloEMClusters_Electron',             'BS ESD AODFULL ElectronDSTest ElectronDSPEBTest', 'Egamma'),
            ('xAOD::CaloClusterTrigAuxContainer#HLT_CaloEMClusters_ElectronAux.',  'BS ESD AODFULL ElectronDSTest ElectronDSPEBTest', 'Egamma'),
            ('xAOD::ElectronContainer#HLT_egamma_Electrons',                       'BS ESD AODFULL ElectronDSTest ElectronDSPEBTest', 'Egamma'),
            ('xAOD::ElectronAuxContainer#HLT_egamma_ElectronsAux.',                'BS ESD AODFULL ElectronDSTest ElectronDSPEBTest', 'Egamma'),
            ('xAOD::ElectronContainer#HLT_egamma_Electrons_GSF',                   'BS ESD AODFULL ElectronDSTest ElectronDSPEBTest', 'Egamma'),
            ('xAOD::ElectronAuxContainer#HLT_egamma_Electrons_GSFAux.',            'BS ESD AODFULL ElectronDSTest ElectronDSPEBTest', 'Egamma'),
        ]
    flags.lock()
    acc = runHLT.runHLTCfg(flags)
    return acc
