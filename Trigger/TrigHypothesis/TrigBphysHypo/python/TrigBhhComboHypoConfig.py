# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from TrigBphysHypo.TrigBhhComboHypoMonitoringConfig import TrigBhhComboHypoMonitoring
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

from AthenaCommon.Logging import logging
log = logging.getLogger('TrigBhhComboHypoConfig')

def BhhComboHypoCfg(flags, name):
    log.debug('BhhComboHypoCfg.name = %s', name)
    suffix = 'Bhh'
    acc = ComponentAccumulator()
    from TrigBphysHypo.TrigBPhyCommonConfig import TrigBPHY_TrkVKalVrtFitterCfg
    from InDetConfig.InDetConversionFinderToolsConfig import BPHY_VertexPointEstimatorCfg

    hypo = CompFactory.TrigBhhComboHypo(
        name = 'BhhComboHypo',
        VertexFitter = acc.popToolsAndMerge(TrigBPHY_TrkVKalVrtFitterCfg(flags, suffix)),
        VertexPointEstimator = acc.popToolsAndMerge(BPHY_VertexPointEstimatorCfg(flags, 'VertexPointEstimator_'+suffix)),
        CheckMultiplicityMap = False,
        TrigBphysCollectionKey = 'HLT_Bhh',
        TrackCollectionKey = 'HLT_IDTrack_Bhh_FTF',
        ApplyMuonRemoval = False,
        DeltaR = 0.01,
        Bhh_trackPtThreshold = 2500.,
        Bhh_massRange = (4500., 6500.),
        Bhh_chi2 = 20.,
        FitAttemptsWarningThreshold = 200,
        FitAttemptsBreakThreshold = 1000,
        MonTool = TrigBhhComboHypoMonitoring(flags, 'TrigBhhComboHypoMonitoring'))

    acc.addEventAlgo(hypo)
    return acc
