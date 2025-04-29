#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

from AthenaConfiguration.AthConfigFlags import AthConfigFlags
from AthenaConfiguration.Enums import FlagEnum,HIMode

_steeringFlags = [ 'doGlobalMon', 'doLVL1CaloMon', 'doLVL1InterfacesMon', 'doCTPMon', 'doHLTMon',
                   'doPixelMon', 'doSCTMon', 'doTRTMon', 'doInDetMon',
                   'doLArMon', 'doTileMon',
                   'doCaloGlobalMon', 'doMuonMon',
                   'doLucidMon', 'doAFPMon', 'doZDCMon',
                   'doHIMon', 'doEgammaMon', 'doJetMon', 'doMissingEtMon',
                   'doJetInputsMon',
                   'doTauMon', 'doJetTagMon', 'doDataFlowMon' ]

_lowLevelSteeringFlags = [ 'InDet.doGlobalMon', 'InDet.doAlignMon',
                           'InDet.doPerfMon',  'Muon.doRawMon',
                           'Muon.doTrackMon', 'Muon.doAlignMon',
                           'Muon.doSegmentMon',
                           'Muon.doPhysicsMon', 'Muon.doTrkPhysMon',
                           'Muon.doCombinedMon', 'LVL1Calo.doValidation'
                           ]


class DQDataType(FlagEnum):
    """Flag values for DQ.DataType"""
    Collisions = 'collisions'
    Cosmics    = 'cosmics'
    HeavyIon   = 'heavyioncollisions'
    MC         = 'monteCarlo'
    User       = 'user'


def createDQConfigFlags():
    acf=AthConfigFlags()
    acf.addFlag('DQ.doMonitoring', False)
    acf.addFlag('DQ.doStreamAwareMon', True)
    acf.addFlag('DQ.disableAtlasReadyFilter', False)
    acf.addFlag('DQ.disableFilledBunchFilter', False)
    acf.addFlag('DQ.enableLumiAccess', True)
    acf.addFlag('DQ.FileKey', 'CombinedMonitoring')
    # two flags here, with different meaning.
    # triggerDataAvailable determines whether we expect trigger objects in the event store
    acf.addFlag('DQ.triggerDataAvailable', True)
    # useTrigger determines whether we should use TrigDecisionTool
    acf.addFlag('DQ.useTrigger', getUseTrigger)

    # computed
    acf.addFlag('DQ.Environment', getEnvironment )
    acf.addFlag('DQ.DataType', getDataType, type=DQDataType )

    # for in-Athena histogram postprocessing
    acf.addFlag('DQ.doPostProcessing', False)
    acf.addFlag('DQ.postProcessingInterval', 100)

    # steering ...
    for flag in _steeringFlags + _lowLevelSteeringFlags:
        if flag == 'doLVL1CaloMon':
            continue
        arg = True
        if flag in ['doJetTagMon', 'doMissingEtMon', 'doTauMon']:
            arg = lambda x: x.DQ.DataType is not DQDataType.Cosmics and x.Reco.HIMode is not HIMode.HI # noqa: E731
        if flag in [ 'doJetMon','doJetTagMon'] :
            arg = lambda x: x.DQ.DataType is not DQDataType.Cosmics # noqa: E731
        if flag == 'doHLTMon':
            # new HLT monitoring not yet compatible with pre-Run 3 data
            # disable HLT monitoring if input is data AOD as not all HLT collections in AOD - ATR-28781
            arg = lambda x: x.Trigger.EDMVersion == 3 and x.DQ.Environment != 'AOD' # noqa: E731
        if flag == 'LVL1Calo.doValidation':
            arg = False
        if flag == 'doZDCMon':
            arg= lambda x: (x.Reco.EnableZDC is True) # noqa: E731
        acf.addFlag('DQ.Steering.' + flag, arg)

    # special protection for L1Calo monitoring : check L1Calo is indeed in
    acf.addFlag('DQ.Steering.doLVL1CaloMon', _hasL1Calo)

    # HLT steering ...
    from PyUtils.moduleExists import moduleExists
    if moduleExists ('TrigHLTMonitoring'):
        from TrigHLTMonitoring.TrigHLTMonitorAlgorithm import createHLTDQConfigFlags
        acf.join(createHLTDQConfigFlags())
    return acf

def getUseTrigger(flags):
    from PyUtils.moduleExists import moduleExists
    hlt_exists = moduleExists ('TrigHLTMonitoring')
    return hlt_exists and flags.DQ.triggerDataAvailable

def getDataType(flags):
    from AthenaConfiguration.Enums import BeamType
    if flags.Input.isMC:
        return DQDataType.MC
    elif flags.Reco.EnableHI:
        return DQDataType.HeavyIon
    elif flags.Beam.Type is BeamType.Cosmics:
        return DQDataType.Cosmics
    elif flags.Beam.Type is BeamType.Collisions:
        return DQDataType.Collisions
    elif flags.Beam.Type is BeamType.SingleBeam:
        # historically, singlebeam treated as collisions
        return DQDataType.Collisions
    else:
        from AthenaCommon.Logging import logging
        local_logger = logging.getLogger('DQConfigFlags_getDataType')
        local_logger.warning('Unable to figure out beam type for DQ; using "User"')
        return DQDataType.User

def getEnvironment(flags):
    if flags.Common.isOnline:
        return 'online'
    else:
        # this could use being rethought to properly encode input and output types perhaps ...
        from AthenaConfiguration.Enums import Format
        if flags.Input.Format is Format.BS:
            if flags.Output.AODFileName:
                return 'tier0'
            else:
                return 'tier0Raw'
        elif 'StreamESD' in flags.Input.ProcessingTags:
            return 'tier0ESD'
        elif 'StreamAOD' in flags.Input.ProcessingTags:
            return 'AOD'
        elif 'StreamDAOD_PHYS' in flags.Input.ProcessingTags:
            return 'DAOD_PHYS'
        else:
            from AthenaCommon.Logging import logging
            local_logger = logging.getLogger('DQConfigFlags_getEnvironment')
            local_logger.warning('Unable to figure out environment for DQ; using "tier0ESD"')
            return 'tier0ESD'

def _hasL1Calo(flags):
    from AthenaConfiguration.AutoConfigFlags import GetFileMD
    md = GetFileMD(flags.Input.Files)
    import eformat
    detMask = eformat.helper.DetectorMask(f'{md.get("detectorMask",[0x0])[0]:032x}')
    isEnabled = any([
        detMask.is_set(eformat.helper.SubDetector.TDAQ_CALO_PREPROC),
        detMask.is_set(eformat.helper.SubDetector.TDAQ_CALO_TOPO_PROC),
        detMask.is_set(eformat.helper.SubDetector.TDAQ_CALO_FEAT_EXTRACT_DAQ),
        detMask.is_set(eformat.helper.SubDetector.TDAQ_CALO_FEAT_EXTRACT_ROI)])
    return isEnabled

def allSteeringFlagsOff(flags):
    for flag in _steeringFlags:
        setattr(getattr(flags, 'DQ.Steering'), flag, False)
