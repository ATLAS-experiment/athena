# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)

from ..Config.MenuComponents import MenuSequence, SelectionCA, InEventRecoCA, InViewRecoCA
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.AccumulatorCache import AccumulatorCache
from TrigEDMConfig.TriggerEDM import recordable


@AccumulatorCache
def bmumuxSequenceGenCfg(flags):

    RoIToolCreator = CompFactory.ViewCreatorMuonSuperROITool if flags.Trigger.InDetTracking.bmumux.SuperRoI else CompFactory.ViewCreatorCentredOnIParticleROITool

    roiToolOptions = {
        'RoIEtaWidth' : flags.Trigger.InDetTracking.bmumux.etaHalfWidth,
        'RoIPhiWidth' : flags.Trigger.InDetTracking.bmumux.phiHalfWidth,
        'RoIZedWidth' : flags.Trigger.InDetTracking.bmumux.zedHalfWidth,
        'RoisWriteHandleKey' : recordable(flags.Trigger.InDetTracking.bmumux.roi) }

    viewMakerOptions = {
        'RoITool' : RoIToolCreator(**roiToolOptions),
        'mergeUsingFeature' : True,
        'PlaceMuonInView' : True,
        'InViewMuonCandidates' : 'BmumuxMuonCandidates',
        'InViewMuons' : 'HLT_Muons_Bmumux' }

    reco = InViewRecoCA('Bmumux', **viewMakerOptions)
    from .BphysicsRecoSequences import bmumuxRecoSequenceCfg
    reco.mergeReco(bmumuxRecoSequenceCfg(flags, reco.inputMaker().InViewRoIs, reco.inputMaker().InViewMuons))

    selAcc = SelectionCA('bmumuxSequence')

    from TrigGenericAlgs.TrigGenericAlgsConfig import ROBPrefetchingAlgCfg_Si
    selAcc.mergeReco(reco, robPrefetchCA=ROBPrefetchingAlgCfg_Si(flags, nameSuffix=reco.name))

    hypoAlg = CompFactory.TrigBphysStreamerHypo('BmumuxStreamerHypoAlg')
    selAcc.addHypoAlgo(hypoAlg)

    from TrigBphysHypo.TrigBphysStreamerHypoConfig import TrigBphysStreamerHypoToolFromDict
    return MenuSequence(flags, selAcc, HypoToolGen=TrigBphysStreamerHypoToolFromDict)


@AccumulatorCache
def bhhSequenceGenCfg(flags):

    roiToolOptions = {
        'isBhh' : True,
        'RoIEtaWidth' : flags.Trigger.InDetTracking.bhh.etaHalfWidth,
        'RoIPhiWidth' : flags.Trigger.InDetTracking.bhh.phiHalfWidth,
        'RoIZedWidth' : flags.Trigger.InDetTracking.bhh.zedHalfWidth,
        'RoisWriteHandleKey' : recordable(flags.Trigger.InDetTracking.bhh.roi) }

    viewMakerOptions = {
        'RoITool' : CompFactory.ViewCreatorMuonSuperROITool(name='RoiTool_Bhh', **roiToolOptions),
        'mergeUsingFeature' : True,
        'PlaceMuonInView' : True,
        'InViewMuonCandidates' : 'BhhMuonCandidates',
        'InViewMuons' : 'HLT_Muons_Bhh' }

    reco = InViewRecoCA('Bhh', **viewMakerOptions)
    from .BphysicsRecoSequences import bhhRecoSequenceCfg
    reco.mergeReco(bhhRecoSequenceCfg(flags, reco.inputMaker().InViewRoIs))

    selAcc = SelectionCA('bhhSequence')

    from TrigGenericAlgs.TrigGenericAlgsConfig import ROBPrefetchingAlgCfg_Si
    selAcc.mergeReco(reco, robPrefetchCA=ROBPrefetchingAlgCfg_Si(flags, nameSuffix=reco.name))

    hypoAlg = CompFactory.TrigBphysStreamerHypo('BhhStreamerHypoAlg')
    selAcc.addHypoAlgo(hypoAlg)

    from TrigBphysHypo.TrigBphysStreamerHypoConfig import TrigBphysStreamerHypoToolFromDict
    return MenuSequence(flags, selAcc, HypoToolGen=TrigBphysStreamerHypoToolFromDict)


def dimuL2SequenceGenCfg(flags):
    from ..Muon.MuonMenuSequences import muCombAlgSequenceCfg
    from TrigBphysHypo.TrigBphysStreamerHypoConfig import TrigBphysStreamerHypoToolFromDict

    sequence, combinedMuonContainerName = muCombAlgSequenceCfg(flags, "Bphys")
    hypo = CompFactory.TrigBphysStreamerHypo('DimuL2StreamerHypoAlg',
                                             triggerList = getNoL2CombChainNames(),
                                             triggerLevel = 'L2')
    sequence.addHypoAlgo(hypo)

    return MenuSequence(flags, sequence, HypoToolGen=TrigBphysStreamerHypoToolFromDict)


@AccumulatorCache
def dimuEFSequenceGenCfg(flags):
    selAcc = SelectionCA('dimuSequence')

    inputMakerAlg = CompFactory.InputMakerForRoI('IM_bphysStreamerDimuEF',
        RoITool = CompFactory.ViewCreatorPreviousROITool(),
        mergeUsingFeature = True)

    reco = InEventRecoCA('bphysStreamerDimuEFReco', inputMaker=inputMakerAlg)
    selAcc.mergeReco(reco)

    hypoAlg = CompFactory.TrigBphysStreamerHypo('DimuEFStreamerHypoAlg', triggerLevel = 'EF')
    selAcc.addHypoAlgo(hypoAlg)

    from TrigBphysHypo.TrigBphysStreamerHypoConfig import TrigBphysStreamerHypoToolFromDict
    return MenuSequence(flags, selAcc, HypoToolGen=TrigBphysStreamerHypoToolFromDict)


def getNoL2CombChainNames():
    from ..Config.GenerateMenuMT import GenerateMenuMT
    menu = GenerateMenuMT()
    chains = [chain.name for chain in menu.chainsInMenu['Bphysics'] if 'noL2Comb' in chain.name]
    return chains
