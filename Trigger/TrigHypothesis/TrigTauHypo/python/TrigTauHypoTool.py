# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from typing import Any

from AthenaCommon.SystemOfUnits import GeV

from .TrigTauHypoMonitoring import getTrigTauPrecisionIDHypoToolMonitoring, getTrigTauPrecisionDiKaonHypoToolMonitoring

from AthenaCommon.Logging import logging
log = logging.getLogger('TrigHLTTauHypoTool')

#============================================================================================
# Precision step hypothesis tool
#============================================================================================
def TrigTauPrecisionHypoToolFromDict(flags, chainDict):
    chainPart = chainDict['chainParts'][0]

    from TriggerMenuMT.HLT.Tau.TauConfigurationTools import getChainIDConfigName
    identification = getChainIDConfigName(chainPart)

    if identification == 'MesonCuts':
        # Meson cut-based triggers (ATR-22644)
        return TrigTauPrecisionDiKaonHypoToolFromDict(flags, chainDict)
    else:
        # Everything else
        return TrigTauPrecisionIDHypoToolFromDict(flags, chainDict)

#-----------------------------------------------------------------
# Standard tau triggers configuration
#-----------------------------------------------------------------
class TauCuts:
    def __init__(self, chain_part: dict[str, Any]):
        self._chain_part = chain_part

    @property
    def n_track_max(self) -> int: return 3

    @property
    def n_iso_track_max(self) -> int: return 999 if self._chain_part['selection'] == 'idperf' else 1

    @property
    def pt_min(self) -> float: return float(self._chain_part['threshold']) * GeV

    @property
    def id_wp(self) -> int:
        sel = self._chain_part['selection']

        if sel == 'perf' or sel == 'idperf': return -1  # disabled
        elif sel.startswith('veryloose'): return 0
        elif sel.startswith('loose'): return 1
        elif sel.startswith('medium'): return 2
        elif sel.startswith('tight'): return 3
        
        raise ValueError(f'Invalid selection: {sel}')

def TrigTauPrecisionIDHypoToolFromDict(flags, chainDict):
    '''TrigTauPrecisionIDHypoTool configuration for the standard Tau triggers'''
    name = chainDict['chainName']
    chainPart = chainDict['chainParts'][0]
    cuts = TauCuts(chainPart)

    # Setup the Hypothesis tool
    from AthenaConfiguration.ComponentFactory import CompFactory
    currentHypo = CompFactory.TrigTauPrecisionIDHypoTool(
        name,
        PtMin=cuts.pt_min,
        NTracksMax=cuts.n_track_max,
        NIsoTracksMax=cuts.n_iso_track_max,
        IDWP=cuts.id_wp,
    )

    from TriggerMenuMT.HLT.Tau.TauConfigurationTools import getChainIDConfigName, getChainPrecisionSeqName, useBuiltInTauJetRNNScore, getPrecisionSequenceTauIDs, getTauIDScoreVariables

    id_score_monitoring = {}
    
    precision_seq_name = getChainPrecisionSeqName(chainPart)
    identification = getChainIDConfigName(chainPart)
    if identification in ['perf', 'idperf']:
        if identification == 'idperf':
            currentHypo.AcceptAll = True

        # Monitor all the included algorithms
        used_builtin_rnnscore = False
        for tau_id in getPrecisionSequenceTauIDs(flags, precision_seq_name):
            # Skip algs without inference scores
            if tau_id in ['MesonCuts']: continue

            # We can only have at most one alg. using the built-in TauJet RNN score variables
            if useBuiltInTauJetRNNScore(tau_id, precision_seq_name):
                if used_builtin_rnnscore:
                    raise ValueError('Cannot have two TauID algorithms with scores stored in the built-in TauJet RNN score variables')
                used_builtin_rnnscore = True

            id_score_monitoring[tau_id] = getTauIDScoreVariables(tau_id, precision_seq_name)
                
    else:
        if useBuiltInTauJetRNNScore(identification, precision_seq_name):
            # To support the legacy tracktwoMVA/LLP/LRT chains, only in those cases we store the
            # ID score and passed WPs in the native TauJet variables
            currentHypo.IDMethod = 1 # TauJet built-in RNN score
        else:
            # Decorator-based triggers
            currentHypo.IDMethod = 2 # Use decorators
            currentHypo.IDWPNames = [f'{identification}_{wp}' for wp in getattr(flags.Trigger.Offline.Tau, identification).WPNames]

        # Monitor this algorithm only
        id_score_monitoring[identification] = getTauIDScoreVariables(identification, precision_seq_name)

    # For any triggers following the tracktwoMVA reconstruction (2023+ DeepSet and GNTau)
    if chainPart['reconstruction'] == 'tracktwoMVA':
        currentHypo.TrackPtCut = 1.5*GeV
        currentHypo.HighPtSelectionLooseIDThr = 200*GeV
        currentHypo.HighPtSelectionJetThr = 430*GeV

    # Only monitor chains with the 'tauMon:online' groups
    if 'tauMon:online' in chainDict['monGroups']:
        currentHypo.MonTool = getTrigTauPrecisionIDHypoToolMonitoring(flags, name, id_score_monitoring.keys())

    # TauID Score monitoring
    currentHypo.MonitoredIDScores = id_score_monitoring

    return currentHypo


