# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from typing import Any

from AthenaCommon.SystemOfUnits import GeV
from AthenaConfiguration.AthConfigFlags import AthConfigFlags

from TriggerMenuMT.HLT.Tau.TauConfigurationTools import getChainIDConfigName
from .TrigTauHypoMonitoring import getTrigTauPrecisionIDHypoToolMonitoring, getTrigTauPrecisionDiKaonHypoToolMonitoring

from AthenaCommon.Logging import logging
log = logging.getLogger('TrigHLTTauHypoTool')


#============================================================================================
# Precision step hypothesis tool
#============================================================================================
def TrigTauPrecisionHypoToolFromDict(flags: AthConfigFlags, chainDict: dict[str, Any]):
    chainPart = chainDict['chainParts'][0]

    identification = getChainIDConfigName(flags, chainPart)

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
    def __init__(self, flags, chain_part: dict[str, Any]):
        self._id = getChainIDConfigName(flags, chain_part)
        self._chain_part = chain_part

        self._id_wp = ''
        self._highpt_id_wp = ''
        self._do_perfcore = True
        self._do_perfiso = True

        self._use_rnn_selection = False

        if self._id in ['idperf', 'noperf', 'perfcore', 'perfiso', 'perf']:
            self._do_perfcore = self._id in ['perfcore', 'perf']
            self._do_perfiso = self._id in ['perfiso', 'perf']

        else:
            if self._id in ['DeepSet', 'RNNLLP'] and self._chain_part['selection'].endswith('RNN'):
                # Support for the legacy triggers
                self._use_rnn_selection = True
                self._id_wp = self._chain_part['selection'][:-3] # Remove the "RNN" suffix
                if self._id_wp in ['medium', 'tight']: self._highpt_id_wp = 'loose'

            else:
                id_wp = self._chain_part['selection'].removesuffix(self._id).lower()

                # Check for a perf selection specifier
                sfx = id_wp[-2:]
                if sfx in ['np', 'pc', 'pi']: id_wp = id_wp[:-2]
                if sfx in ['np', 'pi']: self._do_perfcore = False
                if sfx in ['np', 'pc']: self._do_perfiso = False

                # Find the matching WP with the correct casing
                def find_wp(wp: str, fail: bool = True) -> str:
                    for twp in getattr(flags.Trigger.Offline.Tau, self._id).TargetWPs.keys():
                        if twp.lower() == wp: return twp
                    else:
                        if fail: ValueError(f'Cannot find the "{self._id}" WP "{wp}"')
                        else: return ''

                # Standard ID WP
                self._id_wp = find_wp(id_wp)
                
                # High-pT ID WP
                if id_wp.startswith('medium'): self._highpt_id_wp = find_wp(f'loose{id_wp[6:]}', True)
                elif id_wp.startswith('tight'): self._highpt_id_wp = find_wp(f'loose{id_wp[5:]}', True)
                
    @property
    def n_track_max(self) -> int:
        return 3 if self._do_perfcore else 999

    @property
    def n_iso_track_max(self) -> int:
        return 1 if self._do_perfiso else 999

    @property
    def pt_min(self) -> float: return float(self._chain_part['threshold']) * GeV

    @property
    def id_wp_decor(self) -> str:
        if not self._id_wp: return ''
        if self._use_rnn_selection: return self._id_wp
        return f'{self._id}_{self._id_wp}'

    @property
    def highpt_id_wp_decor(self) -> int:
        if not self._highpt_id_wp: return ''
        if self._use_rnn_selection: return self._highpt_id_wp
        return f'{self._id}_{self._highpt_id_wp}'


def TrigTauPrecisionIDHypoToolFromDict(flags: AthConfigFlags, chainDict: dict[str, Any]):
    '''TrigTauPrecisionIDHypoTool configuration for the standard Tau triggers'''
    name = chainDict['chainName']
    chainPart = chainDict['chainParts'][0]
    cuts = TauCuts(flags, chainPart)

    # Setup the Hypothesis tool
    from AthenaConfiguration.ComponentFactory import CompFactory
    currentHypo = CompFactory.TrigTauPrecisionIDHypoTool(
        name,
        PtMin=cuts.pt_min,
        NTracksMax=cuts.n_track_max,
        NIsoTracksMax=cuts.n_iso_track_max,
        IDWP=cuts.id_wp_decor,
        HighPtIDWP=cuts.highpt_id_wp_decor,
    )

    from TriggerMenuMT.HLT.Tau.TauConfigurationTools import getChainPrecisionSeqName, useBuiltInTauJetRNNScore, getPrecisionSequenceTauIDs, getTauIDScoreVariables

    id_score_monitoring = {}
    
    precision_seq_name = getChainPrecisionSeqName(chainPart)
    identification = getChainIDConfigName(flags, chainPart)
    if identification in ['idperf', 'noperf', 'perf', 'perfcore', 'perfiso']:
        if identification == 'idperf':
            # Disable everything, even the pT cut
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

        # Monitor this algorithm only
        id_score_monitoring[identification] = getTauIDScoreVariables(identification, precision_seq_name)

    # For any triggers following the tracktwoMVA reconstruction (2023+ DeepSet and GNTau)
    if chainPart['reconstruction'] == 'tracktwoMVA':
        currentHypo.TrackPtCut = 1.5*GeV
        currentHypo.HighPtSelectionIDThr = 200*GeV
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

def TrigTauPrecisionDiKaonHypoToolFromDict(flags: AthConfigFlags, chainDict: dict[str, Any]):
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
# Tracking step hypothesis tool (without selection)
#============================================================================================
def TrigTauTrackingHypoToolFromDict(flags: AthConfigFlags, chainDict: dict[str, Any]):
    name = chainDict['chainName']

    from AthenaConfiguration.ComponentFactory import CompFactory
    currentHypo = CompFactory.TrigTauTrackingHypoTool(name)

    return currentHypo



#============================================================================================
# CaloMVA step hypothesis tool
#============================================================================================
def TrigTauCaloMVAHypoToolFromDict(flags: AthConfigFlags, chainDict: dict[str, Any]):
    name = chainDict['chainName']
    threshold = float(chainDict['chainParts'][0]['threshold'])

    from AthenaConfiguration.ComponentFactory import CompFactory
    currentHypo = CompFactory.TrigTauCaloHypoTool(name)
    currentHypo.PtMin = threshold * GeV

    return currentHypo

