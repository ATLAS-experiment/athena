#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

'''@file TrigHLTMonitorAlgorithm.py
@date 2019-09-10
@date 2020-09-18
@date 2022-02-21
@date 2023-03-17
@brief TrigHLTMonitoring top-level files
'''

from AthenaConfiguration.Enums import HIMode
import logging
logger = logging.getLogger('TrigHLTMonitoring')

RAWonlySignatureList = ['Calo']

def createHLTDQConfigFlags():
    from AthenaConfiguration.AthConfigFlags import AthConfigFlags
    acf=AthConfigFlags()

    from AthenaConfiguration.Enums import Format
    from AthenaConfiguration.Enums import BeamType

    acf.addFlag('DQ.Steering.HLT.doGeneral', True)
    acf.addFlag('DQ.Steering.HLT.doBjet', lambda flags: flags.Beam.Type is BeamType.Collisions) # b-jets disabled for cosmics following ATR-25036
    acf.addFlag('DQ.Steering.HLT.doBphys', True) 
    acf.addFlag('DQ.Steering.HLT.doCalo', True) 
    acf.addFlag('DQ.Steering.HLT.doEgamma', True) 
    acf.addFlag('DQ.Steering.HLT.doInDet', True)
    acf.addFlag('DQ.Steering.HLT.doJet', lambda flags: flags.Input.Format is Format.POOL or (flags.Input.Format is Format.BS and flags.Beam.Type is BeamType.Collisions))
    acf.addFlag('DQ.Steering.HLT.doMET', lambda flags: flags.Reco.HIMode is not HIMode.HI) 
    acf.addFlag('DQ.Steering.HLT.doMinBias', True) 
    acf.addFlag('DQ.Steering.HLT.doMuon', True) 
    acf.addFlag('DQ.Steering.HLT.doTau', lambda flags: flags.Beam.Type is BeamType.Collisions and (flags.Reco.HIMode is not HIMode.HI)) 

    return acf

def verifySignatureDQSettings(inputFlags):
    if inputFlags.DQ.Environment == 'AOD' and not inputFlags.Input.isMC:
        for signature in RAWonlySignatureList:
            sigFlag = f"do{signature}"
            if getattr(inputFlags.DQ.Steering.HLT, sigFlag):
                logger.error(f"HLT monitoring on data AOD input for {signature} not allowed - not all HLT collections are recorded to AOD! Try set DQ.Steering.HLT.{sigFlag}=False.")
                raise ValueError("HLT monitoring enabled on data AOD input.")

def TrigHLTMonTopConfig(inputFlags):
    '''Configuring the HLT signatures top-level steering in the DQ monitoring system.'''

    ########
    #HLT top-level steering

    # Define one top-level monitoring algorithm. The new configuration 
    # framework uses a component accumulator.

    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    result = ComponentAccumulator()

    # do not run in RAWtoESD, if we have two-step reco
    if inputFlags.DQ.Environment in ('online', 'tier0', 'tier0ESD', 'AOD'):

        # Verify signature settings in case of AOD input, as content monitored by some signatures not available in AOD.
        # Will raise exception to prevent signature monitoring crashing during running.
        verifySignatureDQSettings(inputFlags)

        if inputFlags.DQ.Steering.HLT.doGeneral:
            from TrigHLTMonitoring.TrigGeneralMonitorAlgorithm import TrigGeneralMonConfig
            result.merge(TrigGeneralMonConfig(inputFlags))

        if inputFlags.DQ.Steering.HLT.doInDet:
            from TrigInDetMonitoring.TIDAMonitoring import TrigInDetMonConfig
            result.merge(TrigInDetMonConfig(inputFlags))

        if inputFlags.DQ.Steering.HLT.doBjet:
            from TrigBjetMonitoring.TrigBjetMonitorAlgorithm import TrigBjetMonConfig
            result.merge(TrigBjetMonConfig(inputFlags))

        if inputFlags.DQ.Steering.HLT.doBphys:
            from TrigBphysMonitoring.TrigBphysMonitorAlgorithm import TrigBphysMonConfig
            result.merge(TrigBphysMonConfig(inputFlags))

        if inputFlags.DQ.Steering.HLT.doCalo:
            from TrigCaloMonitoring.TrigCaloMonitorAlgorithm import TrigCaloMonConfig
            result.merge(TrigCaloMonConfig(inputFlags))

        if inputFlags.DQ.Steering.HLT.doEgamma:
            from TrigEgammaMonitoring.TrigEgammaMonitorAlgorithm import TrigEgammaMonConfig
            result.merge(TrigEgammaMonConfig(inputFlags))

        if inputFlags.DQ.Steering.HLT.doJet:
            from TrigJetMonitoring.TrigJetMonitorAlgorithm import TrigJetMonConfig
            result.merge(TrigJetMonConfig(inputFlags))

        if inputFlags.DQ.Steering.HLT.doMET:
            from TrigMETMonitoring.TrigMETMonitorAlgorithm import TrigMETMonConfig
            result.merge(TrigMETMonConfig(inputFlags))

        if inputFlags.DQ.Steering.HLT.doMinBias:
            from TrigMinBiasMonitoring.TrigMinBiasMonitoringMT import TrigMinBias
            result.merge(TrigMinBias(inputFlags))

        if inputFlags.DQ.Steering.HLT.doMuon:
            from TrigMuonMonitoring.TrigMuonMonitoringConfig import TrigMuonMonConfig
            result.merge(TrigMuonMonConfig(inputFlags))

        if inputFlags.DQ.Steering.HLT.doTau:
            from TrigTauMonitoring.TrigTauMonitorAlgorithm import TrigTauMonConfig
            result.merge(TrigTauMonConfig(inputFlags))

    return result