#-----------------------------------------------------------------
# Meson cut-based triggers configuration (ATR-22644 + ATR-23239)
#-----------------------------------------------------------------
from collections import namedtuple

DiKaonCuts = namedtuple('DiKaonCuts', 'massTrkSysMin massTrkSysMax massTrkSysKaonMin massTrkSysKaonMax massTrkSysKaonPiMin massTrkSysKaonPiMax targetMassTrkSysKaonPi leadTrkPtMin PtMin EMPOverTrkSysPMax')
thresholds_dikaon = {
    ('dikaonmass', 25): DiKaonCuts(0.0*GeV,   1000.0*GeV,  0.987*GeV, 1.060*GeV,   0.0*GeV,  1000.0*GeV, 0.0*GeV,   15.0*GeV, 25.0*GeV, 1.5),
    ('dikaonmass', 35): DiKaonCuts(0.0*GeV,   1000.0*GeV,  0.987*GeV, 1.060*GeV,   0.0*GeV,  1000.0*GeV, 0.0*GeV,   25.0*GeV, 35.0*GeV, 1.5),

    ('kaonpi1',    25): DiKaonCuts(0.0*GeV,   1000.0*GeV,  0.0*GeV,   1000.0*GeV,  0.79*GeV, 0.99*GeV,   0.89*GeV,  15.0*GeV, 25.0*GeV, 1.0),
    ('kaonpi1',    35): DiKaonCuts(0.0*GeV,   1000.0*GeV,  0.0*GeV,   1000.0*GeV,  0.79*GeV, 0.99*GeV,   0.89*GeV,  25.0*GeV, 35.0*GeV, 1.0),

    ('kaonpi2',    25): DiKaonCuts(0.0*GeV,   1000.0*GeV,  0.0*GeV,   1000.0*GeV,  1.8*GeV,  1.93*GeV,   1.865*GeV, 15.0*GeV, 25.0*GeV, 1.0),
    ('kaonpi2',    35): DiKaonCuts(0.0*GeV,   1000.0*GeV,  0.0*GeV,   1000.0*GeV,  1.8*GeV,  1.93*GeV,   1.865*GeV, 25.0*GeV, 35.0*GeV, 1.0),

    ('dipion1',    25): DiKaonCuts(0.475*GeV, 1.075*GeV,   0.0*GeV,   1000.0*GeV,  0.0*GeV,  1000.0*GeV, 0.0*GeV,   15.0*GeV, 25.0*GeV, 1.0),
    ('dipion2',    25): DiKaonCuts(0.460*GeV, 0.538*GeV,   0.0*GeV,   1000.0*GeV,  0.0*GeV,  1000.0*GeV, 0.0*GeV,   15.0*GeV, 25.0*GeV, 1.0),
    ('dipion3',    25): DiKaonCuts(0.279*GeV, 0.648*GeV,   0.0*GeV,   1000.0*GeV,  0.0*GeV,  1000.0*GeV, 0.0*GeV,   25.0*GeV, 25.0*GeV, 2.2),
    ('dipion4',    25): DiKaonCuts(0.460*GeV, 1.075*GeV,   0.0*GeV,   1000.0*GeV,  0.0*GeV,  1000.0*GeV, 0.0*GeV,   15.0*GeV, 25.0*GeV, 1.0),
}

SinglePionCuts = namedtuple('SinglePionCuts', 'leadTrkPtMin PtMin NTracksMax NIsoTracksMax dRmaxMax etOverPtLeadTrkMin etOverPtLeadTrkMax')
thresholds_singlepion = {
    ('singlepion', 25): SinglePionCuts(30.0*GeV, 25.0*GeV, 1, 0, 0.06, 0.4, 0.85),
}

def TrigTauPrecisionDiKaonHypoToolFromDict(flags, chainDict):
    '''TrigTauPrecisionDiKaonHypoTool configuration for the meson cut-based Tau triggers (ATR-22644)'''
    name = chainDict['chainName']
    chainPart = chainDict['chainParts'][0]

    # Setup the Hypothesis tool
    from AthenaConfiguration.ComponentFactory import CompFactory
    currentHypo = CompFactory.TrigTauPrecisionDiKaonHypoTool(name)

    key = (chainPart['selection'], int(chainPart['threshold']))
    if key in thresholds_dikaon:
        thr = thresholds_dikaon[key]
        currentHypo.PtMin                  = thr.PtMin
        currentHypo.leadTrkPtMin           = thr.leadTrkPtMin
        currentHypo.massTrkSysMin          = thr.massTrkSysMin
        currentHypo.massTrkSysMax          = thr.massTrkSysMax
        currentHypo.massTrkSysKaonMin      = thr.massTrkSysKaonMin
        currentHypo.massTrkSysKaonMax      = thr.massTrkSysKaonMax
        currentHypo.massTrkSysKaonPiMin    = thr.massTrkSysKaonPiMin
        currentHypo.massTrkSysKaonPiMax    = thr.massTrkSysKaonPiMax
        currentHypo.targetMassTrkSysKaonPi = thr.targetMassTrkSysKaonPi
        currentHypo.EMPOverTrkSysPMax      = thr.EMPOverTrkSysPMax
        
    elif key in thresholds_singlepion:
        thr = thresholds_singlepion[key]
        currentHypo.PtMin              = thr.PtMin
        currentHypo.NTracksMax         = thr.NTracksMax
        currentHypo.NIsoTracksMax      = thr.NIsoTracksMax
        currentHypo.leadTrkPtMin       = thr.leadTrkPtMin
        currentHypo.dRmaxMax           = thr.dRmaxMax
        currentHypo.etOverPtLeadTrkMin = thr.etOverPtLeadTrkMin
        currentHypo.etOverPtLeadTrkMax = thr.etOverPtLeadTrkMax

    currentHypo.MonTool = getTrigTauPrecisionDiKaonHypoToolMonitoring(flags, name)

    return currentHypo



#============================================================================================
# Precision Tracking step hypothesis tool (without selection)
#============================================================================================
def TrigTauPrecTrackHypoToolFromDict(chainDict):
    name = chainDict['chainName']

    from AthenaConfiguration.ComponentFactory import CompFactory
    currentHypo = CompFactory.TrigTauPrecTrackHypoTool(name)

    return currentHypo



#============================================================================================
# FTF steps hypothesis tools (without selection)
#============================================================================================
def TrigTauFastTrackHypoToolFromDict(chainDict):
    name = chainDict['chainName']

    from AthenaConfiguration.ComponentFactory import CompFactory
    currentHypo = CompFactory.TrigTauFastTrackHypoTool(name)

    return currentHypo



#============================================================================================
# CaloMVA step hypothesis tool
#============================================================================================
def TrigTauCaloMVAHypoToolFromDict(chainDict):
    name = chainDict['chainName']
    threshold = float(chainDict['chainParts'][0]['threshold'])

    from AthenaConfiguration.ComponentFactory import CompFactory
    currentHypo = CompFactory.TrigTauCaloHypoTool(name)
    currentHypo.PtMin = threshold * GeV

    return currentHypo

